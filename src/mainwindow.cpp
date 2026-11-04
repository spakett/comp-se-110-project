#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QQuickWidget>
#include "airQualityLevels.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->mapWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->mapWidget->setInitialProperties({{"viewModel", QVariant::fromValue(&m_mapViewModel)}});
    ui->mapWidget->loadFromModule("CommutePlanner", "HeatmapMap");

    connect(&m_mapViewModel, &MapViewModel::viewportChanged, this, &MainWindow::onViewportChanged);
    connect(ui->zoomInButton, &QPushButton::clicked, this, [this] { emit m_mapViewModel.zoomRequested(1); });
    connect(ui->zoomOutButton, &QPushButton::clicked, this, [this] { emit m_mapViewModel.zoomRequested(-1); });

    createIrritantCheckBoxes();
}

MainWindow::~MainWindow()
{
    delete ui->mapWidget;
    delete ui;
}

void MainWindow::setInputPort(IUiInputPort *core)
{
    m_core = core;
}

void MainWindow::createIrritantCheckBoxes()
{
    for (Irritant irritant : allIrritants()) {
        auto *checkBox = new QCheckBox(QString::fromStdString(irritantDisplayName(irritant)));
        checkBox->setProperty("irritant", static_cast<int>(irritant));
        connect(checkBox, &QCheckBox::toggled, this, &MainWindow::onIrritantSelectionChanged);
        ui->irritantLayout->addWidget(checkBox);
    }
}

void MainWindow::onIrritantSelectionChanged()
{
    m_selectedIrritants.clear();
    for (const QCheckBox *checkBox : ui->irritantGroup->findChildren<QCheckBox *>()) {
        if (checkBox->isChecked())
            m_selectedIrritants.push_back(static_cast<Irritant>(checkBox->property("irritant").toInt()));
    }
    if (m_core)
        m_core->setIrritantFilters(m_selectedIrritants);
    requestHeatmap();
}

void MainWindow::onViewportChanged(double north, double south, double west, double east)
{
    m_viewport = BoundingBox{{north, west}, {south, east}};
    m_hasViewport = true;
    requestHeatmap();
}

void MainWindow::requestHeatmap()
{
    if (m_selectedIrritants.empty()) {
        m_mapViewModel.heatmap()->setCells({});
        return;
    }
    if (m_core && m_hasViewport)
        m_core->requestAreaAirQuality(m_viewport);
}

void MainWindow::displayAirQualityHeatmap(const std::vector<AirQualityData> &areaData)
{
    m_mapViewModel.heatmap()->setCells(HeatmapModel::cellsFromAirQuality(areaData, m_selectedIrritants));
}

void MainWindow::displayError(const std::string &message)
{
    statusBar()->showMessage(QString::fromStdString(message), 5000);
}

void MainWindow::displayLocations(const std::vector<Location> &) {}
void MainWindow::displayRoutes(const std::vector<RouteData> &) {}
void MainWindow::displayHistoricalData(const std::vector<AirQualityData> &) {}
void MainWindow::displayWeatherForecast(const WeatherData &) {}
