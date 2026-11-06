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

    void searchLocation(const std::string& addressQuery) override;
    void requestRoute(const Coordinates& origin, const Coordinates& destination, TravelMode mode) override;
    void setIrritantFilters(const std::vector<Irritant>& irritants) override;
    void requestAreaAirQuality(const BoundingBox& area) override;
    void requestHistoricalData(const Coordinates& location, int daysPast) override;
    void requestWeatherForecast(const Coordinates& coords) override;
    void onAreaAirQualityFound(const std::string& requestId, const std::vector<AirQualityData>& data) override;
    void onApiError(const std::string& requestId, const std::string& message) override;

private:
    std::string nextRequestId();

    IUiOutputPort& m_uiOutput;
    IApiHandlerPort& m_apiHandler;
    std::vector<Irritant> m_selectedIrritants;
    std::uint64_t m_requestCounter = 0;
};