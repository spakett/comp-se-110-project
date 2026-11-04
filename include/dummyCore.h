#pragma once
#include <vector>
#include "IUiInputPort.h"
#include "IUiOutputPort.h"

class DummyCore : public IUiInputPort {
public:
    explicit DummyCore(IUiOutputPort &ui);

    void searchLocation(const std::string &addressQuery) override;
    void requestRoute(const Coordinates &origin, const Coordinates &destination, TravelMode mode) override;
    void setIrritantFilters(const std::vector<Irritant> &irritants) override;
    void requestAreaAirQuality(const BoundingBox &area) override;
    void requestHistoricalData(const Coordinates &location, int daysPast) override;
    void requestWeatherForecast(const Coordinates &coords) override;

private:
    IUiOutputPort &m_ui;
    std::vector<Irritant> m_filters;
};
