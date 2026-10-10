

find_program(
  SPHINX_EXECUTABLE NAMES sphinx-build sphinx-build2
  DOC "Path to sphinx-build executable"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Sphinx "Failed to locate sphinx-build executable" SPHINX_EXECUTABLE
)

option(SPHINX_OUTPUT_HTML "Output standalone HTML files" ON)
option(SPHINX_OUTPUT_MAN "Output man pages" ON)

option(SPHINX_WARNINGS_AS_ERRORS
       "When building documentation treat warnings as errors" ON
)
