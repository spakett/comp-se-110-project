#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QListWidget>
#include <QQuickWidget>
#include <QStackedWidget>
#include "airQualityLevels.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    resize(1100, 720);
    setMinimumSize(900, 600);

    ui->mapWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->mapWidget->setInitialProperties({{"viewModel", QVariant::fromValue(&m_mapViewModel)}});
    ui->mapWidget->loadFromModule("CommutePlanner", "HeatmapMap");

    ui->overviewWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->overviewWidget->setInitialProperties({{"viewModel", QVariant::fromValue(&m_cityViewModel)}});
    ui->overviewWidget->loadFromModule("CommutePlanner", "CityOverviewMap");

    ui->routeWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->routeWidget->setInitialProperties({{"viewModel", QVariant::fromValue(&m_routeViewModel)}});
    ui->routeWidget->loadFromModule("CommutePlanner", "RouteMap");

    auto makeCity = [](std::string name, double lat, double lon, int value,
                       std::map<Irritant, double> readings) -> CityMarker {
        std::map<Irritant, AirQualityLevel> levels;
        for (const auto &[irritant, reading] : readings)
            levels.emplace(irritant, levelForReading(irritant, reading));
        AirQualityData sample;
        sample.levels = levels;
        const AirQualityLevel grade =
            worstLevel(sample, allIrritants()).value_or(AirQualityLevel::Good);
        return CityMarker{std::move(name), lat, lon, value, HeatmapModel::colorForLevel(grade),
                          grade, std::move(readings), std::move(levels)};
    };

    m_cities = {
        makeCity("Helsinki", 60.17, 24.94, 42,
                 {{Irritant::PM2_5, 14}, {Irritant::PM10, 30}, {Irritant::NitrogenDioxide, 35},
                  {Irritant::Ozone, 45}, {Irritant::BirchPollen, 8}, {Irritant::GrassPollen, 20}}),
        makeCity("Tampere", 61.50, 23.79, 31,
                 {{Irritant::PM2_5, 6}, {Irritant::PM10, 15}, {Irritant::NitrogenDioxide, 20},
                  {Irritant::Ozone, 30}, {Irritant::BirchPollen, 5}, {Irritant::GrassPollen, 8}}),
        makeCity("Turku", 60.45, 22.27, 55,
                 {{Irritant::PM2_5, 22}, {Irritant::PM10, 35}, {Irritant::NitrogenDioxide, 30},
                  {Irritant::Ozone, 60}, {Irritant::BirchPollen, 12}, {Irritant::GrassPollen, 15}}),
        makeCity("Oulu", 65.01, 25.47, 23,
                 {{Irritant::PM2_5, 5}, {Irritant::PM10, 12}, {Irritant::NitrogenDioxide, 15},
                  {Irritant::Ozone, 25}, {Irritant::BirchPollen, 3}, {Irritant::GrassPollen, 4}}),
        makeCity("Jyväskylä", 62.24, 25.75, 48,
                 {{Irritant::PM2_5, 12}, {Irritant::PM10, 25}, {Irritant::NitrogenDioxide, 50},
                  {Irritant::Ozone, 70}, {Irritant::BirchPollen, 9}, {Irritant::GrassPollen, 30}}),
        makeCity("Joensuu", 62.60, 29.76, 67,
                 {{Irritant::PM2_5, 30}, {Irritant::PM10, 60}, {Irritant::NitrogenDioxide, 100},
                  {Irritant::Ozone, 90}, {Irritant::BirchPollen, 40}, {Irritant::GrassPollen, 60}}),
    };

    m_cityViewModel.cities()->setCities(m_cities);
    updateCityPanel();

    connect(&m_mapViewModel, &MapViewModel::viewportChanged, this, &MainWindow::onViewportChanged);
    connect(&m_cityViewModel, &CityViewModel::citySelected, this, &MainWindow::onCitySelected);
    connect(&m_routeViewModel, &RouteViewModel::viewportChanged, this, &MainWindow::onRouteViewportChanged);
    connect(&m_routeViewModel, &RouteViewModel::pointsChanged, this, &MainWindow::onRoutePointsChanged);
    connect(ui->findRoutesButton, &QPushButton::clicked, this, &MainWindow::onFindRoutes);
    connect(ui->routeList, &QListWidget::currentRowChanged, this, &MainWindow::onRouteSelected);
    connect(ui->pageStack, &QStackedWidget::currentChanged, this, &MainWindow::refreshVisibleHeatmap);

    ui->navList->setCurrentRow(0);

    createIrritantCheckBoxes();
}

MainWindow::~MainWindow()
{
    delete ui->mapWidget;
    delete ui->overviewWidget;
    delete ui->routeWidget;
    delete ui;
}

void MainWindow::setInputPort(IUiInputPort *core)
{
    m_core = core;
}

