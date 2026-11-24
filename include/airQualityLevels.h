#pragma once
#include <array>
#include <optional>
#include <string>
#include <vector>
#include "domainModels.h"

constexpr double kGridStepLat = 0.1;
constexpr double kGridStepLon = 0.1;

const std::vector<Irritant>& allIrritants();

std::string irritantDisplayName(Irritant irritant);

std::string levelDisplayName(AirQualityLevel level);

const std::array<double, 5>& levelThresholds(Irritant irritant);

AirQualityLevel levelForReading(Irritant irritant, double value);

std::optional<AirQualityLevel> worstLevel(const AirQualityData& data, const std::vector<Irritant>& selected);