#pragma once

#include <string>
#include <vector>

#include "IDataParserPort.h"

class DataParser final : public IDataParserPort {
public:
    std::vector<AirQualityData> parseAirQuality(
        const std::string& jsonPayload,
        const std::vector<Irritant>& irritants,
        const std::vector<Coordinates>& requestedPoints,
        std::string* error = nullptr
    ) override;

    std::vector<WeatherData> parseWeatherForecast(
        const std::string& jsonPayload,
        std::string* error = nullptr
    ) override;

    std::vector<RouteData> parseRoute(
        const std::string& jsonPayload,
        std::string* error = nullptr
    ) override;

    std::vector<Location> parseLocation(
        const std::string& jsonPayload,
        std::string* error = nullptr
    ) override;
};
