

function(build_lib)
  set(options IGNORE_PCH)
  set(oneValueArgs LIBNAME)
  set(multiValueArgs
      SOURCE_FILES
      HEADER_FILES
      LIBRARIES_TO_LINK
      TEST_SOURCES
      DEPRECATED_HEADER_FILES
      MODULE_ENABLED_FEATURES
      PRIVATE_HEADER_FILES
  )
  cmake_parse_arguments(
    "BLIB" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )

  string(REPLACE "${PROJECT_SOURCE_DIR}/" "" FOLDER
                 "${CMAKE_CURRENT_SOURCE_DIR}"
  )

  if("${FOLDER}" MATCHES "src")
    set(ns3-libs "${lib${BLIB_LIBNAME}};${ns3-libs}"
        CACHE INTERNAL "list of processed upstream modules"
    )
  else()
    set(ns3-contrib-libs "${lib${BLIB_LIBNAME}};${ns3-contrib-libs}"
        CACHE INTERNAL "list of processed contrib modules"
    )
  endif()

  if(NOT ${XCODE})
    add_library(
      ${lib${BLIB_LIBNAME}-obj} OBJECT "${BLIB_SOURCE_FILES}"
                                       "${BLIB_HEADER_FILES}"
    )

    if(${PRECOMPILE_HEADERS_ENABLED} AND (NOT ${BLIB_IGNORE_PCH}))
      target_precompile_headers(${lib${BLIB_LIBNAME}-obj} REUSE_FROM stdlib_pch)
    endif()

    add_library(
      ${lib${BLIB_LIBNAME}} SHARED $<TARGET_OBJECTS:${lib${BLIB_LIBNAME}-obj}>
    )
  else()
    add_library(${lib${BLIB_LIBNAME}} SHARED "${BLIB_SOURCE_FILES}")

    if(${PRECOMPILE_HEADERS_ENABLED} AND (NOT ${BLIB_IGNORE_PCH}))
      target_precompile_headers(${lib${BLIB_LIBNAME}} REUSE_FROM stdlib_pch)
    endif()
  endif()

  add_library(ns3::${lib${BLIB_LIBNAME}} ALIAS ${lib${BLIB_LIBNAME}})

  if("${BLIB_LIBNAME}" STREQUAL "core")
    set(config_headers ${CMAKE_HEADER_OUTPUT_DIRECTORY}/config-store-config.h
                       ${CMAKE_HEADER_OUTPUT_DIRECTORY}/core-config.h
    )
    if(${ENABLE_BUILD_VERSION})
      list(APPEND config_headers
           ${CMAKE_HEADER_OUTPUT_DIRECTORY}/version-defines.h
      )
    endif()

    if((NOT FILESYSTEM_LIBRARY_IS_LINKED) OR (${GCC} AND ${GCC8}))
      list(APPEND BLIB_LIBRARIES_TO_LINK -lstdc++fs)
    endif()

    if(${ENABLE_EXAMPLES} AND ${ENABLE_TESTS})
      if(NOT ${XCODE})
        target_compile_definitions(
          ${lib${BLIB_LIBNAME}}-obj PRIVATE NS3_ENABLE_EXAMPLES
        )
      else()
        target_compile_definitions(
          ${lib${BLIB_LIBNAME}} PRIVATE NS3_ENABLE_EXAMPLES
        )
      endif()
    endif()
  endif()
  set_target_properties(
    ${lib${BLIB_LIBNAME}}
    PROPERTIES
      PUBLIC_HEADER
      "${BLIB_HEADER_FILES};${BLIB_DEPRECATED_HEADER_FILES};${config_headers};${CMAKE_HEADER_OUTPUT_DIRECTORY}/${BLIB_LIBNAME}-module.h"
      PRIVATE_HEADER "${BLIB_PRIVATE_HEADER_FILES}"
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  )

  if(${NS3_CLANG_TIMETRACE})
    add_dependencies(timeTraceReport ${lib${BLIB_LIBNAME}})
  endif()

  set(non_ns_libraries_to_link ${CMAKE_THREAD_LIBS_INIT})

  set(ns_libraries_to_link)

  foreach(library ${BLIB_LIBRARIES_TO_LINK})
    remove_lib_prefix("${library}" module_name)

    if("${library}" STREQUAL "${BLIB_LIBNAME}")
      list(APPEND non_ns_libraries_to_link ${library})
      continue()
    endif()

    if(${module_name} IN_LIST ns3-all-enabled-modules)
      list(APPEND ns_libraries_to_link ${library})
    else()
      list(APPEND non_ns_libraries_to_link ${library})
    endif()
    unset(module_name)
  endforeach()

  if(NOT ${NS3_REEXPORT_THIRD_PARTY_LIBRARIES})
    set(exported_libraries PUBLIC ${LIB_AS_NEEDED_PRE} ${ns_libraries_to_link}
                           ${LIB_AS_NEEDED_POST}
    )

    set(private_libraries PRIVATE ${LIB_AS_NEEDED_PRE}
                          ${non_ns_libraries_to_link} ${LIB_AS_NEEDED_POST}
    )

    set(exported_include_directories)
  else()
    set(exported_libraries PUBLIC ${LIB_AS_NEEDED_PRE} ${ns_libraries_to_link}
                           ${non_ns_libraries_to_link} ${LIB_AS_NEEDED_POST}
    )
    set(private_libraries)

    get_target_includes(${lib${BLIB_LIBNAME}} exported_include_directories)

    string(REPLACE "-I" "" exported_include_directories
                   "${exported_include_directories}"
    )

    set(new_exported_include_directories)
    foreach(directory ${exported_include_directories})
      string(FIND "${directory}" "${PROJECT_SOURCE_DIR}" is_prefixed_in_subdir)
      if(${is_prefixed_in_subdir} GREATER_EQUAL 0)
        string(SUBSTRING "${directory}" ${is_prefixed_in_subdir} -1
                         directory_path
        )
        list(APPEND new_exported_include_directories
             $<BUILD_INTERFACE:${directory_path}>
        )
      else()
        list(APPEND new_exported_include_directories ${directory})
      endif()
    endforeach()
    set(exported_include_directories ${new_exported_include_directories})

    string(REPLACE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/include" ""
                   exported_include_directories
                   "${exported_include_directories}"
    )
  endif()

  target_link_libraries(
    ${lib${BLIB_LIBNAME}} ${exported_libraries} ${private_libraries}
  )

  if(NOT ${XCODE})
    set(temp)
    foreach(target ${ns_libraries_to_link})
      list(APPEND temp
           "$<TARGET_PROPERTY:${target},INTERFACE_INCLUDE_DIRECTORIES>"
      )
    endforeach()
    target_include_directories(${lib${BLIB_LIBNAME}}-obj PRIVATE ${temp})
    unset(temp)
  endif()

  set_target_properties(
    ${lib${BLIB_LIBNAME}}
    PROPERTIES OUTPUT_NAME ns${NS3_VER}-${BLIB_LIBNAME}${build_profile_suffix}
  )

  target_include_directories(
    ${lib${BLIB_LIBNAME}}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_OUTPUT_DIRECTORY}/include>
           $<INSTALL_INTERFACE:include>
    INTERFACE ${exported_include_directories}
  )

  get_target_property(
    target_definitions ${lib${BLIB_LIBNAME}} COMPILE_DEFINITIONS
  )
  if(${target_definitions} STREQUAL "target_definitions-NOTFOUND")
    set(target_definitions)
  endif()
  get_directory_property(dir_definitions COMPILE_DEFINITIONS)
  set(exported_definitions "${target_definitions};${dir_definitions}")
  list(REMOVE_DUPLICATES exported_definitions)
  list(REMOVE_ITEM exported_definitions "")
  set_target_properties(
    ${lib${BLIB_LIBNAME}} PROPERTIES INTERFACE_COMPILE_DEFINITIONS
                                     "${exported_definitions}"
  )

  set(ns3-external-libs "${non_ns_libraries_to_link};${ns3-external-libs}"
      CACHE INTERNAL
            "list of non-ns libraries to link to NS3_STATIC and NS3_MONOLIB"
  )
  if(${NS3_STATIC} OR ${NS3_MONOLIB})
    set(lib-ns3-static-objs
        "$<TARGET_OBJECTS:${lib${BLIB_LIBNAME}-obj}>;${lib-ns3-static-objs}"
        CACHE
          INTERNAL
          "list of object files from module used by NS3_STATIC and NS3_MONOLIB"
    )
  endif()

  write_module_header("${BLIB_LIBNAME}" "${BLIB_HEADER_FILES}")

  get_target_property(headers_to_check ${lib${BLIB_LIBNAME}} PUBLIC_HEADER)
  set(missing_headers)
  foreach(header ${headers_to_check})
    if(NOT ((EXISTS ${header}) OR (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${header})
           )
    )
      list(APPEND missing_headers ${header})
    endif()
  endforeach()
  if(missing_headers)
    message(
      FATAL_ERROR "Missing header files for ${BLIB_LIBNAME}: ${missing_headers}"
    )
  endif()

  copy_headers_before_building_lib(
    ${BLIB_LIBNAME} ${CMAKE_HEADER_OUTPUT_DIRECTORY} "${BLIB_HEADER_FILES}"
    public
  )
  if(BLIB_PRIVATE_HEADER_FILES)
    copy_headers_before_building_lib(
      ${BLIB_LIBNAME} ${CMAKE_HEADER_OUTPUT_DIRECTORY}
      "${BLIB_PRIVATE_HEADER_FILES}" private
    )
  endif()

  if(BLIB_DEPRECATED_HEADER_FILES)
    copy_headers_before_building_lib(
      ${BLIB_LIBNAME} ${CMAKE_HEADER_OUTPUT_DIRECTORY}
      "${BLIB_DEPRECATED_HEADER_FILES}" deprecated
    )
  endif()

  set(examples_before ${ns3-execs-clean})
  foreach(example_folder example;examples)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${example_folder})
      if(${ENABLE_EXAMPLES})
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${example_folder}/CMakeLists.txt)
          add_subdirectory(${example_folder})
        endif()
      endif()
      scan_python_examples(${CMAKE_CURRENT_SOURCE_DIR}/${example_folder})
    endif()
  endforeach()
  set(module_examples ${ns3-execs-clean})

  foreach(example ${examples_before})
    list(REMOVE_ITEM module_examples ${example})
  endforeach()
  unset(examples_before)

  set(filtered_in ON)
  if(NS3_FILTER_MODULE_EXAMPLES_AND_TESTS)
    set(filtered_in OFF)
    if(${BLIB_LIBNAME} IN_LIST NS3_FILTER_MODULE_EXAMPLES_AND_TESTS)
      set(filtered_in ON)
    endif()
  endif()

  if(${ENABLE_TESTS} AND ${filtered_in})
    list(LENGTH BLIB_TEST_SOURCES test_source_len)
    if(${test_source_len} GREATER 0)
      set(test${BLIB_LIBNAME} lib${BLIB_LIBNAME}-test CACHE INTERNAL "")

      if(WIN32)
        set(ns3-libs-tests
            "$<TARGET_OBJECTS:${test${BLIB_LIBNAME}}>;${ns3-libs-tests}"
            CACHE INTERNAL "list of test libraries"
        )
        add_library(${test${BLIB_LIBNAME}} OBJECT "${BLIB_TEST_SOURCES}")
      else()
        set(ns3-libs-tests "${test${BLIB_LIBNAME}};${ns3-libs-tests}"
            CACHE INTERNAL "list of test libraries"
        )
        add_library(${test${BLIB_LIBNAME}} SHARED "${BLIB_TEST_SOURCES}")

        if(${NS3_MONOLIB})
          target_link_libraries(
            ${test${BLIB_LIBNAME}} ${LIB_AS_NEEDED_PRE} ${lib-ns3-monolib}
            ${LIB_AS_NEEDED_POST}
          )
        else()
          target_link_libraries(
            ${test${BLIB_LIBNAME}} ${LIB_AS_NEEDED_PRE} ${lib${BLIB_LIBNAME}}
            "${BLIB_LIBRARIES_TO_LINK}" ${LIB_AS_NEEDED_POST}
          )
        endif()
        set_target_properties(
          ${test${BLIB_LIBNAME}}
          PROPERTIES OUTPUT_NAME
                     ns${NS3_VER}-${BLIB_LIBNAME}-test${build_profile_suffix}
        )
      endif()
      target_compile_definitions(
        ${test${BLIB_LIBNAME}} PRIVATE NS_TEST_SOURCEDIR="${FOLDER}/test"
      )
      if(${PRECOMPILE_HEADERS_ENABLED} AND (NOT ${BLIB_IGNORE_PCH}))
        target_precompile_headers(${test${BLIB_LIBNAME}} REUSE_FROM stdlib_pch)
      endif()

      if(${ENABLE_EXAMPLES})
        foreach(source_file ${BLIB_TEST_SOURCES})
          file(READ ${source_file} source_file_contents)
          foreach(example_as_test ${module_examples})
            string(FIND "${source_file_contents}" "${example_as_test}"
                        is_sub_string
            )
            if(NOT (${is_sub_string} EQUAL -1))
              add_dependencies(test-runner-examples-as-tests ${example_as_test})
            endif()
          endforeach()
        endforeach()
      endif()
    endif()
  endif()

  install(
    TARGETS ${lib${BLIB_LIBNAME}}
    EXPORT ns3ExportTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ns3"
    PRIVATE_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ns3"
  )
  if(${NS3_VERBOSE})
    message(STATUS "Processed ${FOLDER}")
  endif()
