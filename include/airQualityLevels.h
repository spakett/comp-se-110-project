#pragma once
#include <array>
#include <optional>
#include <string>
#include <vector>
#include "domainModels.h"

enum class AirQualityLevel { Good, Fair, Moderate, Poor, VeryPoor, ExtremelyPoor };

constexpr double kGridStepLat = 0.05;
constexpr double kGridStepLon = 0.1;

const std::vector<Irritant>& allIrritants();

std::string irritantDisplayName(Irritant irritant);

const std::array<double, 5>& levelThresholds(Irritant irritant);

AirQualityLevel levelForReading(Irritant irritant, double value);

std::optional<AirQualityLevel> worstLevel(const AirQualityData& data, const std::vector<Irritant>& selected);
