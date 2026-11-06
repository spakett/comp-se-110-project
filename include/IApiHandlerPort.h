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

    virtual void searchLocation(const std::string& requestId, const std::string& addressQuery) = 0;
    virtual void requestRoute(
        const std::string& requestId,
        const BoundingBox& area,
        const std::vector<Irritant>& irritants
    ) = 0;
    
    virtual void requestWeatherForecast(
        const std::string& requestId, 
        const Coordinates& coords) = 0;
};