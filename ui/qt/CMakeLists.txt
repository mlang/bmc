cmake_minimum_required(VERSION 3.4.0)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
list(APPEND CMAKE_PREFIX_PATH /usr/local/Cellar/qt5/5.5.1_2)
find_package(Qt5 COMPONENTS Core Multimedia Svg Widgets Xml)
if(Qt5_FOUND)
  qt5_add_resources(RESOURCES app.qrc)
  configure_file(config.h.in config.h)
  add_executable(bmc-ui MACOSX_BUNDLE WIN32
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
  install(TARGETS bmc-ui
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
  )
  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/bmc-ui")
  if(APPLE)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/bmc-ui.app")
  elseif(MSVC)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/bmc-ui.exe")
  endif()
  set(DIRS ${QT_LIBRARY_DIRS})
  install(CODE "
    include(BundleUtilities)
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")"
    COMPONENT Runtime
  )
endif()

