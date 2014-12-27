add_custom_target(doc ALL)
add_custom_command(
  TARGET doc
  COMMAND sphinx-build -b man ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/man
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bmc.rst
          ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.rst
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/bmc.1 DESTINATION share/man/man1)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/braille_music.7 DESTINATION share/man/man7)


