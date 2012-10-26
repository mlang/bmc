cmake_minimum_required(VERSION 2.8)
project(bmc)
set(bmc_VERSION_MAJOR 0)
set(bmc_VERSION_MINOR 0)
set(bmc_VERSION_PATCH 1)
set(bmc_VERSION ${bmc_VERSION_MAJOR}.${bmc_VERSION_MINOR}.${bmc_VERSION_PATCH})
if (CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                  OUTPUT_VARIABLE GCC_VERSION)
  if (GCC_VERSION VERSION_LESS 4.5)
    message(FATAL_ERROR "GCC is too old (<= 4.5)")
  endif()
endif()
add_subdirectory(doxygen)
add_subdirectory(ttb)
add_subdirectory(ui)
find_package(Boost 1.48.0 REQUIRED)
include_directories(${bmc_SOURCE_DIR})
include_directories(${bmc_SOURCE_DIR}/include/)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-std=c++0x)
add_library(bmc_parse SHARED
            brlsym.cpp music.cpp
            numbers.cpp key_signature.cpp time_signature.cpp
            measure.cpp score.cpp
            value_disambiguator.cpp
            lilypond.cpp
           )
set_target_properties(bmc_parse
                      PROPERTIES
                      VERSION ${bmc_VERSION}
                      SOVERSION ${bmc_VERSION_MAJOR}.${bmc_VERSION_MINOR}
                     )
target_link_libraries(bmc_parse bmc_ttb)
add_definitions(-DSOUNDFONT_PATH="/usr/share/sounds/sf2/FluidR3_GM.sf2")
add_executable(bmc main.cpp fluidsynth.cpp)
find_library(FLUIDSYNTH_LIBRARY fluidsynth)
target_link_libraries(bmc bmc_parse ${FLUIDSYNTH_LIBRARY})
add_definitions(-DHAVE_ICONV_H=1)
add_definitions(-DHAVE_WCHAR_H=1)
add_definitions(-DTABLES_DIRECTORY="${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/Tables")

add_executable(bmc_test test.cpp)
set_target_properties(bmc_test
                      PROPERTIES
                      COMPILE_DEFINITIONS CMAKE_SOURCE_DIR="${PROJECT_SOURCE_DIR}/"
                     )
target_link_libraries(bmc_test bmc_parse)
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
    common_factor
    bwv988_v01
    bwv988_v02
    bwv988_v03
    bwv988_v04
    bwv988_v13
    bwv988_v13_de
    bwv988_v19
    bwv988_v30
    bwv988_v01_ly
    bwv988_v04_ly
    bwv988_v13_ly
    bwv988_v19_ly
    bwv988_v30_ly
    bwv988_v01_ly_with_locations
   )
foreach(test_name ${BMC_TEST_NAMES})
  add_test(NAME bmc_${test_name}
           COMMAND bmc_test --run_test=${test_name})
endforeach(test_name)
# A kludge to workaround cmake's inability to add dependencies to the test target
add_custom_target(check
                  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                  DEPENDS bmc_test
                 )

install(TARGETS bmc DESTINATION bin)
install(TARGETS bmc_parse DESTINATION lib)
install(DIRECTORY ${bmc_SOURCE_DIR}/include/ DESTINATION include)
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
