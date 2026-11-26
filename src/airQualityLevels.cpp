#include "airQualityLevels.h"
#include <cmath>

const std::vector<Irritant>& allIrritants() {
    static const std::vector<Irritant> irritants = {
        Irritant::PM2_5, Irritant::PM10, Irritant::NitrogenDioxide, Irritant::SulphurDioxide,
        Irritant::Ozone, Irritant::AlderPollen, Irritant::BirchPollen, Irritant::GrassPollen,
        Irritant::OlivePollen, Irritant::MugwortPollen, Irritant::RagweedPollen,
    };
    return irritants;
}

std::string irritantDisplayName(Irritant irritant) {
    switch (irritant) {
    case Irritant::PM2_5: return "PM 2.5";
    case Irritant::PM10: return "PM 10";
    case Irritant::NitrogenDioxide: return "Nitrogen dioxide (NO₂)";
    case Irritant::SulphurDioxide: return "Sulphur dioxide (SO₂)";
    case Irritant::Ozone: return "Ozone (O₃)";
    case Irritant::CarbonMonoxide: return "Carbon monoxide (CO)";
    case Irritant::CarbonDioxide: return "Carbon dioxide (CO₂)";
    case Irritant::AlderPollen: return "Alder pollen";
    case Irritant::BirchPollen: return "Birch pollen";
    case Irritant::GrassPollen: return "Grass pollen";
    case Irritant::OlivePollen: return "Olive pollen";
    case Irritant::MugwortPollen: return "Mugwort pollen";
    case Irritant::RagweedPollen: return "Ragweed pollen";
    default: return "Unknown";
    }
}

std::string levelDisplayName(AirQualityLevel level) {
    switch (level) {
    case AirQualityLevel::Good: return "Good";
    case AirQualityLevel::Fair: return "Fair";
    case AirQualityLevel::Moderate: return "Moderate";
    case AirQualityLevel::Poor: return "Poor";
    case AirQualityLevel::VeryPoor: return "Very poor";
    case AirQualityLevel::ExtremelyPoor: return "Extremely poor";
    }
    return "Unknown";
}

const std::array<double, 5>& levelThresholds(Irritant irritant) {
    static const std::array<double, 5> pm25 = {10, 20, 25, 50, 75};
    static const std::array<double, 5> pm10 = {20, 40, 50, 100, 150};
    static const std::array<double, 5> no2 = {40, 90, 120, 230, 340};
    static const std::array<double, 5> o3 = {50, 100, 130, 240, 380};
    static const std::array<double, 5> so2 = {100, 200, 350, 500, 750};
    static const std::array<double, 5> pollen = {10, 50, 100, 500, 1000};

    switch (irritant) {
    case Irritant::PM2_5: return pm25;
    case Irritant::PM10: return pm10;
    case Irritant::NitrogenDioxide: return no2;
    case Irritant::Ozone: return o3;
    case Irritant::SulphurDioxide: return so2;
    default: return pollen;
    }
}

AirQualityLevel levelForReading(Irritant irritant, double value) {
    const std::array<double, 5>& bounds = levelThresholds(irritant);
    for (std::size_t i = 0; i < bounds.size(); ++i) {
        if (value < bounds[i])
            return static_cast<AirQualityLevel>(i);
    }
    return AirQualityLevel::ExtremelyPoor;
}

std::optional<AirQualityLevel> worstLevel(const AirQualityData& data, const std::vector<Irritant>& selected) {
    std::optional<AirQualityLevel> worst;
    for (Irritant irritant : selected) {
        const auto level = data.levels.find(irritant);
        if (level == data.levels.end())
            continue;
        if (!worst || level->second > *worst)
            worst = level->second;
    }
    return worst;
}


namespace {
// Finds the sample closest to a point
const AirQualityData* nearestSample(const Coordinates& point, const std::vector<AirQualityData>& samples) {
    const AirQualityData* nearest = nullptr;
    double nearestDistance = 0.0;
    for (const AirQualityData& sample : samples) {
        if (sample.points.empty())
            continue;
        const double latDiff = std::abs(sample.points.front().latitude - point.latitude);
        const double lonDiff = std::abs(sample.points.front().longitude - point.longitude);
        if (latDiff > kGridStepLat || lonDiff > kGridStepLon)
            continue;
        const double distance = latDiff * latDiff + lonDiff * lonDiff;
        if (!nearest || distance < nearestDistance) {
            nearest = &sample;
            nearestDistance = distance;
        }
    }
    return nearest;
}

}

std::optional<AirQualityLevel> routeAirQuality(
    const RouteData& route,
    const std::vector<AirQualityData>& samples,
    const std::vector<Irritant>& selected
) {
    std::optional<AirQualityLevel> worst;
    for (const Coordinates& point : route.polyline) {
        const AirQualityData* sample = nearestSample(point, samples);
        if (!sample)
            continue;
        const std::optional<AirQualityLevel> level = worstLevel(*sample, selected);
        if (level && (!worst || *level > *worst))
            worst = level;
    }
    return worst;
}