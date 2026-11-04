#pragma once
#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "heatmapModel.h"

class MapViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by MainWindow")

    Q_PROPERTY(HeatmapModel* heatmap READ heatmap CONSTANT)

public:
    explicit MapViewModel(QObject* parent = nullptr);

    HeatmapModel* heatmap() { return &m_heatmap; }

    Q_INVOKABLE void reportViewport(double north, double south, double west, double east);

    Q_INVOKABLE QColor colorForLevel(int level) const;

signals:
    void viewportChanged(double north, double south, double west, double east);
    void zoomRequested(int steps);

private:
    HeatmapModel m_heatmap;
};
