#include "dataParser.h"

#include "airQualityLevels.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace {

void setError(std::string* error, const std::string& message) {
    if (error)
        *error = message;
}

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
            if (index >= encoded.size())
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
        points.push_back({latitude / precision, longitude / precision});
    }
    return points;
}

bool parseTrip(const QJsonObject& trip, RouteData& route, std::string* error) {
    const QJsonObject summary = trip.value("summary").toObject();
    const QJsonArray legs = trip.value("legs").toArray();
    if (summary.isEmpty() || legs.isEmpty()) {
        setError(error, "Route response is empty");
        return false;
    }
    route.totalLengthKm = summary.value("length").toDouble();
    route.totalTimeSeconds = summary.value("time").toDouble();
    route.totalCost = summary.value("cost").toDouble(route.totalTimeSeconds);

    for (const QJsonValue& legValue : legs) {
        const QJsonObject leg = legValue.toObject();
        for (const QJsonValue& maneuverValue : leg.value("maneuvers").toArray()) {
            const QJsonObject maneuver = maneuverValue.toObject();
            route.maneuvers.push_back({
                maneuver.value("instruction").toString().toStdString(),
                maneuver.value("length").toDouble(),
                maneuver.value("time").toDouble(),
            });
        }
        std::vector<Coordinates> shape = decodePolyline(leg.value("shape").toString().toStdString());
        if (!route.polyline.empty() && !shape.empty()) {
            shape.erase(shape.begin());
        }
        route.polyline.insert(route.polyline.end(), shape.begin(), shape.end());
    }

    return true;
}

} // namespace

std::vector<AirQualityData> DataParser::parseAirQuality(
    const std::string& jsonPayload,
    const std::vector<Irritant>& irritants,
    const std::vector<Coordinates>& requestedPoints,
    std::string* error
) {
    setError(error, "");

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonPayload), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        setError(error, parseError.errorString().toStdString());
        return {};
    }

    QJsonArray locations;
    if (document.isArray()) {
        locations = document.array();
    } else if (document.isObject() && requestedPoints.size() == 1) {
        locations.append(document.object());
    } else {
        setError(error, "Air-quality response had an unexpected shape");
        return {};
    }
    if (locations.size() != static_cast<int>(requestedPoints.size())) {
        setError(error, "Air-quality response did not contain every requested grid point");
        return {};
    }

    std::vector<AirQualityData> samples;
    for (qsizetype index = 0; index < locations.size(); ++index) {
        if (!locations[index].isObject()) {
            setError(error, "Air-quality response contained an invalid location");
            return {};
        }
        const QJsonObject location = locations[index].toObject();
        const QJsonObject current = location.value("current").toObject();
        if (current.isEmpty() || !current.value("time").isString()) {
            setError(error, "Air-quality response did not contain current readings");
            return {};
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
    return samples;
}

std::vector<RouteData> DataParser::parseRoute(
    const std::string& jsonPayload,
    std::string* error
) {
    setError(error, "");

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonPayload), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        setError(error, parseError.errorString().toStdString());
        return {};
    }
    if (!document.isObject()) {
        setError(error, "Route response had an unexpected shape");
        return {};
    }

    const QJsonObject root = document.object();
    if (root.contains("error")) {
        setError(error, root.value("error").toString().toStdString());
        return {};
    }

    std::vector<RouteData> routes;
    RouteData primary{};
    if (!parseTrip(root.value("trip").toObject(), primary, error)) {
        return {};
    }
    routes.push_back(std::move(primary));

    // Skip any alternates that fail to parse and keep the primary
    for (const QJsonValue& alternate : root.value("alternates").toArray()) {
        RouteData route{};
        if (parseTrip(alternate.toObject().value("trip").toObject(), route, nullptr)) {
            routes.push_back(std::move(route));
        }
    }
    return routes;
}
