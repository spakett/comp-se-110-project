#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "IUiInputPort.h"
#include "IUiOutputPort.h"
#include "cityViewModel.h"
#include "mapViewModel.h"
#include "routeViewModel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public IUiOutputPort
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setInputPort(IUiInputPort *core);

    void displayLocations(const std::vector<Location> &locations) override;
    void displayRoutes(const std::vector<RouteData> &routes) override;
    void displayAirQualityHeatmap(const std::vector<AirQualityData> &areaData) override;
    void displayHistoricalData(const std::vector<AirQualityData> &historicalData) override;
    void displayWeatherForecast(const WeatherData &weather) override;
    void displayError(const std::string &message) override;

private:
    enum Page { HomePage, HeatmapPage, RoutePage };

    void createIrritantCheckBoxes();
    void onIrritantSelectionChanged();
    void onViewportChanged(double north, double south, double west, double east);
    void requestHeatmap();
    void refreshVisibleHeatmap();
    void onCitySelected(int index);
    void updateCityPanel();
    void onRouteViewportChanged(double north, double south, double west, double east);
    void requestRouteHeatmap();
    void onFindRoutes();
    void onRouteSelected(int row);
    void onRoutePointsChanged();
    TravelMode selectedTravelMode() const;

    Ui::MainWindow *ui;
    MapViewModel m_mapViewModel;
    CityViewModel m_cityViewModel;
    RouteViewModel m_routeViewModel;
    IUiInputPort *m_core = nullptr;

    std::vector<Irritant> m_selectedIrritants;
    BoundingBox m_viewport{};
    bool m_hasViewport = false;

    std::vector<CityMarker> m_cities;
    int m_selectedCityIndex = -1;

    std::vector<RouteData> m_routes;
    BoundingBox m_routeViewport{};
    bool m_hasRouteViewport = false;
};
#endif // MAINWINDOW_H
