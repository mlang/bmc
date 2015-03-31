cmake_minimum_required(VERSION 2.8)
project(bmc)
set(bmc_VERSION_MAJOR 0)
set(bmc_VERSION_MINOR 0)
set(bmc_VERSION_PATCH 1)
set(bmc_VERSION ${bmc_VERSION_MAJOR}.${bmc_VERSION_MINOR}.${bmc_VERSION_PATCH})
add_definitions(-DBMC_VERSION="${bmc_VERSION}")
option(bmc_USE_DOXYGEN "Use Doxygen to generate C++ class documentation" OFF)
if(bmc_USE_DOXYGEN)
  add_subdirectory(doxygen)
endif(bmc_USE_DOXYGEN)
add_subdirectory(xsdcxx-musicxml)
add_subdirectory(ttb)
add_subdirectory(ui)
option(bmc_USE_PYTHON "Build components that require Python" ON)
set(bmc_REQUIRED_BOOST_COMPONENTS program_options)
if(bmc_USE_PYTHON)
  find_package(PythonLibs REQUIRED)
  set(bmc_REQUIRED_BOOST_COMPONENTS ${bmc_REQUIRED_BOOST_COMPONENTS} python)
endif(bmc_USE_PYTHON)
find_package(Boost 1.54.0 REQUIRED COMPONENTS ${bmc_REQUIRED_BOOST_COMPONENTS})
find_package(Threads)
include_directories(
  ${bmc_BINARY_DIR}
  ${bmc_SOURCE_DIR}
  ${bmc_SOURCE_DIR}/include
  ${bmc_SOURCE_DIR}/xsdcxx-musicxml
  ${Boost_INCLUDE_DIRS}
)
add_definitions(-std=c++11)
add_library(bmc_parse SHARED
            brlsym.cpp music.cpp ast.cpp
            numbers.cpp key_signature.cpp time_signature.cpp
            partial_voice_sign.cpp simile.cpp tuplet_start.cpp
            measure.cpp score.cpp
            value_disambiguation.cpp value_disambiguator.cpp
            lilypond.cpp musicxml.cpp linebreaking.cpp reformat.cpp
           )
set_target_properties(bmc_parse
                      PROPERTIES
                      VERSION ${bmc_VERSION}
                      SOVERSION ${bmc_VERSION_MAJOR}.${bmc_VERSION_MINOR}
                     )
target_link_libraries(bmc_parse bmc_ttb xsdcxx-musicxml ${CMAKE_THREAD_LIBS_INIT})
add_library(bmc_parse-static STATIC
            brlsym.cpp music.cpp ast.cpp
            numbers.cpp key_signature.cpp time_signature.cpp
            partial_voice_sign.cpp simile.cpp tuplet_start.cpp
            measure.cpp score.cpp
            value_disambiguation.cpp value_disambiguator.cpp
            lilypond.cpp musicxml.cpp linebreaking.cpp reformat.cpp
           )
target_link_libraries(bmc_parse-static bmc_ttb-static xsdcxx-musicxml-static ${CMAKE_THREAD_LIBS_INIT})
add_executable(bmc bmc.cpp)
target_link_libraries(bmc ${Boost_PROGRAM_OPTIONS_LIBRARY} bmc_parse)
add_subdirectory(ui)
add_definitions(-DSOUNDFONT_PATH="/usr/share/sounds/sf2/FluidR3_GM.sf2")
#add_executable(bmc main.cpp fluidsynth.cpp)
#find_library(FLUIDSYNTH_LIBRARY fluidsynth)
#target_link_libraries(bmc bmc_parse ${FLUIDSYNTH_LIBRARY})
add_definitions(-DHAVE_ICONV_H=1)
add_definitions(-DHAVE_WCHAR_H=1)
add_definitions(-DTABLES_DIRECTORY="${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/Tables")

add_executable(bmc_test test.cpp)
set_target_properties(bmc_test
                      PROPERTIES
                      COMPILE_DEFINITIONS CMAKE_SOURCE_DIR="${PROJECT_SOURCE_DIR}/"
                     )
