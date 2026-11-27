#include "apiHandler.h"

#include "airQualityLevels.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QStringList>

#include <cmath>
#include <exception>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr char kAirQualityApi[] = "open-meteo-air-quality";
constexpr char kAirQualityEndpoint[] = "https://air-quality-api.open-meteo.com/v1/air-quality";
constexpr char kDomain[] = "cams_europe";
constexpr std::size_t kMaxGridPoints = 100;

constexpr char kRoutingApi[] = "valhalla-route";
constexpr char kRoutingEndpoint[] = "https://valhalla1.openstreetmap.de/route";
constexpr int kRouteAlternates = 5;

std::string apiVariable(Irritant irritant) {
    switch (irritant) {
    case Irritant::PM2_5: return "pm2_5";
    case Irritant::PM10: return "pm10";
    case Irritant::NitrogenDioxide: return "nitrogen_dioxide";
    case Irritant::SulphurDioxide: return "sulphur_dioxide";
    case Irritant::Ozone: return "ozone";
    case Irritant::AlderPollen: return "alder_pollen";
    case Irritant::BirchPollen: return "birch_pollen";
    case Irritant::GrassPollen: return "grass_pollen";
    case Irritant::OlivePollen: return "olive_pollen";
    case Irritant::MugwortPollen: return "mugwort_pollen";
    case Irritant::RagweedPollen: return "ragweed_pollen";
    default: return {};
    }
}

QStringList selectedVariables(const std::vector<Irritant>& irritants) {
    QStringList variables;
    for (Irritant irritant : irritants) {
        const std::string name = apiVariable(irritant);
        if (!name.empty()) {
            const QString variable = QString::fromStdString(name);
            if (!variables.contains(variable))
                variables.append(variable);
        }
    }
    return variables;
}

bool makeGrid(const BoundingBox& area, std::vector<Coordinates>& points, std::string& error) {
    const double north = area.topLeft.latitude;
    const double south = area.bottomRight.latitude;
    const double west = area.topLeft.longitude;
    const double east = area.bottomRight.longitude;
    if (!std::isfinite(north) || !std::isfinite(south) || !std::isfinite(west) || !std::isfinite(east)
        || north <= south || east <= west || north > 90.0 || south < -90.0
        || west < -180.0 || east > 180.0) {
        error = "Invalid heatmap area";
        return false;
    }

    const double firstLatitude = std::floor(south / kGridStepLat) * kGridStepLat;
    const double firstLongitude = std::floor(west / kGridStepLon) * kGridStepLon;
    for (double latitude = firstLatitude; latitude < north + kGridStepLat; latitude += kGridStepLat) {
        for (double longitude = firstLongitude; longitude < east + kGridStepLon; longitude += kGridStepLon) {
            points.push_back({latitude, longitude});
            if (points.size() > kMaxGridPoints) {
                error = "Zoom in to request a smaller heatmap area";
                return false;
            }
        }
    }
    return !points.empty();
}

QUrl makeRequestUrl(const std::vector<Coordinates>& points, const QStringList& variables) {
    QStringList latitudes;
    QStringList longitudes;
    for (const Coordinates& point : points) {
        latitudes.append(QString::number(point.latitude, 'f', 7));
        longitudes.append(QString::number(point.longitude, 'f', 7));
    }

    QUrl url(QString::fromLatin1(kAirQualityEndpoint));
    QUrlQuery query;
    query.addQueryItem("latitude", latitudes.join(','));
    query.addQueryItem("longitude", longitudes.join(','));
    query.addQueryItem("current", variables.join(','));
    query.addQueryItem("timezone", "GMT");
    query.addQueryItem("domains", QString::fromLatin1(kDomain));
    url.setQuery(query);
    return url;
}

const char* costingFor(TravelMode mode) {
    switch(mode) {
        case TravelMode::Auto: return "auto";
        case TravelMode::Bicycle: return "bicycle";
        case TravelMode::Pedestrian: return "pedestrian";
    }
    return "pedestrian";
}

std::string makeRouteBody(const Coordinates& origin, const Coordinates& destination, TravelMode mode) {
    const auto location = [](const Coordinates& c) {
        return QJsonObject{{"lat", c.latitude}, {"lon", c.longitude}};
    };
    const QJsonObject body {
        {"locations", QJsonArray{location(origin), location(destination)}},
        {"costing", QString::fromLatin1(costingFor(mode))},
        {"alternates", kRouteAlternates},
        {"units", "kilometers"},
        {"language", "en-US"},
    };
    return QJsonDocument(body).toJson(QJsonDocument::Compact).toStdString();
}

}

ApiHandler::ApiHandler(INetworkRequestPort& networkRequest, IDatabasePort& database, IDataParserPort& dataParser)
    : m_networkRequest(networkRequest)
    , m_database(database)
    , m_dataParser(dataParser) {
}

void ApiHandler::setCallback(IApiHandlerCallbackPort& callback) {
    m_callback = &callback;
}

