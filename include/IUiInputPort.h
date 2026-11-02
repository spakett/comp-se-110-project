#pragma once
#include <string>
#include <vector>
#include "DomainModels.h"

// The UI uses this to trigger Core use cases
class IUiInputPort {
public:
    virtual ~IUiInputPort() = default;

    virtual void searchLocation(const std::string& addressQuery) = 0;
    virtual void requestRoute(const Coordinates& origin, const Coordinates& destination, TravelMode mode) = 0;
    virtual void setIrritantFilters(const std::vector<Irritant>& irritants) = 0;
    virtual void requestAreaAirQuality(const BoundingBox& area) = 0;
    virtual void requestHistoricalData(const Coordinates& location, int daysPast) = 0;
};