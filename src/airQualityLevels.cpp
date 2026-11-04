#include "airQualityLevels.h"

const std::vector<Irritant>& allIrritants() {
    static const std::vector<Irritant> irritants = {
        Irritant::PM2_5, Irritant::PM10, Irritant::NitrogenDioxide,
        Irritant::Ozone, Irritant::BirchPollen, Irritant::GrassPollen,
    };
    return irritants;
}

std::string irritantDisplayName(Irritant irritant) {
    switch (irritant) {
    case Irritant::PM2_5: return "PM 2.5";
    case Irritant::PM10: return "PM 10";
    case Irritant::NitrogenDioxide: return "Nitrogen dioxide (NO₂)";
    case Irritant::Ozone: return "Ozone (O₃)";
    case Irritant::BirchPollen: return "Birch pollen";
    case Irritant::GrassPollen: return "Grass pollen";
    default: return "Unknown";
    }
}

const std::array<double, 5>& levelThresholds(Irritant irritant) {
    static const std::array<double, 5> pm25 = {10, 20, 25, 50, 75};
    static const std::array<double, 5> pm10 = {20, 40, 50, 100, 150};
    static const std::array<double, 5> no2 = {40, 90, 120, 230, 340};
    static const std::array<double, 5> o3 = {50, 100, 130, 240, 380};
    static const std::array<double, 5> pollen = {10, 50, 100, 500, 1000};

    switch (irritant) {
    case Irritant::PM2_5: return pm25;
    case Irritant::PM10: return pm10;
    case Irritant::NitrogenDioxide: return no2;
    case Irritant::Ozone: return o3;
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
        const auto reading = data.readings.find(irritant);
        if (reading == data.readings.end())
            continue;
        const AirQualityLevel level = levelForReading(irritant, reading->second);
        if (!worst || level > *worst)
            worst = level;
    }
    return worst;
}
