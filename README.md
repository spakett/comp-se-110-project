# comp-se-110-project
Repository for COMP.SE.110's project.
A commute planner that forms an air-quality and irritant heatmap and uses that to plan a health-optimised commuter route.  

# Dependencies
In order to build the project you'll need:
- Compiler: GCC (C++17 or higher)
- Build Tools: CMake, Ninja
- Framework: Qt6
- Libraries: SQLite

# Building
This is an example of the building process (on a Fedora 44 installation. Windows should work the same as long as the dependencies are present or through a WSL instance).

Clone repository
`git clone <repository_url>`
`cd comp-se-110-project`

Create build directory
`mkdir build && cd build`

Configure the project
`cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

Build
`ninja`

# Running the Application
After a successfull build, launch the application from the build directory:
`./CommutePlanner`