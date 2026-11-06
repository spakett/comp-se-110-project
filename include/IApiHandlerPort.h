#pragma once

#include <string>
#include <vector>

#include "domainModels.h"

class IApiHandlerCallbackPort {
public:
    virtual ~IApiHandlerCallbackPort() = default;

    virtual void onAreaAirQualityFound(const std::string& requestId, const std::vector<AirQualityData>& data) = 0;
    virtual void onApiError(const std::string& requestId, const std::string& message) = 0;
};

class IApiHandlerPort {
public:
    virtual ~IApiHandlerPort() = default;

    virtual void requestAreaAirQuality(
        const std::string& requestId,
        const BoundingBox& area,
        const std::vector<Irritant>& irritants
    ) = 0;
};