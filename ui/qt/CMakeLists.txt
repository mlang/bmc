cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED Core Multimedia Svg Widgets Xml)
add_definitions(-std=c++11)
qt5_add_resources(RESOURCES app.qrc)
add_executable(bmc-ui
  app.cpp
  mainwindow.cpp mainwindow.h
  lilypondsvgwidget.cpp lilypondsvgwidget.h
  lilypondsvgcontainer.h
  ${RESOURCES}
)
target_link_libraries(bmc-ui Qt5::Widgets Qt5::Multimedia Qt5::Svg Qt5::Xml bmc_parse-static)

