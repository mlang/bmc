# CMake find_package() Module for Sphinx documentation generator
# http://sphinx-doc.org/
#
# Example usage:
#
# find_package(SphinxBuild)
#
# If successful the following variables will be defined
# SPHINX_BUILD_FOUND
# SPHINX_BUILD_EXECUTABLE

find_program(SPHINX_BUILD_EXECUTABLE
  NAMES sphinx-build sphinx-build2
  DOC "Path to sphinx-build executable"
)

# Handle REQUIRED and QUIET arguments
# this will also set SPHINX_BUILD_FOUND to true if SPHINX_BUILD_EXECUTABLE exists
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SphinxBuild
  "Failed to locate sphinx-build executable" SPHINX_BUILD_EXECUTABLE
)
