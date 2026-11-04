#pragma once
#include <string>
#include <vector>
#include <map>

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

// Represents a location
struct Location {
    std::string displayName;
    Coordinates coords;
    BoundingBox boundingBox;
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

// Air quality metrics mapped by Irritant
struct AirQualityData {
    std::string timestamp;
    std::map<Irritant, double> readings;
    std::vector<Coordinates> points; // Added for heatmap rendering
};

// Parsed environmental data
struct WeatherData {
    std::string timestamp;
    double temperature;
    double apparentTemperature;
    double relativeHumidity;
    double maxUvIndex;
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
};
