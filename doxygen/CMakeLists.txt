find_package(Doxygen REQUIRED)
include(FindLATEX)
configure_file(Doxyfile.in Doxyfile)
add_custom_target(doxygen)
if(DOXYGEN_FOUND)
add_custom_command(TARGET doxygen
                   COMMAND ${DOXYGEN_EXECUTABLE}
                  )
add_custom_command(TARGET doxygen
                   COMMAND ${PDFLATEX_COMPILER} refman.tex
                   WORKING_DIRECTORY latex
                  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        DESTINATION share/doc/${CMAKE_PROJECT_NAME}
       )
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc/${CMAKE_PROJECT_NAME})
endif()
