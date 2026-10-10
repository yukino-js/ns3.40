
if(${NS3_COVERAGE})
  mark_as_advanced(GCOV)
  find_program(GCOV gcov)
  if(NOT ("${GCOV}" STREQUAL "GCOV-NOTFOUND"))
    add_definitions(--coverage)
    link_libraries(-lgcov)
  endif()

  mark_as_advanced(LCOV)
  find_program(LCOV lcov)
  if("${LCOV}" STREQUAL "LCOV-NOTFOUND")
    message(FATAL_ERROR "LCOV is required but it is not installed.")
  endif()

  if(${NS3_COVERAGE_ZERO_COUNTERS})
    set(zero_counters "--lcov-zerocounters")
  endif()

  file(MAKE_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/coverage)

  add_custom_target(
    coverage_gcc
    COMMAND lcov -o ns3.info -c --directory ${CMAKE_BINARY_DIR} ${zero_counters}
    COMMAND genhtml ns3.info
    WORKING_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/coverage
    DEPENDS run_test_py
  )
endif()
