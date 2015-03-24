cmake_minimum_required(VERSION 2.8.11)
project(bmc-ui)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
add_definitions(-std=c++11)
qt5_add_resources(RESOURCES app.qrc)
add_executable(bmc-ui app.cpp mainwindow.cpp mainwindow.h ${RESOURCES})
target_link_libraries(bmc-ui Qt5::Widgets Qt5::Multimedia)

