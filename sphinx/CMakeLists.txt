configure_file(conf.py.in conf.py @ONLY)
add_custom_command(
  OUTPUT man/bmc.1 man/braille_music.7
  COMMAND sphinx-build -q -b man -c ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/man
  DEPENDS bmc.rst tutorial.rst ${CMAKE_SOURCE_DIR}/python/bmc/sphinx.py
)
add_custom_target(man ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/man/bmc.1
          ${CMAKE_CURRENT_BINARY_DIR}/man/braille_music.7
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/bmc.1 DESTINATION share/man/man1)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/braille_music.7 DESTINATION share/man/man7)
set_property(DIRECTORY APPEND
  PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/man"
)

add_custom_command(
  OUTPUT html/index.html html/tutorial.html
  COMMAND sphinx-build -q -b html -c ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/html
  DEPENDS index.rst goldberg.rst tutorial.rst ${CMAKE_SOURCE_DIR}/python/bmc/sphinx.py
)
add_custom_target(html ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
)
set_property(DIRECTORY APPEND
  PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/html"
)

add_custom_target(linkcheck
  COMMAND sphinx-build -q -b linkcheck -c ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/linkcheck
)
set_property(DIRECTORY APPEND
  PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/linkcheck"
)
