#include "core.h"

Core::Core(IUiOutputPort& uiOutput, IApiHandlerPort& apiHandler)
    : m_uiOutput(uiOutput)
    , m_apiHandler(apiHandler) {
}

void Core::searchLocation(const std::string&) {
    m_uiOutput.displayError("Location search is not yet implemented");
}

void Core::requestRoute(const Coordinates&, const Coordinates&, TravelMode) {
    m_uiOutput.displayError("Routing is not yet implemented");
}

void Core::setIrritantFilters(const std::vector<Irritant>& irritants) {
    m_selectedIrritants = irritants;
}

void Core::requestAreaAirQuality(const BoundingBox& area) {
    m_apiHandler.requestAreaAirQuality(nextRequestId(), area, m_selectedIrritants);
}

void Core::requestHistoricalData(const Coordinates&, int) {
    m_uiOutput.displayError("Historical data is not yet implemented");
}

void Core::requestWeatherForecast(const Coordinates&) {
    m_uiOutput.displayError("Weather forecasts are not yet implemented");
}

void Core::onAreaAirQualityFound(const std::string&, const std::vector<AirQualityData>& data) {
    m_uiOutput.displayAirQualityHeatmap(data);
}

void Core::onApiError(const std::string&, const std::string& message) {
    m_uiOutput.displayError(message);
}

std::string Core::nextRequestId() {
    return std::to_string(++m_requestCounter);
}