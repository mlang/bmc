configure_file(conf.py.in conf.py @ONLY)
add_custom_command(
  OUTPUT man/bmc.1 man/braille_music.7
  COMMAND sphinx-build -b man -c ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/man
  DEPENDS bmc.rst tutorial.rst ${CMAKE_SOURCE_DIR}/python/bmc/sphinx.py
)
add_custom_target(man ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/man/bmc.1
          ${CMAKE_CURRENT_BINARY_DIR}/man/braille_music.7
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/bmc.1 DESTINATION share/man/man1)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/braille_music.7 DESTINATION share/man/man7)

add_custom_command(
  OUTPUT html/index.html
  COMMAND sphinx-build -b html -c ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/html
  DEPENDS index.rst goldberg.rst tutorial.rst ${CMAKE_SOURCE_DIR}/python/bmc/sphinx.py
)
add_custom_target(html ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
)

