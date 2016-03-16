add_executable(bmc_test test.cpp)
target_compile_features(bmc_test PRIVATE cxx_range_for cxx_auto_type)
target_link_libraries(bmc_test braillemusic-static)
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
  set_tests_properties(bmc_${test_name} PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endforeach(test_name)
# A kludge to workaround cmake's inability to add dependencies to the test target
add_custom_target(check
                  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                  DEPENDS bmc_test bmc
)

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
