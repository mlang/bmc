cmake_minimum_required(VERSION 2.8.11)
project(bmc-ui)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)

qt5_wrap_ui(UI_HEADERS mainwindow.ui)
add_executable(bmc-ui app.cpp mainwindow.cpp mainwindow.h ${UI_HEADERS})
target_link_libraries(bmc-ui Qt5::Widgets)