const char* ApiHandler::label(RequestKind kind) {
    switch(kind) {
        case RequestKind::AreaAirQuality: return "air-quality";
        case RequestKind::Routes: return "route";
    }
    return "API";
}

void ApiHandler::requestAreaAirQuality(
    const std::string& requestId,
    const BoundingBox& area,
    const std::vector<Irritant>& irritants
) {
    if (!m_callback)
        return;
    if (irritants.empty()) {
        m_callback->onAreaAirQualityFound(requestId, {});
        return;
    }

    std::vector<Coordinates> points;
    std::string error;
    if (!makeGrid(area, points, error)) {
        m_callback->onApiError(requestId, error);
        m_callback->onAreaAirQualityFound(requestId, {});
        return;
    }

    const QStringList variables = selectedVariables(irritants);
    if (variables.isEmpty()) {
        m_callback->onApiError(requestId, "No supported irritants are selected");
        m_callback->onAreaAirQualityFound(requestId, {});
        return;
    }

    const std::string url = makeRequestUrl(points, variables).toString(QUrl::FullyEncoded).toStdString();
    dispatchRequest(
        requestId,
        RequestContext{RequestKind::AreaAirQuality, kAirQualityApi, url, points, irritants},
        "GET", url, {{"Accept", "application/json"}}
    );
}

void ApiHandler::requestRoutes(
    const std::string& requestId,
    const Coordinates& origin,
    const Coordinates& destination,
    TravelMode mode
) {
    if(!m_callback) 
        return;
    const std::string url = kRoutingEndpoint;
    const std::string body = makeRouteBody(origin, destination, mode);
    dispatchRequest(
        requestId, 
        RequestContext{RequestKind::Routes, kRoutingApi, "POST " + url + "\n" + body, {}, {}},
        "POST", url,
        {{"Accept", "application/json"}, {"Content-Type", "application/json"}},
        body
    );
}

void ApiHandler::onNetworkResponse(
    const std::string& requestId,
    const std::string& apiIdentifier,
    const std::string& jsonPayload
) {
    if (!m_callback)
        return;
    const auto request = m_pendingRequests.find(requestId);
    if (request == m_pendingRequests.end())
        return;
    const RequestContext context = request->second;
    m_pendingRequests.erase(request);
    if (apiIdentifier != context.apiIdentifier) {
        m_callback->onApiError(requestId, "Network response did not match its request");
        return;
    }

    if (deliverResponse(requestId, context, jsonPayload, true)) {
        try {
            m_database.saveCache(context.cacheKey, jsonPayload);
        } catch (...) {
        }
    }
}

void ApiHandler::onNetworkError(
    const std::string& requestId,
    const std::string&,
    const std::string& errorMessage
) {
    if (!m_callback)
        return;
    const auto request = m_pendingRequests.find(requestId);
    if(request == m_pendingRequests.end())
        return;
    const RequestKind kind = request->second.kind;
    m_pendingRequests.erase(request);
    m_callback->onApiError(requestId, std::string(label(kind)) + " request failed: " + errorMessage);
}

void ApiHandler::dispatchRequest(
    const std::string& requestId,
    const RequestContext& context,
    const std::string& method,
    const std::string& url,
    const std::map<std::string, std::string>& headers,
    const std::string& body
) {
    if (!m_callback)
        return;
    try {
        const std::string cachedPayload = m_database.loadCache(context.cacheKey);
        if (!cachedPayload.empty() && deliverResponse(requestId, context, cachedPayload, false))
            return;
    } catch (...) {
    }

    m_pendingRequests[requestId] = context;
    try {
        m_networkRequest.request(requestId, method, context.apiIdentifier, url, headers, body);
    } catch (const std::exception& exception) {
        m_pendingRequests.erase(requestId);
        m_callback->onApiError(requestId, std::string("Could not start ") + label(context.kind) + " request: " + exception.what());
    } catch (...) {
        m_pendingRequests.erase(requestId);
        m_callback->onApiError(requestId, std::string("Could not start ") + label(context.kind) + " request");
    }
}

bool ApiHandler::deliverResponse(
    const std::string& requestId,
    const RequestContext& context,
    const std::string& payload,
    bool reportError
) {
    if (!m_callback)
        return false;
    std::string error;
    switch(context.kind) {
        case RequestKind::AreaAirQuality: {
            const std::vector<AirQualityData> samples =
                m_dataParser.parseAirQuality(payload, context.irritants, context.points, &error);
            if(error.empty()) {
                m_callback->onAreaAirQualityFound(requestId, samples);
                return true;
            }
            break;
        }
        case RequestKind::Routes: {
            const std::vector<RouteData> routes = m_dataParser.parseRoute(payload, &error);
            if(error.empty()) {
                m_callback->onRoutesFound(requestId, routes);
                return true;
            }
            break;
        }
    }
    if(reportError)
        m_callback->onApiError(requestId, std::string("Could not parse ") + label(context.kind) + " response: " + error);
    return false;
}
