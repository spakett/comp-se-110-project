#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "IApiHandlerPort.h"
#include "IUiInputPort.h"
#include "IUiOutputPort.h"

class Core final : public IUiInputPort, public IApiHandlerCallbackPort {
public:
    Core(IUiOutputPort& uiOutput, IApiHandlerPort& apiHandler);

    void requestRoute(const Coordinates& origin, const Coordinates& destination, TravelMode mode) override;
    void setIrritantFilters(const std::vector<Irritant>& irritants) override;
    void requestAreaAirQuality(const BoundingBox& area) override;
    void onAreaAirQualityFound(const std::string& requestId, const std::vector<AirQualityData>& data) override;
    void onApiError(const std::string& requestId, const std::string& message) override;
    void onRoutesFound(const std::string& requestId, const std::vector<RouteData>& routes) override;

private:
    std::string nextRequestId();

    IUiOutputPort& m_uiOutput;
    IApiHandlerPort& m_apiHandler;
    std::vector<Irritant> m_selectedIrritants;
    std::uint64_t m_requestCounter = 0;
    std::string m_latestRouteRequestId;
    std::vector<AirQualityData> m_latestAirQuality;
};