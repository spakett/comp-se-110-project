#include "heatmapModel.h"

std::vector<HeatmapCell> HeatmapModel::cellsFromAirQuality(const std::vector<AirQualityData>& samples,
                                                           const std::vector<Irritant>& selected) {
    const double halfLat = kGridStepLat / 2.0;
    const double halfLon = kGridStepLon / 2.0;

    std::vector<HeatmapCell> cells;
    for (const AirQualityData& sample : samples) {
        const std::optional<AirQualityLevel> level = worstLevel(sample, selected);
        if (!level)
            continue;
        for (const Coordinates& p : sample.points) {
            cells.push_back({p.latitude + halfLat, p.latitude - halfLat,
                             p.longitude - halfLon, p.longitude + halfLon, *level});
        }
    }
    return cells;
}

HeatmapModel::HeatmapModel(QObject* parent) : QAbstractListModel(parent) {}

void HeatmapModel::setCells(std::vector<HeatmapCell> cells) {
    beginResetModel();
    m_cells = std::move(cells);
    endResetModel();
}

int HeatmapModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_cells.size());
}

QVariant HeatmapModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_cells.size()))
        return {};

    const HeatmapCell& cell = m_cells[index.row()];
    switch (role) {
    case NorthRole: return cell.north;
    case SouthRole: return cell.south;
    case WestRole: return cell.west;
    case EastRole: return cell.east;
    case ColorRole: {
        QColor color = colorForLevel(cell.level);
        color.setAlphaF(0.4f);
        return color;
    }
    default: return {};
    }
}

QHash<int, QByteArray> HeatmapModel::roleNames() const {
    return {{NorthRole, "north"}, {SouthRole, "south"}, {WestRole, "west"},
            {EastRole, "east"},   {ColorRole, "cellColor"}};
}

QColor HeatmapModel::colorForLevel(AirQualityLevel level) {
    switch (level) {
    case AirQualityLevel::Good: return QColor(0x5b, 0xa8, 0x6b);
    case AirQualityLevel::Fair: return QColor(0xb5, 0xc9, 0x4a);
    case AirQualityLevel::Moderate: return QColor(0xf2, 0xc1, 0x4e);
    case AirQualityLevel::Poor: return QColor(0xef, 0x86, 0x4a);
    case AirQualityLevel::VeryPoor: return QColor(0xe0, 0x50, 0x4a);
    case AirQualityLevel::ExtremelyPoor: return QColor(0x9b, 0x2f, 0x6a);
    }
    return Qt::transparent;
}