endfunction()

function(build_lib_example)
  set(options IGNORE_PCH)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCE_FILES HEADER_FILES LIBRARIES_TO_LINK)
  cmake_parse_arguments("BLIB_EXAMPLE" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  string(REPLACE "${PROJECT_SOURCE_DIR}/" "" FOLDER "${CMAKE_CURRENT_SOURCE_DIR}")

  check_for_missing_libraries(
    missing_dependencies "${BLIB_EXAMPLE_LIBRARIES_TO_LINK}"
  )

  set(filtered_in ON)
  if(NS3_FILTER_MODULE_EXAMPLES_AND_TESTS)
    set(filtered_in OFF)
    if(${BLIB_LIBNAME} IN_LIST NS3_FILTER_MODULE_EXAMPLES_AND_TESTS)
      set(filtered_in ON)
    endif()
  endif()

  if((NOT missing_dependencies) AND ${filtered_in})
    if(${BLIB_EXAMPLE_IGNORE_PCH})
      set(IGNORE_PCH IGNORE_PCH)
    endif()
    build_exec(
      EXECNAME ${BLIB_EXAMPLE_NAME}
      SOURCE_FILES ${BLIB_EXAMPLE_SOURCE_FILES}
      HEADER_FILES ${BLIB_EXAMPLE_HEADER_FILES}
      LIBRARIES_TO_LINK
        ${lib${BLIB_LIBNAME}} ${BLIB_EXAMPLE_LIBRARIES_TO_LINK}
        ${optional_visualizer_lib}
      EXECUTABLE_DIRECTORY_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${FOLDER}/
      ${IGNORE_PCH}
    )
  endif()
endfunction()

function(write_module_header name header_files)
  string(TOUPPER "${name}" uppercase_name)
  string(REPLACE "-" "_" final_name "${uppercase_name}")
  list(APPEND contents "#ifdef NS3_MODULE_COMPILATION ")
  list(
    APPEND
    contents
    "
    error \"Do not include ns3 module aggregator headers from other modules; these are meant only for end user scripts.\" "
  )
  list(APPEND contents "
#endif "
  )
  list(APPEND contents "
#ifndef NS3_MODULE_"
  )
  list(APPEND contents ${final_name})
  list(APPEND contents "
    // Module headers: "
  )

  foreach(header ${header_files})
    get_filename_component(head ${header} NAME)
    list(APPEND contents "
    #include <ns3/${head}>"
    )
  endforeach()

  list(APPEND contents "
#endif "
  )
  if(EXISTS ${CMAKE_HEADER_OUTPUT_DIRECTORY}/${name}-module.h)
    file(READ ${CMAKE_HEADER_OUTPUT_DIRECTORY}/${name}-module.h oldcontents)
    string(REPLACE ";" "" contents "${contents}")
    if("${contents}" STREQUAL "${oldcontents}")
      return()
    endif()
  endif()
  file(WRITE ${CMAKE_HEADER_OUTPUT_DIRECTORY}/${name}-module.h ${contents})
endfunction()
