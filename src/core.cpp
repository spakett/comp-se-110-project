#include "core.h"

Core::Core(IUiOutputPort& uiOutput, IApiHandlerPort& apiHandler)
    : m_uiOutput(uiOutput)
    , m_apiHandler(apiHandler) {
}

void Core::searchLocation(const std::string& addressQuery) {
    m_apiHandler.searchLocation(nextRequestId(), addressQuery);
}

void Core::requestRoute(const Coordinates& origin, const Coordinates& destination, TravelMode mode) {
    m_apiHandler.requestRoute(nextRequestId(), origin, destination, mode);
}

void Core::setIrritantFilters(const std::vector<Irritant>& irritants) {
    m_selectedIrritants = irritants;
}

void Core::requestAreaAirQuality(const BoundingBox& area) {
    m_apiHandler.requestAreaAirQuality(nextRequestId(), area, m_selectedIrritants);
}

void Core::requestHistoricalData(const Coordinates& location, int daysPast) {
    m_apiHandler.requestHistoricalData(nextRequestId(), location, daysPast, m_selectedIrritants);
}

void Core::requestWeatherForecast(const Coordinates& coords) {
    m_apiHandler.requestWeatherForecast(nextRequestId(), coords);
}

void Core::onLocationsFound(const std::string&, const std::vector<Location>& locations) {
    m_uiOutput.displayLocations(locations);
}

void Core::onRoutesFound(const std::string&, const std::vector<RouteData>& routes) {
    m_uiOutput.displayRoutes(routes);
}

void Core::onAreaAirQualityFound(const std::string&, const std::vector<AirQualityData>& data) {
    m_uiOutput.displayAirQualityHeatmap(data);
}

void Core::onHistoricalDataFound(const std::string&, const std::vector<AirQualityData>& data) {
    m_uiOutput.displayHistoricalData(data);
}

void Core::onWeatherForecastFound(const std::string&, const WeatherData& weather) {
    m_uiOutput.displayWeatherForecast(weather);
}

void Core::onApiError(const std::string&, const std::string& message) {
    m_uiOutput.displayError(message);
}

std::string Core::nextRequestId() {
    return std::to_string(++m_requestCounter);
}