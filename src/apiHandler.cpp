#include "apiHandler.h"

#include "airQualityLevels.h"

#include <QJsonArray>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
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

bool parseSamples(
    const std::string& payload,
    const std::vector<Irritant>& irritants,
    const std::vector<Coordinates>& requestedPoints,
    std::vector<AirQualityData>& samples,
    std::string& error
) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(payload), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        error = parseError.errorString().toStdString();
        return false;
    }

    QJsonArray locations;
    if (document.isArray()) {
        locations = document.array();
    } else if (document.isObject() && requestedPoints.size() == 1) {
        locations.append(document.object());
    } else {
        error = "Air-quality response had an unexpected shape";
        return false;
    }
    if (locations.size() != static_cast<int>(requestedPoints.size())) {
        error = "Air-quality response did not contain every requested grid point";
        return false;
    }

    for (qsizetype index = 0; index < locations.size(); ++index) {
        if (!locations[index].isObject()) {
            error = "Air-quality response contained an invalid location";
            return false;
        }
        const QJsonObject location = locations[index].toObject();
        const QJsonObject current = location.value("current").toObject();
        if (current.isEmpty() || !current.value("time").isString()) {
            error = "Air-quality response did not contain current readings";
            return false;
        }

        const Coordinates fallback = requestedPoints[static_cast<std::size_t>(index)];
        const double latitude = location.value("latitude").toDouble(fallback.latitude);
        const double longitude = location.value("longitude").toDouble(fallback.longitude);
        AirQualityData sample{current.value("time").toString().toStdString(), {}, {}, {{latitude, longitude}}};
        for (Irritant irritant : irritants) {
            const std::string name = apiVariable(irritant);
            const QJsonValue reading = current.value(QString::fromStdString(name));
            if (!name.empty() && reading.isDouble()) {
                const double value = reading.toDouble();
                sample.readings.emplace(irritant, value);
                sample.levels.emplace(irritant, levelForReading(irritant, value));
            }
        }
        samples.push_back(std::move(sample));
    }
    return true;
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

// Valhalla encodes shapes as polyline6 (precision 1e6)
std::vector<Coordinates> decodePolyline(const std::string& encoded, double precision = 1e6) {
    std::vector<Coordinates> points;
    std::size_t index = 0;
    long long latitude = 0;
    long long longitude = 0;
    const auto next = [&](long long& out) {
        long long result = 0;
        int shift = 0;
        int byte = 0;
        do {
            if(index >= encoded.size())
                return false;
            byte = static_cast<unsigned char>(encoded[index++]) - 63;
            result |= static_cast<long long>(byte & 0x1f) << shift;
            shift += 5;
        } while (byte >= 0x20);
        out = (result & 1) ? ~(result >> 1) : (result >> 1);
        return true;
    };
    while (index < encoded.size()) {
        long long dLat = 0;
        long long dLon = 0;
        if (!next(dLat) || !next(dLon))
            break;
        latitude += dLat;
        longitude += dLon;
        points.push_back({latitude/precision, longitude/precision});
    }
    return points;
}

bool parseTrip(const QJsonObject& trip, RouteData& route, std::string& error) {
    const QJsonObject summary = trip.value("summary").toObject();
    const QJsonArray legs = trip.value("legs").toArray();
    if (summary.isEmpty() || legs.isEmpty()) {
        error = "Route response is empty";
        return false;
    }
    route.totalLengthKm = summary.value("length").toDouble();
    route.totalTimeSeconds = summary.value("time").toDouble();
    route.totalCost = summary.value("cost").toDouble(route.totalTimeSeconds);

    for(const QJsonValue& legValue : legs) {
        const QJsonObject leg = legValue.toObject();
        for(const QJsonValue& maneuverValue : leg.value("maneuvers").toArray()) {
            const QJsonObject maneuver = maneuverValue.toObject();
            route.maneuvers.push_back({
                maneuver.value("instruction").toString().toStdString(),
                maneuver.value("length").toDouble(),
                maneuver.value("time").toDouble(),
            });
        }
        std::vector<Coordinates> shape = decodePolyline(leg.value("shape").toString().toStdString());
        if(!route.polyline.empty() && !shape.empty()){
            shape.erase(shape.begin());
        }
        route.polyline.insert(route.polyline.end(), shape.begin(), shape.end());
    }

    return true;
}

bool parseRoutes(const std::string& payload, std::vector<RouteData>& routes, std::string& error) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(payload), &parseError);
    if(parseError.error != QJsonParseError::NoError) {
        error = parseError.errorString().toStdString();
        return false;
    }
    if(!document.isObject()) {
        error = "Route response had an unexpected shape";
        return false;
    }

    const QJsonObject root = document.object();
    if(root.contains("error")) {
        error = root.value("error").toString().toStdString();
        return false;
    }

    RouteData primary{};
    if(!parseTrip(root.value("trip").toObject(), primary, error)) {
        return false;
    }
    routes.push_back(std::move(primary));

    // Skip any alternates that are failed to parse and keep the primary
    for(const QJsonValue& alternate : root.value("alternates").toArray()) {
        RouteData route{};
        std::string ignored;
        if(parseTrip(alternate.toObject().value("trip").toObject(), route, ignored)) {
            routes.push_back(std::move(route));
        }
    }
    return true;
}

}

ApiHandler::ApiHandler(INetworkRequestPort& networkRequest, IDatabasePort& database)
    : m_networkRequest(networkRequest)
    , m_database(database) {
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
            std::vector<AirQualityData> samples;
            if(parseSamples(payload, context.irritants, context.points, samples, error)) {
                m_callback->onAreaAirQualityFound(requestId, samples);
                return true;
            }
            break;
        }
        case RequestKind::Routes: {
            std::vector<RouteData> routes;
            if(parseRoutes(payload, routes, error)) {
                m_callback->onRoutesFound(requestId, routes);
                return true;
            }
            break;
        }
    }
    if(reportError)
        m_callback->onApiError(requestId, std::string("Could not parse ") + label(context.kind) + " reponse: " + error);
    return false;
}
