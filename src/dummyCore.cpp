#include "dummyCore.h"

#include <algorithm>
#include <cmath>
#include "airQualityLevels.h"

namespace {

const Coordinates kHelsinki{60.17, 24.94};

double distanceKm(const Coordinates &a, const Coordinates &b) {
    const double dLat = (a.latitude - b.latitude) * 111.0;
    const double dLon = (a.longitude - b.longitude) * 55.0;
    return std::sqrt(dLat * dLat + dLon * dLon);
}

}

DummyCore::DummyCore(IUiOutputPort &ui) : m_ui(ui) {}

void DummyCore::setIrritantFilters(const std::vector<Irritant> &irritants) {
    m_filters = irritants;
}

void DummyCore::requestAreaAirQuality(const BoundingBox &area) {
    const double north = area.topLeft.latitude;
    const double south = area.bottomRight.latitude;
    const double west = area.topLeft.longitude;
    const double east = area.bottomRight.longitude;

    if (north - south > 3.0 || east - west > 6.0) {
        m_ui.displayError("Zoom in to see the heatmap");
        m_ui.displayAirQualityHeatmap({});
        return;
    }

    std::vector<AirQualityData> samples;
    for (double lat = std::floor(south / kGridStepLat) * kGridStepLat; lat < north + kGridStepLat; lat += kGridStepLat) {
        for (double lon = std::floor(west / kGridStepLon) * kGridStepLon; lon < east + kGridStepLon; lon += kGridStepLon) {
            const Coordinates point{lat, lon};
            const double closeness = std::max(0.0, 1.0 - distanceKm(point, kHelsinki) / 25.0);

            AirQualityData sample{"", {}, {point}};
            for (Irritant irritant : m_filters)
                sample.readings[irritant] = levelThresholds(irritant)[4] * closeness;
            samples.push_back(sample);
        }
    }
    m_ui.displayAirQualityHeatmap(samples);
}

void DummyCore::searchLocation(const std::string &) {
    m_ui.displayError("Location search is not implemented yet");
}

void DummyCore::requestRoute(const Coordinates &, const Coordinates &, TravelMode) {
    m_ui.displayError("Routing is not implemented yet");
}

void DummyCore::requestHistoricalData(const Coordinates &, int) {
    m_ui.displayError("Historical data is not implemented yet");
}

void DummyCore::requestWeatherForecast(const Coordinates &) {
    m_ui.displayError("Weather forecast is not implemented yet");
}
