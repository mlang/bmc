add_definitions("-DHAVE_ICONV_H=1")
add_definitions("-DHAVE_WCHAR_H=1")
add_definitions("-DPACKAGE_NAME=\"brltty\"")
add_definitions("-DTEXT_TABLE_EXTENSION=\"ttb\"")

find_program(ICU_CONFIG icu-config)

if(NOT ICU_CONFIG)
  set(ICU_FOUND FALSE)
else(NOT WHICH_ICU_CONFIG_RESULT EQUAL 0)
  execute_process(COMMAND ${ICU_CONFIG} --exists
                  RESULT_VARIABLE ICU_EXISTS_RESULT)
  if(NOT ${ICU_EXISTS_RESULT} EQUAL 0)
    set(ICU_FOUND FALSE)
  else(NOT ${ICU_EXISTS_RESULT} EQUAL 0)
    execute_process(COMMAND ${ICU_CONFIG} --cppflags-searchpath
                    COMMAND sed s/^-I//g
                    OUTPUT_VARIABLE ICU_INCLUDE_DIRS)

    execute_process(COMMAND ${ICU_CONFIG} --ldflags-libsonly
                    COMMAND tr -d \n
                    COMMAND sed "s/[\t ]*-l/ /g"
                    COMMAND sed "s/^[\t ]*//"
                    COMMAND sed "s/[\t ]*$//"
                    OUTPUT_VARIABLE ICU_LIBRARY_NAMES)

    separate_arguments(ICU_LIBRARY_NAMES)

    foreach(CURRENT_LIB_NAME ${ICU_LIBRARY_NAMES})
      set(CURRENT_LIBRARY ${CURRENT_LIB_NAME}-NOTFOUND)
      find_library(CURRENT_LIBRARY ${CURRENT_LIB_NAME})
      list(APPEND ICU_LIBRARIES ${CURRENT_LIBRARY})
    endforeach(CURRENT_LIB_NAME ${ICU_LIBRARY_NAMES})

    set(ICU_FOUND TRUE)
  endif(NOT ${ICU_EXISTS_RESULT} EQUAL 0)
endif(NOT ICU_CONFIG)

if(ICU_FOUND)
  if(NOT ICU_FIND_QUIETLY)
    MESSAGE(STATUS "Found ICU")
  endif(NOT ICU_FIND_QUIETLY)
else(ICU_FOUND)
  if(ICU_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find ICU")
  endif(ICU_FIND_REQUIRED)
endif(ICU_FOUND)

if(ICU_FOUND)
  add_definitions("-DHAVE_ICU=1")
endif(ICU_FOUND)

add_library(ttb SHARED
            charset.c dataarea.c datafile.c file.c log.c
            ttb_compile.c ttb_native.c ttb_translate.c
            unicode.c
           )
target_link_libraries(ttb ${ICU_LIBRARIES})
install(TARGETS ttb DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
