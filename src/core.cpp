#include "core.h"
#include <algorithm>
#include "airQualityLevels.h"

Core::Core(IUiOutputPort& uiOutput, IApiHandlerPort& apiHandler)
    : m_uiOutput(uiOutput)
    , m_apiHandler(apiHandler) {
}

void Core::searchLocation(const std::string&) {
    m_uiOutput.displayError("Location search is not yet implemented");
}

void Core::requestRoute(const Coordinates& origin, const Coordinates& destination, TravelMode mode) {
    m_latestRouteRequestId = nextRequestId();
    m_apiHandler.requestRoutes(m_latestRouteRequestId, origin, destination, mode);
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
    m_latestAirQuality = data;
    m_uiOutput.displayAirQualityHeatmap(data);
}

void Core::onApiError(const std::string&, const std::string& message) {
    m_uiOutput.displayError(message);
}

void Core::onRoutesFound(const std::string& requestId, const std::vector<RouteData>& routes) {
    if(requestId != m_latestRouteRequestId)
        return;
    if(routes.empty()) 
        m_uiOutput.displayError("No route found between selected points");
    std::vector<RouteData> scoredRoutes = routes;
    for (RouteData& route : scoredRoutes)
        route.airQuality = routeAirQuality(route, m_latestAirQuality, m_selectedIrritants);
    std::stable_sort(scoredRoutes.begin(), scoredRoutes.end(), [](const RouteData& a, const RouteData& b) {
        if (!b.airQuality)
            return a.airQuality.has_value();
        return a.airQuality && *a.airQuality < *b.airQuality;
    });
    m_uiOutput.displayRoutes(scoredRoutes);
}

std::string Core::nextRequestId() {
    return std::to_string(++m_requestCounter);
}