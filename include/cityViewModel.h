#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "cityOverviewModel.h"

class CityViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by MainWindow")

    Q_PROPERTY(CityOverviewModel* cities READ cities CONSTANT)

public:
    explicit CityViewModel(QObject* parent = nullptr);

    CityOverviewModel* cities() { return &m_cities; }

    Q_INVOKABLE void selectCity(int index);

signals:
    void citySelected(int index);

private:
    CityOverviewModel m_cities;
};
