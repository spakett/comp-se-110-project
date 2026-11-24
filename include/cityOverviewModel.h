#pragma once
#include <QAbstractListModel>
#include <QColor>
#include <QtQml/qqmlregistration.h>
#include <map>
#include <string>
#include <vector>
#include "airQualityLevels.h"

struct CityMarker {
    std::string name;
    double latitude;
    double longitude;
    int value;
    QColor color;
    AirQualityLevel grade;
    std::map<Irritant, double> readings;
    std::map<Irritant, AirQualityLevel> levels;
};

class CityOverviewModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by CityViewModel")

public:
    enum Role { LatitudeRole = Qt::UserRole + 1, LongitudeRole, ValueRole, ColorRole };

    explicit CityOverviewModel(QObject* parent = nullptr);

    void setCities(std::vector<CityMarker> cities);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<CityMarker> m_cities;
};
