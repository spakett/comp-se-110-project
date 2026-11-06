#include "mapViewModel.h"

MapViewModel::MapViewModel(QObject* parent) : QObject(parent) {}

void MapViewModel::reportViewport(double north, double south, double west, double east) {
    emit viewportChanged(north, south, west, east);
}

QColor MapViewModel::colorForLevel(int level) const {
    return HeatmapModel::colorForLevel(static_cast<AirQualityLevel>(level));
}
