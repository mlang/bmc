cmake_minimum_required(VERSION 3.4.0)
project(bmc VERSION 0.0.1 LANGUAGES CXX)
add_definitions(-DBMC_VERSION="${bmc_VERSION}")
option(bmc_USE_DOXYGEN "Use Doxygen to generate C++ class documentation" OFF)

if(MSVC)
  option(BUILD_STATIC "Build static binary" ON)
  if(BUILD_STATIC)
    message("Building static binary")
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
  endif(BUILD_STATIC)
endif(MSVC)

if(bmc_USE_DOXYGEN)
  add_subdirectory(doxygen)
endif(bmc_USE_DOXYGEN)

option(bmc_USE_PYTHON "Build components that require Python" ON)
set(bmc_REQUIRED_BOOST_COMPONENTS program_options)
if(bmc_USE_PYTHON)
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp)
  set(bmc_REQUIRED_BOOST_COMPONENTS ${bmc_REQUIRED_BOOST_COMPONENTS} python)
endif(bmc_USE_PYTHON)
if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  if(BUILD_STATIC)
    set(Boost_USE_STATIC_RUNTIME ON)
  endif()
endif(MSVC)
find_package(Boost 1.58.0 REQUIRED COMPONENTS ${bmc_REQUIRED_BOOST_COMPONENTS})
find_package(Threads)
add_subdirectory(xsdcxx-musicxml)
include_directories(
  ${bmc_BINARY_DIR}
  ${bmc_SOURCE_DIR}
  ${bmc_SOURCE_DIR}/include
  ${bmc_SOURCE_DIR}/xsdcxx-musicxml
  ${Boost_INCLUDE_DIRS} ${XSDCXX_INCLUDE_DIR}
)

add_subdirectory(lib)

add_executable(bmc bmc.cpp)
target_link_libraries(bmc ${Boost_PROGRAM_OPTIONS_LIBRARY} braillemusic-static)
target_compile_features(bmc PRIVATE cxx_range_for cxx_auto_type)
add_subdirectory(ui)
add_definitions(-DSOUNDFONT_PATH="/usr/share/sounds/sf2/FluidR3_GM.sf2")
#add_executable(bmc main.cpp fluidsynth.cpp)
#find_library(FLUIDSYNTH_LIBRARY fluidsynth)
#target_link_libraries(bmc braillemusic ${FLUIDSYNTH_LIBRARY})

if(bmc_USE_PYTHON)
  include_directories(${PYTHON_INCLUDE_DIR})
  add_library(pybmc SHARED python.cpp)
  set_target_properties(pybmc PROPERTIES
    PREFIX ""
    OUTPUT_NAME "_bmc"
    LIBRARY_OUTPUT_DIRECTORY python/bmc
  )
  target_compile_features(pybmc PRIVATE cxx_auto_type cxx_range_for)
  target_link_libraries(pybmc ${Boost_PYTHON_LIBRARY} braillemusic)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c
            "import site, sys; sys.stdout.write(site.PREFIXES[-1])"
    OUTPUT_VARIABLE PYTHON_PREFIX
  )
  file ( TO_CMAKE_PATH "${PYTHON_PREFIX}" PYTHON_PREFIX )
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c
            "import site, sys; sys.stdout.write(site.getsitepackages()[-1])"
    OUTPUT_VARIABLE PYTHON_SITE_DIR
  )
  file(TO_CMAKE_PATH "${PYTHON_SITE_DIR}" PYTHON_SITE_DIR)
  string(REGEX REPLACE "^${PYTHON_PREFIX}/" "" PYTHON_SITE_DIR "${PYTHON_SITE_DIR}")

  install(TARGETS pybmc
    LIBRARY DESTINATION ${PYTHON_SITE_DIR}/bmc
    COMPONENT library
  )
  install(FILES python/bmc/__init__.py python/bmc/sphinx.py
    DESTINATION ${PYTHON_SITE_DIR}/bmc
    COMPONENT library
  )
endif(bmc_USE_PYTHON)
install(TARGETS bmc DESTINATION bin)

find_package(PkgConfig)
#  pkg_search_module(CGICC cgicc)
#  if(CGICC_FOUND)
#    include_directories(${CGICC_INCLUDE_DIRS})
#    add_executable(bmc.cgi cgi.cpp)
#    target_compile_features(bmc.cgi PRIVATE cxx_range_for cxx_auto_type)
#    target_link_libraries(bmc.cgi braillemusic ${CGICC_LIBRARIES})
#  endif(CGICC_FOUND)

if(bmc_USE_PYTHON)
  add_subdirectory(sphinx)
endif(bmc_USE_PYTHON)
add_subdirectory(test)

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
if(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
endif(APPLE)
set(CPACK_RESOURCE_FILE_LICENSE "${bmc_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${bmc_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${bmc_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${bmc_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT       "Mario Lang <mlang@delysid.org>")
set(CPACK_STRIP_FILES           TRUE)
set(CPACK_SOURCE_GENERATOR      "TGZ;TXZ;")
include(CPack)
