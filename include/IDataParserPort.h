#pragma once

#include <string>
#include <vector>

#include "domainModels.h"

// Translates raw JSON payloads into structs defined in domainModels.
class IDataParserPort {
public:
    virtual ~IDataParserPort() = default;

    virtual std::vector<AirQualityData> parseAirQuality(
        const std::string& jsonPayload,
        const std::vector<Irritant>& irritants,
        const std::vector<Coordinates>& requestedPoints,
        std::string* error = nullptr
    ) = 0;

    virtual std::vector<RouteData> parseRoute(
        const std::string& jsonPayload,
        std::string* error = nullptr
    ) = 0;
};
