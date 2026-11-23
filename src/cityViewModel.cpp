#include "cityViewModel.h"

CityViewModel::CityViewModel(QObject* parent) : QObject(parent) {}

void CityViewModel::selectCity(int index) {
    emit citySelected(index);
}
