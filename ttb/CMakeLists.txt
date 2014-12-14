set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(BMC_TTB_COMMON_FILES
    charset.c
    dataarea.c
    datafile.c
    file.c
    log.c
    queue.c
    ttb_compile.c
    ttb_native.c
    unicode.c
   )
find_path(ICONV_INCLUDE_DIR iconv.h)
if(ICONV_INCLUDE_DIR)
  add_definitions(-DHAVE_ICONV_H=1)
endif()
add_definitions(-DHAVE_WCHAR_H)
add_definitions(-DPACKAGE_NAME="brltty")
add_definitions(-DTEXT_TABLE_EXTENSION=".ttb")
add_definitions(-DTEXT_SUBTABLE_EXTENSION=".tti")

find_package(ICU 4.4 REQUIRED)
if(ICU_FOUND)
  add_definitions("-DHAVE_ICU=1")
endif(ICU_FOUND)

add_executable(tbl2hex tbl2hex.c ${BMC_TTB_COMMON_FILES})
target_link_libraries(tbl2hex ${ICU_LIBRARIES})
add_custom_command(OUTPUT text.auto.h
                   COMMAND tbl2hex ${CMAKE_CURRENT_SOURCE_DIR}/Tables/de.ttb > text.auto.h
                  )
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include/)
add_library(bmc_ttb SHARED ttb_translate.c text.auto.h ${BMC_TTB_COMMON_FILES})
set_target_properties(bmc_ttb
                      PROPERTIES
                      VERSION 0.0.1
                      SOVERSION 0.0
                     )
target_link_libraries(bmc_ttb ${ICU_LIBRARIES})
install(TARGETS bmc_ttb DESTINATION lib)
install(DIRECTORY Tables DESTINATION share/${CMAKE_PROJECT_NAME})
