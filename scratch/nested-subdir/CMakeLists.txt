
add_library(
  scratch-nested-subdir-lib
  lib/scratch-nested-subdir-library-source.cc
)

build_exec(
  EXECNAME scratch-nested-subdir-executable
  SOURCE_FILES scratch-nested-subdir-executable.cc
  LIBRARIES_TO_LINK scratch-nested-subdir-lib
                    ${libcore}
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/nested-subdir
)
