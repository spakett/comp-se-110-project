#include "routeViewModel.h"
#include <QGeoCoordinate>

RouteViewModel::RouteViewModel(QObject* parent) : QObject(parent) {}

void RouteViewModel::setRoute(const std::vector<Coordinates>& polyline) {
    m_routePath.clear();
    for (const Coordinates& point : polyline)
        m_routePath.append(QVariant::fromValue(QGeoCoordinate(point.latitude, point.longitude)));
    emit routePathChanged();
}

void RouteViewModel::pickPoint(double latitude, double longitude) {
    const QGeoCoordinate coord(latitude, longitude);
    if (!m_start.isValid() || m_destination.isValid()) {
        m_start = coord;
        m_destination = QGeoCoordinate();
    } else {
        m_destination = coord;
    }
    emit pointsChanged();
}

void RouteViewModel::reportViewport(double north, double south, double west, double east) {
    emit viewportChanged(north, south, west, east);
}

QColor RouteViewModel::colorForLevel(int level) const {
    return HeatmapModel::colorForLevel(static_cast<AirQualityLevel>(level));
}
