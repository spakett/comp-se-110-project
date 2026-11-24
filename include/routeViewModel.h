#pragma once
#include <QColor>
#include <QGeoCoordinate>
#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include <vector>
#include "domainModels.h"
#include "heatmapModel.h"

class RouteViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by MainWindow")

    Q_PROPERTY(QVariantList routePath READ routePath NOTIFY routePathChanged)
    Q_PROPERTY(HeatmapModel* heatmap READ heatmap CONSTANT)
    Q_PROPERTY(QVariant startPoint READ startPoint NOTIFY pointsChanged)
    Q_PROPERTY(QVariant destinationPoint READ destinationPoint NOTIFY pointsChanged)

public:
    explicit RouteViewModel(QObject* parent = nullptr);

    QVariantList routePath() const { return m_routePath; }
    HeatmapModel* heatmap() { return &m_heatmap; }
    QVariant startPoint() const { return QVariant::fromValue(m_start); }
    QVariant destinationPoint() const { return QVariant::fromValue(m_destination); }

    void setRoute(const std::vector<Coordinates>& polyline);

    bool hasStart() const { return m_start.isValid(); }
    bool hasDestination() const { return m_destination.isValid(); }
    Coordinates start() const { return {m_start.latitude(), m_start.longitude()}; }
    Coordinates destination() const { return {m_destination.latitude(), m_destination.longitude()}; }

    Q_INVOKABLE void pickPoint(double latitude, double longitude);
    Q_INVOKABLE void reportViewport(double north, double south, double west, double east);
    Q_INVOKABLE QColor colorForLevel(int level) const;

signals:
    void routePathChanged();
    void viewportChanged(double north, double south, double west, double east);
    void pointsChanged();

private:
    QVariantList m_routePath;
    HeatmapModel m_heatmap;
    QGeoCoordinate m_start;
    QGeoCoordinate m_destination;
};
