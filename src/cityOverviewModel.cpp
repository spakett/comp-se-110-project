#include "cityOverviewModel.h"

CityOverviewModel::CityOverviewModel(QObject* parent) : QAbstractListModel(parent) {}

void CityOverviewModel::setCities(std::vector<CityMarker> cities) {
    beginResetModel();
    m_cities = std::move(cities);
    endResetModel();
}

int CityOverviewModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_cities.size());
}

QVariant CityOverviewModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_cities.size()))
        return {};

    const CityMarker& city = m_cities[index.row()];
    switch (role) {
    case LatitudeRole: return city.latitude;
    case LongitudeRole: return city.longitude;
    case ValueRole: return city.value;
    case ColorRole: return city.color;
    default: return {};
    }
}

QHash<int, QByteArray> CityOverviewModel::roleNames() const {
    return {{LatitudeRole, "latitude"},
            {LongitudeRole, "longitude"},
            {ValueRole, "value"},
            {ColorRole, "markerColor"}};
}
