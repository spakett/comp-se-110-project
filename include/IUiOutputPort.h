#pragma once
#include <vector>
#include <string>
#include "domainModels.h"

// Core uses this to push processed data to the UI
class IUiOutputPort {
public:
    virtual ~IUiOutputPort() = default;

    virtual void displayRoutes(const std::vector<RouteData>& routes) = 0;
    virtual void displayAirQualityHeatmap(const std::vector<AirQualityData>& areaData) = 0;
    virtual void displayError(const std::string& message) = 0;
};