void MainWindow::createIrritantCheckBoxes()
{
    int index = 0;
    for (Irritant irritant : allIrritants()) {
        auto *checkBox = new QCheckBox(QString::fromStdString(irritantDisplayName(irritant)));
        checkBox->setProperty("irritant", static_cast<int>(irritant));
        connect(checkBox, &QCheckBox::toggled, this, &MainWindow::onIrritantSelectionChanged);
        ui->irritantGrid->addWidget(checkBox, index / 4, index % 4);
        ++index;
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
    updateCityPanel();
    refreshVisibleHeatmap();
}

void MainWindow::refreshVisibleHeatmap()
{
    const int page = ui->pageStack->currentIndex();
    if (page == HeatmapPage)
        requestHeatmap();
    else if (page == RoutePage)
        requestRouteHeatmap();
}

void MainWindow::onCitySelected(int index)
{
    m_selectedCityIndex = index;
    updateCityPanel();
}

void MainWindow::onRouteViewportChanged(double north, double south, double west, double east)
{
    m_routeViewport = BoundingBox{{north, west}, {south, east}};
    m_hasRouteViewport = true;
    requestRouteHeatmap();
}

void MainWindow::requestRouteHeatmap()
{
    if (m_selectedIrritants.empty()) {
        m_routeViewModel.heatmap()->setCells({});
        return;
    }
    if (m_core && m_hasRouteViewport)
        m_core->requestAreaAirQuality(m_routeViewport);
}

void MainWindow::onFindRoutes()
{
    if (!m_routeViewModel.hasStart() || !m_routeViewModel.hasDestination()) {
        statusBar()->showMessage("Click the map to set a start and a destination", 5000);
        return;
    }
    if (m_core)
        m_core->requestRoute(m_routeViewModel.start(), m_routeViewModel.destination(), selectedTravelMode());
}

void MainWindow::onRoutePointsChanged()
{
    ui->startLabel->setText(m_routeViewModel.hasStart()
        ? QString("Start: %1, %2")
              .arg(m_routeViewModel.start().latitude, 0, 'f', 4)
              .arg(m_routeViewModel.start().longitude, 0, 'f', 4)
        : QStringLiteral("Start: click the map"));
    ui->destLabel->setText(m_routeViewModel.hasDestination()
        ? QString("Destination: %1, %2")
              .arg(m_routeViewModel.destination().latitude, 0, 'f', 4)
              .arg(m_routeViewModel.destination().longitude, 0, 'f', 4)
        : QStringLiteral("Destination: click the map"));
}

TravelMode MainWindow::selectedTravelMode() const
{
    if (ui->bikeRadio->isChecked())
        return TravelMode::Bicycle;
    if (ui->walkRadio->isChecked())
        return TravelMode::Pedestrian;
    return TravelMode::Auto;
}

void MainWindow::onRouteSelected(int row)
{
    if (row < 0 || row >= static_cast<int>(m_routes.size()))
        return;
    m_routeViewModel.setRoute(m_routes[row].polyline);
}

void MainWindow::updateCityPanel()
{
    while (QLayoutItem *item = ui->cityPollutantsLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    if (m_selectedCityIndex < 0 || m_selectedCityIndex >= static_cast<int>(m_cities.size())) {
        ui->cityNameLabel->setText("Select a city");
        ui->cityGradeLabel->clear();
        ui->cityGradeLabel->setStyleSheet("");
        return;
    }

    const CityMarker &city = m_cities[m_selectedCityIndex];
    ui->cityNameLabel->setText(QString::fromStdString(city.name));

    ui->cityGradeLabel->setText(
        QString("Overall grade: %1").arg(QString::fromStdString(levelDisplayName(city.grade))));
    ui->cityGradeLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(city.color.name()));

    if (m_selectedIrritants.empty()) {
        auto *hint = new QLabel("No pollutants selected.");
        hint->setWordWrap(true);
        ui->cityPollutantsLayout->addWidget(hint);
        return;
    }

    for (Irritant irritant : m_selectedIrritants) {
        const auto reading = city.readings.find(irritant);
        const auto level = city.levels.find(irritant);
        const QString name = QString::fromStdString(irritantDisplayName(irritant));
        QString valueText = QStringLiteral("—");
        QColor swatchColor = Qt::gray;
        if (reading != city.readings.end())
            valueText = QString::number(reading->second, 'f', 1);
        if (level != city.levels.end())
            swatchColor = HeatmapModel::colorForLevel(level->second);

        auto *row = new QWidget;
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        auto *swatch = new QFrame;
        swatch->setFixedSize(14, 14);
        swatch->setStyleSheet(QString("background-color: %1;").arg(swatchColor.name()));

        auto *label = new QLabel(QString("%1: %2").arg(name, valueText));

        rowLayout->addWidget(swatch);
        rowLayout->addWidget(label);
        rowLayout->addStretch();
        ui->cityPollutantsLayout->addWidget(row);
    }
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
    HeatmapModel *target = ui->pageStack->currentIndex() == RoutePage
        ? m_routeViewModel.heatmap()
        : m_mapViewModel.heatmap();
    target->setCells(HeatmapModel::cellsFromAirQuality(areaData, m_selectedIrritants));
}

void MainWindow::displayError(const std::string &message)
{
    statusBar()->showMessage(QString::fromStdString(message), 5000);
}

void MainWindow::displayRoutes(const std::vector<RouteData> &routes)
{
    m_routes = routes;
    ui->routeList->clear();
    m_routeViewModel.setRoute({});
    for (std::size_t i = 0; i < m_routes.size(); ++i) {
        const RouteData &route = m_routes[i];
        const QString air = route.airQuality
            ? QString::fromStdString(levelDisplayName(*route.airQuality))
            : QStringLiteral("unknown");
        ui->routeList->addItem(QStringLiteral("Route %1 — %2 km, %3 min, air: %4")
                                   .arg(i + 1)
                                   .arg(route.totalLengthKm, 0, 'f', 1)
                                   .arg(qRound(route.totalTimeSeconds / 60.0))
                                   .arg(air));
    }
}
