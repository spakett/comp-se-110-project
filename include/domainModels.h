#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

// Represents a geographical point
struct Coordinates {
    double latitude;
    double longitude;
};

// Represents a geographical bounding box
struct BoundingBox {
    Coordinates topLeft;
    Coordinates bottomRight;
};

// Travel modes
enum class TravelMode {
    Auto,
    Bicycle,
    Pedestrian
};

// Represents the health-related irritants a user can filter
enum class Irritant {
    PM2_5, PM10, CarbonMonoxide, CarbonDioxide, NitrogenDioxide,
    SulphurDioxide, Ozone, Methane, Ammonia, AlderPollen,
    BirchPollen, GrassPollen, OlivePollen, MugwortPollen,
    RagweedPollen, Dust, UvIndex
};

enum class AirQualityLevel { Good, Fair, Moderate, Poor, VeryPoor, ExtremelyPoor };

// Air quality metrics mapped by Irritant
struct AirQualityData {
    std::string timestamp;
    std::map<Irritant, double> readings;
    std::map<Irritant, AirQualityLevel> levels;
    std::vector<Coordinates> points; // Added for heatmap rendering
};

// Represents a single step in a route
struct Maneuver {
    std::string instruction;
    double lengthKm;
    double timeSeconds;
};

// Suggested route parsed from Valhalla API
struct RouteData {
    double totalCost;
    double totalLengthKm;
    double totalTimeSeconds;
    std::vector<Maneuver> maneuvers;
    std::vector<Coordinates> polyline; // Added for map rendering
    std::optional<AirQualityLevel> airQuality; // Worst level along the route, empty if unknown
};
