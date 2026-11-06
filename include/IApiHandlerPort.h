#pragma once

#include <string>
#include <vector>

#include "domainModels.h"

class IApiHandlerCallbackPort {
public:
    virtual ~IApiHandlerCallbackPort() = default;

    virtual void onLocationsFound(const std::string& requestId, const std::vector<Location>& locations) = 0;
    virtual void onRoutesFound(const std::string& requestId, const std::vector<RouteData>& routes) = 0;
    virtual void onAreaAirQualityFound(const std::string& requestId, const std::vector<AirQualityData>& data) = 0;
    virtual void onHistoricalDataFound(const std::string& requestId, const std::vector<AirQualityData>& data) = 0;
    virtual void onWeatherForecastFound(const std::string& requestId, const WeatherData& weather) = 0;
    virtual void onApiError(const std::string& requestId, const std::string& message) = 0;
};

class IApiHandlerPort {
public:
    virtual ~IApiHandlerPort() = default;

    virtual void searchLocation(const std::string& requestId, const std::string& addressQuery) = 0;
    virtual void requestRoute(
        const std::string& requestId,
        const Coordinates& origin,
        const Coordinates& destination,
        TravelMode mode
    ) = 0;

    virtual void requestAreaAirQuality(
        const std::string& requestId,
        const BoundingBox& area,
        const std::vector<Irritant>& irritants
    ) = 0;
    
    virtual void requestHistoricalData(
        const std::string& requestId,
        const Coordinates& location,
        int daysPast,
        const std::vector<Irritant>& irritants
    ) = 0;
    
    virtual void requestWeatherForecast(
        const std::string& requestId, 
        const Coordinates& coords) = 0;
};