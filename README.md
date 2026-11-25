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
MinGW compiler is used that comes with the Qt installer.

### Install Qr Creator
1. Download Qt and Qt Creator IDE 
2. Select following components from Qt:
- MinGW 64-bit
- Additional Libraries -> Qt Location
- Additional Libraries -> Qt Positioning

### Install CMake
Ensure that CMake is installed and added to the system PATH (it should be at C:\Program Files\CMake\bin\cmake.exe)

### Install SQLite3 with MSYS2
1. Download from <https://www.msys2.org/> and keep the default folder `C:\msys64`
2. Open **MSYS2 MINGW64** shell and start installing SQLite3 there. Run:
```bash
pacman -S mingw-w64-x86_64-sqlite3
```
3. Ensure that you already got these files:
```
C:\msys64\mingw64\include\sqlite3.h
C:\msys64\mingw64\lib\libsqlite3.dll.a
```

### Configure project in Qt Creator
1. On **Configure Project**, select **Desktop Qt 6.12.0 MinGW 64-bit**
2. Choose **Projects** from the left sidebar, then select **Build Settings** tab
3. Find the **Additonal CMake options** field and fill in with this line to add them as CMake variables:
```
-DSQLite3_INCLUDE_DIR:PATH=C:/msys64/mingw64/include -DSQLite3_LIBRARY:FILEPATH=C:/msys64/mingw64/lib/libsqlite3.dll.a
```
4. Build and Run the project


## Tests

The API-handler regression test is registered with CTest. More tests will follow in the final submission.

```bash
ctest --test-dir build --output-on-failure
```