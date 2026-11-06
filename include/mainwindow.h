#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "IUiInputPort.h"
#include "IUiOutputPort.h"
#include "mapViewModel.h"

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
    void createIrritantCheckBoxes();
    void onIrritantSelectionChanged();
    void onViewportChanged(double north, double south, double west, double east);
    void requestHeatmap();

    Ui::MainWindow *ui;
    MapViewModel m_mapViewModel;
    IUiInputPort *m_core = nullptr;

    std::vector<Irritant> m_selectedIrritants;
    BoundingBox m_viewport{};
    bool m_hasViewport = false;
};
#endif // MAINWINDOW_H
