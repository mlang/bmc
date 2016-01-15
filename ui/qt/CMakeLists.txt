cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED Core Multimedia Svg Widgets Xml)
qt5_add_resources(RESOURCES app.qrc)
add_executable(bmc-ui
  app.cpp
  mainwindow.cpp mainwindow.h
  lilypondsvgwidget.cpp lilypondsvgwidget.h
  lilypondsvgcontainer.h
  optionsdialog.cpp optionsdialog.h
  ${RESOURCES}
)
target_compile_features(bmc-ui PRIVATE cxx_range_for)
target_link_libraries(bmc-ui
  Qt5::Widgets Qt5::Multimedia Qt5::Svg Qt5::Xml
  braillemusic-static
)
install(TARGETS bmc-ui DESTINATION bin)

