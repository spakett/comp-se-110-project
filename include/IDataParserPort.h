#pragma once
#include <string>
#include <vector>
#include "DomainModels.h"

// Core uses this to translate raw JSON into structs defined in domainModels
class IDataParserPort {
public:
    virtual ~IDataParserPort() = default;

    virtual std::vector<AirQualityData> parseAirQuality(const std::string& jsonPayload) = 0;
    virtual std::vector<WeatherData> parseWeatherForecast(const std::string& jsonPayload) = 0;
    virtual std::vector<RouteData> parseRoute(const std::string& jsonPayload) = 0;
    virtual std::vector<Location> parseLocation(const std::string& jsonPayload) = 0;
};