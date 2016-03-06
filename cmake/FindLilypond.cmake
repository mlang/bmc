# CMake find_package() Module for Lilypond music engraver
# http://lilypond.org/
#
# Example usage:
#
# find_package(Lilypond)
#
# If successful the following variables will be defined
# LILYPOND_FOUND
# LILYPOND_EXECUTABLE

set(CMAKE_FIND_APPBUNDLE LAST)
find_program(LILYPOND_EXECUTABLE
  NAMES lilypond
  PATH_SUFFIXES lilypond.app/Contents/Resources/bin
  DOC "Path to lilypond executable"
)

# Handle REQUIRED and QUIET arguments
# this will also set LILYPOND_FOUND to true if LILYPOND_EXECUTABLE exists
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Lilypond
  "Failed to locate lilypond executable" LILYPOND_EXECUTABLE
)
