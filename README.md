# COMP.SE.110 Project: Commute Planner

A commute planner that forms an air-quality and irritant heatmap and uses that to plan a health-optimised commuter route.

The current implementation covers heatmap display only and the other parts (location search, route planning, historical data, and weather forecasts) will be implemented later. See project documentation in the folder 'docs' for a detailed breakdown of the applications architecture. 

## Requirements

- CMake and Ninja
- Qt 6.12 or newer, with Core, Widgets, Quick, QuickWidgets, Location, Positioning, and Network modules
- SQLite3 development library and headers

(If Qt is installed outside the system package paths, pass the Qt kit's CMake prefix to CMake as shown below.)

## Linux

On linux (e.g. Fedora), the system packages can be installed with:

```bash
sudo dnf install gcc-c++ cmake ninja-build sqlite-devel \
	qt6-qtbase-devel qt6-qtdeclarative-devel \
	qt6-qtlocation-devel qt6-qtpositioning-devel
```

The Qt packages must provide Qt 6.12 or newer. If the Fedora release provides an older version, install a newer Qt kit and set `CMAKE_PREFIX_PATH` to its `gcc_64` directory. For example:

```bash
cmake -S . -B build -G Ninja \
	-DCMAKE_PREFIX_PATH="$HOME/Qt/6.12.0/gcc_64" \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

Run the application from the repository root:

```bash
./build/bin/CommutePlanner
```

## Windows (Using Qt Creator)
1. Install Qr Creator and make sure it has 6.12+ kit installed
2. Open the project's CMakeList as a new project and select Desktop GCC kit with Ninja.
3. Build and Run the project.

## Tests

The API-handler regression test is registered with CTest. More tests will follow in the final submission.

```bash
ctest --test-dir build --output-on-failure
```