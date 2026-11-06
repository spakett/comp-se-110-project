cmake_minimum_required(VERSION 3.21)

# Export compile_commands for IntelliSense/Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CommutePlanner LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 6.12 REQUIRED COMPONENTS Core Widgets Quick QuickWidgets Location Positioning Network)

# Find SQLite3
find_package(SQLite3 REQUIRED)

qt_standard_project_setup(REQUIRES 6.12)
if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

# Help Qt Creator and CMake locate the UI form.
set(CMAKE_AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

qt_add_executable(CommutePlanner
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    include/mainwindow.h
    ui/mainwindow.ui
    include/airQualityLevels.h
    src/airQualityLevels.cpp
    include/dummyCore.h
    src/dummyCore.cpp
    include/QtNetworkClient.h
    src/QtNetworkClient.cpp
)

qt_add_qml_module(CommutePlanner
    URI CommutePlanner
    VERSION 1.0
    QML_FILES
        qml/HeatmapMap.qml
    SOURCES
        include/mapViewModel.h
        src/mapViewModel.cpp
        include/heatmapModel.h
        src/heatmapModel.cpp
)

# Make project headers available to source files.
target_include_directories(CommutePlanner
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(CommutePlanner
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Quick
        Qt::QuickWidgets
        Qt::Location
        Qt::Positioning
        Qt::Network
        SQLite3::SQLite3
)

include(GNUInstallDirs)

install(TARGETS CommutePlanner
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET CommutePlanner
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
