add_library(braillemusic SHARED
            text2braille.cpp
            brlsym.cpp music.cpp ast.cpp
            numbers.cpp key_signature.cpp time_signature.cpp
            partial_voice_sign.cpp simile.cpp tuplet_start.cpp
            measure.cpp score.cpp
            value_disambiguation.cpp value_disambiguator.cpp
            lilypond.cpp musicxml.cpp linebreaking.cpp reformat.cpp
)
set_target_properties(braillemusic
  PROPERTIES
  VERSION ${bmc_VERSION}
  SOVERSION ${bmc_VERSION_MAJOR}.${bmc_VERSION_MINOR}
)
target_link_libraries(braillemusic xsdcxx-musicxml Threads::Threads)
target_compile_features(braillemusic PRIVATE cxx_range_for cxx_final)
add_library(braillemusic-static STATIC
  text2braille.cpp
  brlsym.cpp music.cpp ast.cpp
  numbers.cpp key_signature.cpp time_signature.cpp
  partial_voice_sign.cpp simile.cpp tuplet_start.cpp
  measure.cpp score.cpp
  value_disambiguation.cpp value_disambiguator.cpp
  lilypond.cpp musicxml.cpp linebreaking.cpp reformat.cpp
)
target_include_directories(braillemusic-static PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(braillemusic PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(braillemusic-static
  xsdcxx-musicxml-static Threads::Threads
)
target_compile_features(braillemusic-static PRIVATE cxx_final)

install(TARGETS braillemusic braillemusic-static
  ARCHIVE DESTINATION lib COMPONENT Development
  LIBRARY DESTINATION lib COMPONENT Development
)
install(DIRECTORY ${bmc_SOURCE_DIR}/include/bmc DESTINATION include)


