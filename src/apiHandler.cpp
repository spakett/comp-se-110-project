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

constexpr char kApiIdentifier[] = "open-meteo-air-quality";
constexpr char kEndpoint[] = "https://air-quality-api.open-meteo.com/v1/air-quality";
constexpr char kDomain[] = "cams_europe";
constexpr std::size_t kMaxGridPoints = 100;

std::string apiVariable(Irritant irritant) {
    switch (irritant) {
    case Irritant::PM2_5: return "pm2_5";
    case Irritant::PM10: return "pm10";
    case Irritant::NitrogenDioxide: return "nitrogen_dioxide";
    case Irritant::Ozone: return "ozone";
    case Irritant::BirchPollen: return "birch_pollen";
    case Irritant::GrassPollen: return "grass_pollen";
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

    QUrl url(QString::fromLatin1(kEndpoint));
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
        AirQualityData sample{current.value("time").toString().toStdString(), {}, {{latitude, longitude}}};
        for (Irritant irritant : irritants) {
            const std::string name = apiVariable(irritant);
            const QJsonValue reading = current.value(QString::fromStdString(name));
            if (!name.empty() && reading.isDouble())
                sample.readings.emplace(irritant, reading.toDouble());
        }
        samples.push_back(std::move(sample));
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

    const QUrl url = makeRequestUrl(points, variables);
    dispatchRequest(
        requestId,
        url.toString(QUrl::FullyEncoded).toStdString(),
        {{"Accept", "application/json"}},
        points,
        irritants
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
    if (apiIdentifier != kApiIdentifier) {
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
    const std::string& apiIdentifier,
    const std::string& errorMessage
) {
    if (!m_callback)
        return;
    if (m_pendingRequests.erase(requestId) == 0)
        return;
    const std::string prefix = apiIdentifier == kApiIdentifier ? "Air quality" : "Network";
    m_callback->onApiError(requestId, prefix + ": " + errorMessage);
}

void ApiHandler::dispatchRequest(
    const std::string& requestId,
    const std::string& url,
    const std::map<std::string, std::string>& headers,
    const std::vector<Coordinates>& points,
    const std::vector<Irritant>& irritants
) {
    if (!m_callback)
        return;
    const RequestContext context{url, points, irritants};
    try {
        const std::string cachedPayload = m_database.loadCache(context.cacheKey);
        if (!cachedPayload.empty() && deliverResponse(requestId, context, cachedPayload, false))
            return;
    } catch (...) {
    }

    m_pendingRequests[requestId] = context;
    try {
        m_networkRequest.request(requestId, "GET", kApiIdentifier, url, headers);
    } catch (const std::exception& exception) {
        m_pendingRequests.erase(requestId);
        m_callback->onApiError(requestId, std::string("Could not start air-quality request: ") + exception.what());
    } catch (...) {
        m_pendingRequests.erase(requestId);
        m_callback->onApiError(requestId, "Could not start air-quality request");
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
    std::vector<AirQualityData> samples;
    std::string error;
    if (!parseSamples(payload, context.irritants, context.points, samples, error)) {
        if (reportError)
            m_callback->onApiError(requestId, "Could not parse air-quality response: " + error);
        return false;
    }
    m_callback->onAreaAirQualityFound(requestId, samples);
    return true;
}