target_link_libraries(bmc_test bmc_parse)
if(bmc_USE_PYTHON)
  include_directories(${PYTHON_INCLUDE_DIR})
  add_library(pybmc SHARED python.cpp)
  set_target_properties(pybmc PROPERTIES
    PREFIX ""
    OUTPUT_NAME "_bmc"
    LIBRARY_OUTPUT_DIRECTORY python/bmc
  )
  target_link_libraries(pybmc ${Boost_PYTHON_LIBRARY} bmc_parse)
endif(bmc_USE_PYTHON)
find_package(Boost QUIET COMPONENTS unit_test_framework)
if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  get_target_property(bmc_test_cflags bmc_test COMPILE_DEFINITIONS)
  set_target_properties(bmc_test
                        PROPERTIES COMPILE_DEFINITIONS "${bmc_test_cflags}; BOOST_TEST_DYN_LINK")
  target_link_libraries(bmc_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
else(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  message(STATUS "Boost unit test framework library not available")
endif(Boost_UNIT_TEST_FRAMEWORK_FOUND)
enable_testing()
set(BMC_TEST_NAMES
    time_signature_grammar_test_1
    key_signature_grammar_test_1 key_signature_grammar_test_2
    key_signature_grammar_test_3
    brl_parser_test
    measure_test1 measure_test2
    measure_interpretations_test1 measure_interpretations_test2
    notegroup_test1
    compiler_test1
    score_solo_test1 score_solo_test2
    score_tuplet_test1 score_tuplet_test2 score_tuplet_test3 score_tuplet_test4
    score_tuplet_test5 score_tuplet_test6 score_tuplet_test7 score_tuplet_test8
    slur_test1
    score_multiple_time_sigs
    common_factor
    bwv988_v01
    bwv988_v02
    bwv988_v03
    bwv988_v04
    bwv988_v05
    bwv988_v06
    bwv988_v07
    bwv988_v08
    bwv988_v09
    bwv988_v10
    bwv988_v11
    bwv988_v12
    bwv988_v13
    bwv988_v13_de
    bwv988_v14
    bwv988_v15
    bwv988_v16
    bwv988_v17
    bwv988_v18
    bwv988_v19
    bwv988_v22
    bwv988_v30
   )
foreach(test_name ${BMC_TEST_NAMES})
  add_test(NAME bmc_${test_name}
           COMMAND bmc_test --run_test=${test_name})
endforeach(test_name)
# A kludge to workaround cmake's inability to add dependencies to the test target
add_custom_target(check
                  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                  DEPENDS bmc_test bmc
                 )

install(TARGETS bmc DESTINATION bin)
install(TARGETS bmc_parse DESTINATION lib)
install(DIRECTORY ${bmc_SOURCE_DIR}/include/bmc DESTINATION include/bmc)

find_package(PkgConfig)
  pkg_search_module(CGICC cgicc)
  if(CGICC_FOUND)
    include_directories(${CGICC_INCLUDE_DIRS})
    add_executable(bmc.cgi cgi.cpp)
    target_link_libraries(bmc.cgi bmc_parse ${CGICC_LIBRARIES})
  endif(CGICC_FOUND)

find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
  execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" "import music21;"
                  RESULT_VARIABLE music21_NOTFOUND ERROR_QUIET)
  if(NOT music21_NOTFOUND)
    message(STATUS "Found music21")
    add_test(NAME music21 COMMAND ${CMAKE_SOURCE_DIR}/m21.sh ${CMAKE_SOURCE_DIR}/m21.py ${CMAKE_BINARY_DIR}/bmc2ly)
  else()
    message(STATUS "music21 not found")
  endif()
endif(PYTHONINTERP_FOUND)


if(bmc_USE_PYTHON)
  add_subdirectory(sphinx)
endif(bmc_USE_PYTHON)

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${bmc_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${bmc_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${bmc_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${bmc_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT       "Mario Lang <mlang@delysid.org>")
set(CPACK_STRIP_FILES           TRUE)
#set(CPACK_SOURCE_GENERATOR      "TGZ;TXZ;")
include(CPack)
