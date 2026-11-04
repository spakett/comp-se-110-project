#pragma once
#include <QAbstractListModel>
#include <QColor>
#include <QtQml/qqmlregistration.h>
#include <vector>
#include "airQualityLevels.h"

struct HeatmapCell {
    double north;
    double south;
    double west;
    double east;
    AirQualityLevel level;
};

class HeatmapModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by MapViewModel")

public:
    enum Role { NorthRole = Qt::UserRole + 1, SouthRole, WestRole, EastRole, ColorRole };

    explicit HeatmapModel(QObject* parent = nullptr);

    void setCells(std::vector<HeatmapCell> cells);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    static QColor colorForLevel(AirQualityLevel level);

    static std::vector<HeatmapCell> cellsFromAirQuality(const std::vector<AirQualityData>& samples,
                                                        const std::vector<Irritant>& selected);

private:
    std::vector<HeatmapCell> m_cells;
};
