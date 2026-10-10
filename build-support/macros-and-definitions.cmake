
add_definitions(-DPROJECT_SOURCE_PATH="${PROJECT_SOURCE_DIR}")

set(NS3_INT64X64 "INT128" CACHE STRING "Int64x64 implementation")
set_property(CACHE NS3_INT64X64 PROPERTY STRINGS INT128 CAIRO DOUBLE)


option(NS3_REEXPORT_THIRD_PARTY_LIBRARIES "Export all third-party libraries
and include directories to ns-3 module consumers" ON
)

option(NS3_ENABLE_SUDO
       "Set executables ownership to root and enable the SUID flag" OFF
)

option(NS3_PIP_PACKAGING "Control aspects related to pip wheel packaging" OFF)

include(${PROJECT_SOURCE_DIR}/build-support/3rd-party/colored-messages.cmake)

if(EXISTS "/proc/version")
  file(READ "/proc/version" CMAKE_LINUX_DISTRO)
  string(FIND "${CMAKE_LINUX_DISTRO}" "Microsoft" res)
  if(res EQUAL -1)
    set(WSLv1 False)
  else()
    set(WSLv1 True)
  endif()
endif()

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
  add_definitions(-D__LINUX__)
endif()

if(APPLE)
  add_definitions(-D__APPLE__)
  set(CMAKE_FIND_APPBUNDLE "LAST")
endif()

if(WIN32)
  set(NS3_PRECOMPILE_HEADERS OFF
      CACHE BOOL "Precompile module headers to speed up compilation" FORCE
  )

  add_definitions(/D_USE_MATH_DEFINES)
endif()

set(cat_command cat)

if(CMAKE_XCODE_BUILD_SYSTEM)
  set(XCODE True)
else()
  set(XCODE False)
endif()

include(ProcessorCount)
ProcessorCount(NumThreads)

if("${NS3_OUTPUT_DIRECTORY}" STREQUAL "")
  message(STATUS "Using default output directory ${PROJECT_SOURCE_DIR}/build")
  set(CMAKE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
else()
  set(absolute_ns3_output_directory "${NS3_OUTPUT_DIRECTORY}")
  if(NOT IS_ABSOLUTE ${NS3_OUTPUT_DIRECTORY})
    set(absolute_ns3_output_directory
        "${PROJECT_SOURCE_DIR}/${NS3_OUTPUT_DIRECTORY}"
    )
  endif()

  string(REPLACE "\\" "/" absolute_ns3_output_directory
                 "${absolute_ns3_output_directory}"
  )

  if(NOT (EXISTS ${absolute_ns3_output_directory}))
    message(
      STATUS
        "User-defined output directory \"${NS3_OUTPUT_DIRECTORY}\" doesn't exist. Trying to create it"
    )
    file(MAKE_DIRECTORY ${absolute_ns3_output_directory})
    if(NOT (EXISTS ${absolute_ns3_output_directory}))
      message(
        FATAL_ERROR
          "User-defined output directory \"${absolute_ns3_output_directory}\" could not be created. "
          "Try changing the value of NS3_OUTPUT_DIRECTORY"
      )
    endif()

    if(NOT ("${absolute_ns3_output_directory}" MATCHES "${PROJECT_SOURCE_DIR}"))
      message(
        WARNING
          "User-defined output directory \"${absolute_ns3_output_directory}\" is outside "
          " of the ns-3 directory ${PROJECT_SOURCE_DIR}, which will break some tests"
      )
    endif()
  endif()
  message(
    STATUS
      "User-defined output directory \"${absolute_ns3_output_directory}\" will be used"
  )
  set(CMAKE_OUTPUT_DIRECTORY ${absolute_ns3_output_directory})
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY})
set(CMAKE_HEADER_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY}/include/ns3)
set(THIRD_PARTY_DIRECTORY ${PROJECT_SOURCE_DIR}/3rd-party)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
link_directories(${CMAKE_OUTPUT_DIRECTORY}/lib)
file(MAKE_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY})

include(GNUInstallDirs)
include(build-support/custom-modules/ns3-cmake-package.cmake)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:$ORIGIN/:$ORIGIN/../lib")

if(${NS3_USE_LIB64})
  link_directories(${CMAKE_OUTPUT_DIRECTORY}/lib64)
  set(CMAKE_INSTALL_RPATH
      "${CMAKE_INSTALL_RPATH}:${CMAKE_INSTALL_PREFIX}/lib64:$ORIGIN/:$ORIGIN/../lib64"
  )
endif()

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(${XCODE})
  foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
        ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    )
  endforeach()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_LINK_DEPENDS_NO_SHARED TRUE)

set(below_minimum_msg "compiler is below the minimum required version")
set(CLANG FALSE)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${AppleClang_MinVersion})
    message(
      FATAL_ERROR
        "Apple Clang ${CMAKE_CXX_COMPILER_VERSION} ${below_minimum_msg} ${AppleClang_MinVersion}"
    )
  endif()
  set(CLANG TRUE)
endif()

if((NOT CLANG) AND ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${Clang_MinVersion})
    message(
      FATAL_ERROR
        "Clang ${CMAKE_CXX_COMPILER_VERSION} ${below_minimum_msg} ${Clang_MinVersion}"
    )
  endif()
  set(CLANG TRUE)
endif()

if(CLANG)
  if(${NS3_COLORED_OUTPUT} OR "$ENV{CLICOLOR}")
    add_definitions(-fcolor-diagnostics)
  endif()
endif()

set(GCC FALSE)
set(GCC8 FALSE)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${GNU_MinVersion})
    message(
      FATAL_ERROR
        "GNU ${CMAKE_CXX_COMPILER_VERSION} ${below_minimum_msg} ${GNU_MinVersion}"
    )
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0.0")
    set(GCC8 TRUE)
  endif()
  set(GCC TRUE)
  add_definitions(-fno-semantic-interposition)
  if(${NS3_COLORED_OUTPUT} OR "$ENV{CLICOLOR}")
    add_definitions(-fdiagnostics-color=always)
  endif()
endif()
unset(below_minimum_msg)

set(CXX_UNSUPPORTED_STANDARDS 98 11 14)
set(CMAKE_CXX_STANDARD_MINIMUM 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(LIB_AS_NEEDED_PRE)
set(LIB_AS_NEEDED_POST)
set(STATIC_LINK_FLAGS -static -static-libstdc++ -static-libgcc)
if(${GCC} AND NOT APPLE)
  set(LIB_AS_NEEDED_PRE -Wl,--no-as-needed)
  set(LIB_AS_NEEDED_POST -Wl,--as-needed)
  set(LIB_AS_NEEDED_PRE_STATIC -Wl,--whole-archive,-Bstatic)
  set(LIB_AS_NEEDED_POST_STATIC -Wl,--no-whole-archive)
  set(LIB_AS_NEEDED_POST_STATIC_DYN -Wl,-Bdynamic,--no-whole-archive)
endif()

if(${CLANG} AND APPLE)
  set(LIB_AS_NEEDED_POST)
  set(LIB_AS_NEEDED_PRE_STATIC -Wl,-all_load)
  set(STATIC_LINK_FLAGS)
endif()

if(${NS3_FAST_LINKERS})
  mark_as_advanced(MOLD LLD)
  find_program(MOLD mold)
  find_program(LLD ld.lld)


  if(NOT USING_FAST_LINKER
     AND NOT (${MOLD} STREQUAL "MOLD-NOTFOUND")
     AND LINUX
     AND ${GCC}
     AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.1.0)
  )
    set(USING_FAST_LINKER MOLD)
    add_link_options("-fuse-ld=mold")
  endif()

  if(NOT USING_FAST_LINKER AND NOT (${LLD} STREQUAL "LLD-NOTFOUND")
     AND (${GCC} OR ${CLANG})
  )
    set(USING_FAST_LINKER LLD)
    add_link_options("-fuse-ld=lld")
    if(WIN32)
      set(LIB_AS_NEEDED_PRE)
    endif()
  endif()
endif()

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(CheckFunctionExists)

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

macro(library_target_name libname targetname)
  set(${targetname} lib${libname})
endmacro()

macro(clear_global_cached_variables)
  unset(build_profile CACHE)
  unset(build_profile_suffix CACHE)
  unset(lib-ns3-static-objs CACHE)
  unset(ns3-contrib-libs CACHE)
  unset(ns3-example-folders CACHE)
  unset(ns3-execs CACHE)
  unset(ns3-execs-clean CACHE)
  unset(ns3-execs-py CACHE)
  unset(ns3-external-libs CACHE)
  unset(ns3-headers-to-module-map CACHE)
  unset(ns3-libs CACHE)
  unset(ns3-libs-tests CACHE)
  mark_as_advanced(
    build_profile
    build_profile_suffix
    lib-ns3-static-objs
    ns3-contrib-libs
    ns3-example-folders
    ns3-execs
    ns3-execs-clean
    ns3-execs-py
    ns3-external-libs
    ns3-headers-to-module-map
    ns3-libs
    ns3-libs-tests
  )
endmacro()

include(build-support/3rd-party/find-program-hints.cmake)

function(check_deps package_deps program_deps missing_deps)
  set(local_missing_deps)
  foreach(package ${package_deps})
    find_package(${package})
    if(NOT ${${package}_FOUND})
      list(APPEND local_missing_deps ${package})
    endif()
  endforeach()

  foreach(program ${program_deps})
    string(TOUPPER ${program} upper_${program})
    mark_as_advanced(${upper_${program}})
    find_program(
      ${upper_${program}} ${program} HINTS ${3RD_PARTY_FIND_PROGRAM_HINTS}
    )
    if("${${upper_${program}}}" STREQUAL "${upper_${program}}-NOTFOUND")
      list(APPEND local_missing_deps ${program})
    endif()
  endforeach()

  set(${missing_deps} ${local_missing_deps} PARENT_SCOPE)
endfunction()

macro(process_options)
  clear_global_cached_variables()

  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "default" CACHE STRING "Choose the type of build."
                                         FORCE
    )
    set(NS3_ASSERT ON CACHE BOOL "Enable assert on failure" FORCE)
    set(NS3_LOG ON CACHE BOOL "Enable logging to be built" FORCE)
    set(NS3_WARNINGS_AS_ERRORS OFF
        CACHE BOOL "Treat warnings as errors. Requires NS3_WARNINGS=ON" FORCE
    )
  endif()

  string(TOLOWER ${CMAKE_BUILD_TYPE} cmakeBuildType)
  set(build_profile "${cmakeBuildType}" CACHE INTERNAL "")
  if(${cmakeBuildType} STREQUAL "debug")
    add_definitions(-DNS3_BUILD_PROFILE_DEBUG)
  elseif(${cmakeBuildType} STREQUAL "relwithdebinfo" OR ${cmakeBuildType}
                                                        STREQUAL "default"
  )
    set(cmakeBuildType relwithdebinfo)
    set(CMAKE_CXX_FLAGS_DEFAULT ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    add_definitions(-DNS3_BUILD_PROFILE_DEBUG)
  elseif(${cmakeBuildType} STREQUAL "release")
    if(${NS3_NATIVE_OPTIMIZATIONS})
      add_definitions(-DNS3_BUILD_PROFILE_OPTIMIZED)
      set(build_profile "optimized" CACHE INTERNAL "")
    else()
      add_definitions(-DNS3_BUILD_PROFILE_RELEASE)
    endif()
  else()
    add_definitions(-DNS3_BUILD_PROFILE_RELEASE)
  endif()

  set(ENABLE_EXAMPLES OFF)
  if(${NS3_EXAMPLES} OR ${ns3rc_examples_enabled})
    set(ENABLE_EXAMPLES ON)
  endif()

  set(ENABLE_TESTS OFF)
  if(${NS3_TESTS} OR ${ns3rc_tests_enabled})
    set(ENABLE_TESTS ON)
    enable_testing()
  else()
    list(REMOVE_ITEM libs_to_build test)
  endif()

  set(profiles_without_suffixes release)
  set(build_profile_suffix "" CACHE INTERNAL "")
  if(NOT (${build_profile} IN_LIST profiles_without_suffixes))
    set(build_profile_suffix -${build_profile} CACHE INTERNAL "")
  endif()

  if(${NS3_VERBOSE})
    set_property(GLOBAL PROPERTY TARGET_MESSAGES TRUE)
    set(CMAKE_FIND_DEBUG_MODE TRUE)
    set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE INTERNAL "")
  else()
    set_property(GLOBAL PROPERTY TARGET_MESSAGES OFF)
    unset(CMAKE_FIND_DEBUG_MODE)
    unset(CMAKE_VERBOSE_MAKEFILE CACHE)
  endif()

  if(${NS3_WARNINGS})
    if(MSVC)
      add_compile_options(/W3)
      if(${NS3_WARNINGS_AS_ERRORS})
        add_compile_options(/WX)
      endif()
    else()
      add_compile_options(-Wall)
      if(${NS3_WARNINGS_AS_ERRORS})
        add_compile_options(-Werror -Wno-error=deprecated-declarations)
      endif()
    endif()
  endif()

  include(build-support/custom-modules/ns3-versioning.cmake)
  set(ENABLE_BUILD_VERSION False)
  configure_embedded_version()

  if(${NS3_CLANG_FORMAT})
    find_program(CLANG_FORMAT clang-format)
    if("${CLANG_FORMAT}" STREQUAL "CLANG_FORMAT-NOTFOUND")
      message(FATAL_ERROR "Clang-format was not found")
    else()
      file(
        GLOB_RECURSE
        ALL_CXX_SOURCE_FILES
        src/*.cc
        src/*.h
        examples/*.cc
        examples/*.h
        utils/*.cc
        utils/*.h
        scratch/*.cc
        scratch/*.h
      )
      add_custom_target(
        clang-format COMMAND ${CLANG_FORMAT} -style=file -i
                             ${ALL_CXX_SOURCE_FILES}
      )
      unset(ALL_CXX_SOURCE_FILES)
    endif()
  endif()

  if(${NS3_CLANG_TIDY})
    find_program(
      CLANG_TIDY NAMES clang-tidy clang-tidy-14 clang-tidy-15 clang-tidy-16
    )
    if("${CLANG_TIDY}" STREQUAL "CLANG_TIDY-NOTFOUND")
      message(FATAL_ERROR "Clang-tidy was not found")
    else()
      if((${CMAKE_VERSION} VERSION_LESS "3.12.0") AND ${NS3_CCACHE}
         AND (NOT ("${CCACHE}" STREQUAL "CCACHE-NOTFOUND"))
      )
        message(
          FATAL_ERROR
            "The current CMake ${CMAKE_VERSION} won't ccache objects correctly when running with clang-tidy."
            "Update CMake to at least version 3.12, or disable either ccache or clang-tidy to continue."
        )
      endif()
      set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY}")
    endif()
  else()
    unset(CMAKE_CXX_CLANG_TIDY)
  endif()

  if(${NS3_CLANG_TIMETRACE})
    if(${CLANG})
      include(ExternalProject)
      ExternalProject_Add(
        ClangBuildAnalyzer
        GIT_REPOSITORY "https://github.com/aras-p/ClangBuildAnalyzer.git"
        GIT_TAG "47406981a1c5a89e8f8c62802b924c3e163e7cb4"
        CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        INSTALL_COMMAND cmake -E copy_if_different ClangBuildAnalyzer
                        ${PROJECT_BINARY_DIR}
      )

      add_definitions(-ftime-trace)
      add_custom_target(
        timeTraceReport
        COMMAND
          ${PROJECT_BINARY_DIR}/ClangBuildAnalyzer --all ${PROJECT_BINARY_DIR}
          ${PROJECT_BINARY_DIR}/clangBuildAnalyzerReport.bin
        COMMAND
          ${PROJECT_BINARY_DIR}/ClangBuildAnalyzer --analyze
          ${PROJECT_BINARY_DIR}/clangBuildAnalyzerReport.bin >
          ${PROJECT_SOURCE_DIR}/ClangBuildAnalyzerReport.txt
        DEPENDS ClangBuildAnalyzer
      )
    else()
      message(
        FATAL_ERROR
          "TimeTrace is a Clang feature, but you're using a different compiler."
      )
    endif()
  endif()

  mark_as_advanced(CMAKE_FORMAT_PROGRAM)
  find_program(CMAKE_FORMAT_PROGRAM cmake-format HINTS ~/.local/bin)
  if("${CMAKE_FORMAT_PROGRAM}" STREQUAL "CMAKE_FORMAT_PROGRAM-NOTFOUND")
    message(${HIGHLIGHTED_STATUS} "Proceeding without cmake-format")
  else()
    file(GLOB_RECURSE MODULES_CMAKE_FILES src/**/CMakeLists.txt
         contrib/**/CMakeLists.txt examples/**/CMakeLists.txt
         scratch/**/CMakeLists.txt
    )
    file(
      GLOB
      INTERNAL_CMAKE_FILES
      CMakeLists.txt
      utils/**/CMakeLists.txt
      src/CMakeLists.txt
      build-support/**/*.cmake
      build-support/*.cmake
    )
    add_custom_target(
      cmake-format
      COMMAND
        ${CMAKE_FORMAT_PROGRAM} -c
        ${PROJECT_SOURCE_DIR}/build-support/cmake-format.yaml -i
        ${INTERNAL_CMAKE_FILES}
      COMMAND
        ${CMAKE_FORMAT_PROGRAM} -c
        ${PROJECT_SOURCE_DIR}/build-support/cmake-format-modules.yaml -i
        ${MODULES_CMAKE_FILES}
    )
    unset(MODULES_CMAKE_FILES)
    unset(INTERNAL_CMAKE_FILES)
  endif()

  if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD_MINIMUM})
  endif()

  list(FIND CXX_UNSUPPORTED_STANDARDS ${CMAKE_CXX_STANDARD} unsupported)
  if(${unsupported} GREATER -1)
    message(
      FATAL_ERROR
        "You're trying to use the unsupported C++ ${CMAKE_CXX_STANDARD}.\n"
        "Try -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD_MINIMUM}."
    )
  endif()

  cmake_policy(SET CMP0066 NEW)
  cmake_policy(SET CMP0067 NEW)

  include(build-support/custom-modules/ns3-compiler-workarounds.cmake)

  if(${NS3_DES_METRICS})
    add_definitions(-DENABLE_DES_METRICS)
  endif()

  if(${NS3_SANITIZE} AND ${NS3_SANITIZE_MEMORY})
    message(
      FATAL_ERROR
        "The memory sanitizer can't be used with other sanitizers. Disable one of them."
    )
  endif()

  if(${NS3_SANITIZE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak,undefined")
  endif()

  if(${NS3_SANITIZE_MEMORY})
    if(${CLANG})
      set(blacklistfile ${PROJECT_SOURCE_DIR}/memory-sanitizer-blacklist.txt)
      if(EXISTS ${blacklistfile})
        set(CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} -fsanitize=memory -fsanitize-blacklist=${blacklistfile}"
        )
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")
      endif()

      if(NOT ($ENV{MSAN_OPTIONS} MATCHES "strict_memcmp=0"))
        message(
          WARNING
            "Please export MSAN_OPTIONS=strict_memcmp=0 "
            "and call the generated buildsystem directly to proceed.\n"
            "Trying to build with cmake or IDEs will probably fail since"
            "CMake can't export environment variables to its parent process."
        )
      endif()
      unset(blacklistfile)
    else()
      message(FATAL_ERROR "The memory sanitizer is only supported by Clang")
    endif()
  endif()

  if(${NS3_NATIVE_OPTIMIZATIONS} AND ${GCC})
    add_compile_options(-march=native -mtune=native)
  endif()

  if(${NS3_LINK_TIME_OPTIMIZATION})
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_AVAILABLE OUTPUT output)
    if(LTO_AVAILABLE)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
      message(STATUS "Link-time optimization (LTO) is supported.")
    else()
      message(
        STATUS "Link-time optimization (LTO) is not supported: ${output}."
      )
    endif()
  endif()

  if(${NS3_LINK_WHAT_YOU_USE})
    set(CMAKE_LINK_WHAT_YOU_USE TRUE)
  else()
    set(CMAKE_LINK_WHAT_YOU_USE FALSE)
  endif()

  if(${NS3_INCLUDE_WHAT_YOU_USE})
    find_program(INCLUDE_WHAT_YOU_USE_PROG iwyu)
    if("${INCLUDE_WHAT_YOU_USE_PROG}" STREQUAL
       "INCLUDE_WHAT_YOU_USE_PROG-NOTFOUND"
    )
      message(FATAL_ERROR "iwyu (include-what-you-use) was not found.")
    endif()
    message(STATUS "iwyu is enabled")
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
        ${PROJECT_SOURCE_DIR}/build-support/iwyu-wrapper.sh;${PROJECT_SOURCE_DIR}
    )
  else()
    unset(CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
  endif()

  if(${XCODE})
    if(${NS3_STATIC} OR ${NS3_MONOLIB})
      message(
        FATAL_ERROR
          "Xcode doesn't play nicely with CMake object libraries,"
          "and those are used for NS3_STATIC and NS3_MONOLIB.\n"
          "Disable them or try a different generator"
      )
    endif()
    set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
  else()
    unset(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY)
    unset(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED)
    unset(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY)
  endif()

  include_directories(${CMAKE_OUTPUT_DIRECTORY}/include)

  list(APPEND CMAKE_MODULE_PATH
       "${PROJECT_SOURCE_DIR}/build-support/custom-modules"
  )
  list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/build-support/3rd-party")

  include(ns3-vcpkg-hunter)

  if(${NS3_CPM})
    set(CPM_DOWNLOAD_VERSION 0.38.2)
    set(CPM_DOWNLOAD_LOCATION
        "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake"
    )
    if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
      message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
      file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
      )
    endif()
    include(${CPM_DOWNLOAD_LOCATION})
    set(CPM_USE_LOCAL_PACKAGES ON)
  endif()
  if(TEST_PACKAGE_MANAGER)
    if(${TEST_PACKAGE_MANAGER} STREQUAL "CPM")
      cpmaddpackage(
        NAME ARMADILLO GIT_TAG 6cada351248c9a967b137b9fcb3d160dad7c709b
        GIT_REPOSITORY https://gitlab.com/conradsnicta/armadillo-code.git
      )
      find_package(ARMADILLO REQUIRED)
      message(STATUS "Armadillo was found? ${ARMADILLO_FOUND}")
    elseif(${TEST_PACKAGE_MANAGER} STREQUAL "VCPKG")
      add_package(Armadillo)
      find_package(Armadillo REQUIRED)
      message(STATUS "Armadillo was found? ${ARMADILLO_FOUND}")
    else()
      find_package(Armadillo REQUIRED)
    endif()
  endif()

  set(ENABLE_SQLITE False)
  if(${NS3_SQLITE})
    find_external_library(
      DEPENDENCY_NAME SQLite3 HEADER_NAME sqlite3.h LIBRARY_NAME sqlite3
    )

    if(${SQLite3_FOUND})
      set(ENABLE_SQLITE True)
      add_definitions(-DHAVE_SQLITE3)
      include_directories(${SQLite3_INCLUDE_DIRS})
    else()
      message(${HIGHLIGHTED_STATUS} "SQLite was not found")
    endif()
  endif()

  set(ENABLE_EIGEN False)
  if(${NS3_EIGEN})
    find_package(Eigen3 QUIET)

    if(${EIGEN3_FOUND})
      set(ENABLE_EIGEN True)
      add_definitions(-DHAVE_EIGEN3)
      add_definitions(-DEIGEN_MPL2_ONLY)
      include_directories(${EIGEN3_INCLUDE_DIR})
    else()
      message(${HIGHLIGHTED_STATUS} "Eigen was not found")
    endif()
  endif()

  if(${NS3_GTK3})
    find_package(HarfBuzz QUIET)
    if(NOT ${HarfBuzz_FOUND})
      message(${HIGHLIGHTED_STATUS}
              "Harfbuzz is required by GTK3 and was not found."
      )
    else()
      set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "")
      find_package(GTK3 QUIET)
      unset(CMAKE_SUPPRESS_DEVELOPER_WARNINGS CACHE)
      if(NOT ${GTK3_FOUND})
        message(${HIGHLIGHTED_STATUS}
                "GTK3 was not found. Continuing without it."
        )
      else()
        if(${GTK3_VERSION} VERSION_LESS 3.22)
          set(GTK3_FOUND FALSE)
          message(${HIGHLIGHTED_STATUS}
                  "GTK3 found with incompatible version ${GTK3_VERSION}"
          )
        else()
          message(STATUS "GTK3 was found.")
          include_directories(${GTK3_INCLUDE_DIRS} ${HarfBuzz_INCLUDE_DIRS})
        endif()
      endif()

    endif()
  endif()

  if(${NS3_STATIC})
    message(
      WARNING "Statically linking 3rd party libraries have not been tested.\n"
              "Disable Brite, Click, Gtk, GSL, Mpi, Openflow and SQLite"
              " if you want a standalone static ns-3 library."
    )
    if(WIN32)
      message(FATAL_ERROR "Static builds are unsupported on Windows"
                          "\nSocket libraries cannot be linked statically"
      )
    endif()
  else()
    find_package(LibXml2 QUIET)
    if(NOT ${LIBXML2_FOUND})
      message(${HIGHLIGHTED_STATUS}
              "LibXML2 was not found. Continuing without it."
      )
    else()
      message(STATUS "LibXML2 was found.")
      add_definitions(-DHAVE_LIBXML2)
      include_directories(${LIBXML2_INCLUDE_DIR})
    endif()
  endif()

  set(THREADS_PREFER_PTHREAD_FLAG)
  find_package(Threads QUIET)
  if(NOT ${Threads_FOUND})
    message(FATAL_ERROR Threads are required by ns-3)
  endif()

  set(Python3_LIBRARIES)
  set(Python3_EXECUTABLE)
  set(Python3_FOUND FALSE)
  set(Python3_INCLUDE_DIRS)
  if(${NS3_PYTHON_BINDINGS})
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
      find_package(Python3 COMPONENTS Interpreter Development)
    else()
      set(Python_ADDITIONAL_VERSIONS 3.6 3.7 3.8 3.9 3.10 3.11)
      find_package(PythonInterp)
      find_package(PythonLibs)

      set(Python3_Interpreter_FOUND ${PYTHONINTERP_FOUND})
      set(Python3_Development_FOUND ${PYTHONLIBS_FOUND})
      if(${PYTHONINTERP_FOUND})
        set(Python3_EXECUTABLE ${PYTHON_EXECUTABLE})
        set(Python3_FOUND TRUE)
      endif()
      if(${PYTHONLIBS_FOUND})
        set(Python3_LIBRARIES ${PYTHON_LIBRARIES})
        set(Python3_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS})
      endif()
    endif()
  else()
    check_deps("" "python3" python3_deps)
    if(python3_deps)
      message(FATAL_ERROR "Python3 was not found")
    else()
      set(Python3_EXECUTABLE ${PYTHON3})
    endif()
  endif()

  if(${Python3_Interpreter_FOUND})
    if(${Python3_Development_FOUND})
      set(Python3_FOUND TRUE)
      if(APPLE)

        list(GET Python3_LIBRARIES 0 pylib)
        string(REGEX REPLACE "(.*Frameworks)/Python(3.|.)framework.*" "\\1"
                             DEVELOPER_DIR ${pylib}
        )
        if("${DEVELOPER_DIR}" MATCHES "Frameworks")
          set(CMAKE_BUILD_RPATH "${DEVELOPER_DIR}" CACHE STRING "")
          set(CMAKE_INSTALL_RPATH "${DEVELOPER_DIR}" CACHE STRING "")
        endif()
      endif()
      include_directories(${Python3_INCLUDE_DIRS})
    else()
      message(${HIGHLIGHTED_STATUS}
              "Python: development libraries were not found"
      )
      set(ENABLE_PYTHON_BINDINGS_REASON "missing Python development libraries")
    endif()
  else()
    if(${NS3_PYTHON_BINDINGS})
      message(
        ${HIGHLIGHTED_STATUS}
        "Python: an incompatible version of Python was found, python bindings will be disabled"
      )
      set(ENABLE_PYTHON_BINDINGS_REASON "incompatible Python version")
    endif()
  endif()

  set(ENABLE_PYTHON_BINDINGS OFF)
  if(${NS3_PYTHON_BINDINGS})
    if(NOT ${Python3_FOUND})
      message(
        ${HIGHLIGHTED_STATUS}
        "Bindings: python bindings require Python, but it could not be found"
      )
      set(ENABLE_PYTHON_BINDINGS_REASON "missing dependency: python")
    elseif(APPLE AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
      message(${HIGHLIGHTED_STATUS}
              "Bindings: macOS silicon detected -- see issue 930"
      )
      set(ENABLE_PYTHON_BINDINGS_REASON
          "macOS silicon detected -- see issue 930"
      )
    else()
      check_python_packages("cppyy" missing_packages)
      if(missing_packages)
        message(
          ${HIGHLIGHTED_STATUS}
          "Bindings: python bindings disabled due to the following missing dependencies: ${missing_packages}"
        )
        set(ENABLE_PYTHON_BINDINGS_REASON
            "missing dependency: ${missing_packages}"
        )
      else()
        set(ENABLE_PYTHON_BINDINGS ON)
      endif()

      set(destination_dir ${CMAKE_OUTPUT_DIRECTORY}/bindings/python/ns)
      configure_file(
        bindings/python/ns__init__.py ${destination_dir}/__init__.py COPYONLY
      )

      if(NOT NS3_BINDINGS_INSTALL_DIR)
        execute_process(
          COMMAND python3 -m site --user-site
          OUTPUT_VARIABLE SUGGESTED_BINDINGS_INSTALL_DIR
        )
        string(STRIP "${SUGGESTED_BINDINGS_INSTALL_DIR}"
                     SUGGESTED_BINDINGS_INSTALL_DIR
        )
        message(
          ${HIGHLIGHTED_STATUS}
          "NS3_BINDINGS_INSTALL_DIR was not set. The python bindings won't be installed with ./ns3 install."
          "This setting is meant for packaging and redistribution."
        )
        message(
          ${HIGHLIGHTED_STATUS}
          "Set NS3_BINDINGS_INSTALL_DIR=\"${SUGGESTED_BINDINGS_INSTALL_DIR}\" to install it to the default location."
        )
      else()
        if(${NS3_BINDINGS_INSTALL_DIR} STREQUAL "INSTALL_PREFIX")
          set(NS3_BINDINGS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
        endif()
        install(FILES bindings/python/ns__init__.py
                DESTINATION ${NS3_BINDINGS_INSTALL_DIR}/ns RENAME __init__.py
        )
        add_custom_target(
          uninstall_bindings COMMAND rm -R ${NS3_BINDINGS_INSTALL_DIR}/ns
        )
        add_dependencies(uninstall uninstall_bindings)
      endif()
    endif()
  endif()

  if(${NS3_NINJA_TRACING})
    if(${CMAKE_GENERATOR} STREQUAL Ninja)
      include(ExternalProject)
      ExternalProject_Add(
        NinjaTracing
        GIT_REPOSITORY "https://github.com/nico/ninjatracing.git"
        GIT_TAG "f9d21e973cfdeafa913b83a927fef56258f70b9a"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
      )
      ExternalProject_Get_Property(NinjaTracing SOURCE_DIR)
      set(embed_time_trace)
      if(${NS3_CLANG_TIMETRACE} AND ${CLANG})
        set(embed_time_trace --embed-time-trace)
      endif()
      add_custom_target(
        ninjaTrace
        COMMAND
          ${Python3_EXECUTABLE} ${SOURCE_DIR}/ninjatracing -a
          ${embed_time_trace} ${PROJECT_BINARY_DIR}/.ninja_log >
          ${PROJECT_SOURCE_DIR}/ninja_performance_trace.json
        DEPENDS NinjaTracing
      )
      unset(embed_time_trace)
      unset(SOURCE_DIR)
    else()
      message(FATAL_ERROR "Ninjatracing requires the Ninja generator")
    endif()
  endif()

  if(${ENABLE_PYTHON_BINDINGS} AND ${CLANG})
    add_compile_options(-Wno-potentially-evaluated-expression)
  endif()

  set(ENABLE_VISUALIZER FALSE)
  if(${NS3_VISUALIZER})
    if(${NS3_PYTHON_BINDINGS})
      if(NOT ${Python3_FOUND})
        set(ENABLE_VISUALIZER_REASON "missing Python")
      elseif(NOT ${ENABLE_PYTHON_BINDINGS})
        set(ENABLE_VISUALIZER_REASON "missing Python Bindings")
      else()
        set(ENABLE_VISUALIZER TRUE)
      endif()
    else()
      set(ENABLE_VISUALIZER_REASON "Python Bindings are disabled")
    endif()
    if(ENABLE_VISUALIZER_REASON)
      message(${HIGHLIGHTED_STATUS} "Visualizer: ${ENABLE_VISUALIZER_REASON}")
    endif()
  endif()

  if(${NS3_COVERAGE} AND (NOT ${ENABLE_TESTS} OR NOT ${ENABLE_EXAMPLES}))
    message(
      FATAL_ERROR
        "Code coverage requires examples and tests.\nTry reconfiguring CMake with -DNS3_TESTS=ON -DNS3_EXAMPLES=ON"
    )
  endif()

  if(${ENABLE_TESTS})
    add_custom_target(test-runner-examples-as-tests)
    add_custom_target(all-test-targets)

    add_custom_target(
      run_test_py
      COMMAND ${Python3_EXECUTABLE} test.py --no-build
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      DEPENDS all-test-targets
    )
    if(${ENABLE_EXAMPLES})
      include(build-support/custom-modules/ns3-coverage.cmake)
    endif()
  endif()

  configure_file(
    build-support/config-store-config-template.h
    ${CMAKE_HEADER_OUTPUT_DIRECTORY}/config-store-config.h
  )

  set(ENABLE_MPI FALSE)
  if(${NS3_MPI})
    find_package(MPI QUIET)
    if(NOT ${MPI_FOUND})
      message(FATAL_ERROR "MPI was not found.")
    else()
      message(STATUS "MPI was found.")
      add_definitions(-DNS3_MPI)
      include_directories(${MPI_CXX_INCLUDE_DIRS})
      set(ENABLE_MPI TRUE)
    endif()
  endif()

  set(ENABLE_MTP FALSE)
  if(${NS3_MTP})
    add_definitions(-DNS3_MTP)
    set(ENABLE_MTP TRUE)
  endif()

  mark_as_advanced(Boost_INCLUDE_DIR)
  find_package(Boost)
  if(${Boost_FOUND})
    include_directories(${Boost_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIRS})
  endif()

  if(${NS3_GSL})
    find_package(GSL QUIET)
    if(NOT ${GSL_FOUND})
      message(${HIGHLIGHTED_STATUS} "GSL was not found. Continuing without it.")
    else()
      message(STATUS "GSL was found.")
      add_definitions(-DHAVE_GSL)
      include_directories(${GSL_INCLUDE_DIRS})
    endif()
  endif()


  mark_as_advanced(DOXYGEN)
  check_deps("" "doxygen;dot;dia;python3" doxygen_docs_missing_deps)
  if(doxygen_docs_missing_deps)
    message(
      ${HIGHLIGHTED_STATUS}
      "docs: doxygen documentation not enabled due to missing dependencies: ${doxygen_docs_missing_deps}"
    )
    set(doxygen_missing_msg
        echo The following Doxygen dependencies are missing: ${doxygen_docs_missing_deps}.
            Reconfigure the project after installing them.
    )

    add_custom_target(
      run-print-introspected-doxygen COMMAND ${doxygen_missing_msg}
    )
    add_custom_target(
      run-introspected-command-line COMMAND ${doxygen_missing_msg}
    )
    add_custom_target(
      assemble-introspected-command-line COMMAND ${doxygen_missing_msg}
    )
    add_custom_target(update_doxygen_version COMMAND ${doxygen_missing_msg})
    add_custom_target(doxygen COMMAND ${doxygen_missing_msg})
    add_custom_target(doxygen-no-build COMMAND ${doxygen_missing_msg})
  else()
    set(DOXYGEN_EXECUTABLE ${DOXYGEN})

    add_custom_target(
      update_doxygen_version
      COMMAND bash ${PROJECT_SOURCE_DIR}/doc/ns3_html_theme/get_version.sh
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    add_custom_target(
      doxygen-no-build
      COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/doc/doxygen.conf
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      DEPENDS update_doxygen_version
      USES_TERMINAL
    )
    if((NOT ${ENABLE_TESTS}) AND (NOT ${ENABLE_EXAMPLES}))
      set(doxygen_target_requires_tests_msg
              echo The \\'doxygen\\' target called by \\'./ns3 docs doxygen\\' or \\'./ns3 docs all\\' commands
              require examples and tests to generate introspected documentation.
              Enable examples and tests, or use \\'doxygen-no-build\\'.
              )
      add_custom_target(doxygen COMMAND ${doxygen_target_requires_tests_msg})
      unset(doxygen_target_requires_tests_msg)
    else()
      add_custom_target(
        run-print-introspected-doxygen
        COMMAND
          ${CMAKE_OUTPUT_DIRECTORY}/utils/ns${NS3_VER}-print-introspected-doxygen${build_profile_suffix}
          > ${PROJECT_SOURCE_DIR}/doc/introspected-doxygen.h
        COMMAND
          ${CMAKE_OUTPUT_DIRECTORY}/utils/ns${NS3_VER}-print-introspected-doxygen${build_profile_suffix}
          --output-text > ${PROJECT_SOURCE_DIR}/doc/ns3-object.txt
        DEPENDS print-introspected-doxygen
      )
      add_custom_target(
        run-introspected-command-line
        COMMAND ${CMAKE_COMMAND} -E env NS_COMMANDLINE_INTROSPECTION=..
                ${Python3_EXECUTABLE} ./test.py --no-build --constrain=example
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        DEPENDS all-test-targets
      )

      file(
        WRITE ${CMAKE_BINARY_DIR}/introspected-command-line-preamble.h
        "/* This file is automatically generated by
  CommandLine::PrintDoxygenUsage() from the CommandLine configuration
  in various example programs.  Do not edit this file!  Edit the
  CommandLine configuration in those files instead.
  */\n"
      )
      add_custom_target(
        assemble-introspected-command-line
        COMMAND
          ${cat_command}
          ${CMAKE_BINARY_DIR}/introspected-command-line-preamble.h
          ${PROJECT_SOURCE_DIR}/testpy-output/*.command-line >
          ${PROJECT_SOURCE_DIR}/doc/introspected-command-line.h 2> NULL
        DEPENDS run-introspected-command-line
      )

      add_custom_target(
        doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/doc/doxygen.conf
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        DEPENDS update_doxygen_version run-print-introspected-doxygen
                assemble-introspected-command-line
        USES_TERMINAL
      )
    endif()
  endif()

  mark_as_advanced(
    SPHINX_EXECUTABLE SPHINX_OUTPUT_HTML SPHINX_OUTPUT_MAN
    SPHINX_WARNINGS_AS_ERRORS
  )

  check_deps(
    "Sphinx" "epstopdf;pdflatex;latexmk;convert;dvipng"
    sphinx_docs_missing_deps
  )
  if(sphinx_docs_missing_deps)
    message(
      ${HIGHLIGHTED_STATUS}
      "docs: sphinx documentation not enabled due to missing dependencies: ${sphinx_docs_missing_deps}"
    )
    set(sphinx_missing_msg
        echo The following Sphinx dependencies are missing: ${sphinx_docs_missing_deps}.
            Reconfigure the project after installing them.
    )

    add_custom_target(sphinx COMMAND ${sphinx_missing_msg})
    add_custom_target(sphinx_manual COMMAND ${sphinx_missing_msg})
    add_custom_target(sphinx_models COMMAND ${sphinx_missing_msg})
    add_custom_target(sphinx_tutorial COMMAND ${sphinx_missing_msg})
    add_custom_target(sphinx_contributing COMMAND ${sphinx_missing_msg})
    add_custom_target(sphinx_installation COMMAND ${sphinx_missing_msg})
  else()
    add_custom_target(sphinx COMMENT "Building sphinx documents")
    mark_as_advanced(MAKE)
    find_program(MAKE NAMES make mingw32-make)
    if(${MAKE} STREQUAL "MAKE-NOTFOUND")
      message(
        FATAL_ERROR "Make was not found but it is required by Sphinx docs"
      )
    elseif(${MAKE} MATCHES "mingw32-make")
      get_filename_component(make_directory ${MAKE} DIRECTORY)
      get_filename_component(make_parent_directory ${make_directory} DIRECTORY)
      if(NOT (EXISTS ${make_directory}/make.exe))
        file(COPY ${MAKE} DESTINATION ${make_parent_directory})
        file(RENAME ${make_parent_directory}/mingw32-make.exe
             ${make_directory}/make.exe
        )
      endif()
      set(MAKE ${make_directory}/make.exe)
    else()

    endif()

    function(sphinx_target targetname)
      add_custom_target(
        sphinx_${targetname}
        COMMAND ${MAKE} SPHINXOPTS=-N -k html singlehtml latexpdf
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc/${targetname}
      )
      add_dependencies(sphinx sphinx_${targetname})
    endfunction()
    sphinx_target(manual)
    sphinx_target(models)
    sphinx_target(tutorial)
    sphinx_target(contributing)
    sphinx_target(installation)
  endif()

  include(CheckCXXSourceCompiles)

  if(${NS3_INT64X64} MATCHES "INT128")
    check_cxx_source_compiles(
      "#include <stdint.h>
       int main()
         {
            if ((uint128_t *) 0) return 0;
            if (sizeof (uint128_t)) return 0;
            return 1;
         }"
      HAVE_UINT128_T
    )
    check_cxx_source_compiles(
      "#include <stdint.h>
       int main()
         {
           if ((__uint128_t *) 0) return 0;
           if (sizeof (__uint128_t)) return 0;
           return 1;
        }"
      HAVE___UINT128_T
    )
    if(HAVE_UINT128_T OR HAVE___UINT128_T)
      set(INT64X64_USE_128 TRUE)
    else()
      message(${HIGHLIGHTED_STATUS}
              "Int128 was not found. Falling back to Cairo."
      )
      set(NS3_INT64X64 "CAIRO")
    endif()
  endif()

  if(${NS3_INT64X64} MATCHES "DOUBLE")
    include(CheckTypeSize)
    check_type_size("double" SIZEOF_DOUBLE)
    check_type_size("long double" SIZEOF_LONG_DOUBLE)

    if(${SIZEOF_LONG_DOUBLE} EQUAL ${SIZEOF_DOUBLE})
      message(
        STATUS
          "Long double has the wrong size: LD ${SIZEOF_LONG_DOUBLE} vs D ${SIZEOF_DOUBLE}. Falling back to CAIRO."
      )
      set(NS3_INT64X64 "CAIRO")
    else()
      set(INT64X64_USE_DOUBLE TRUE)
    endif()
  endif()

  if(${NS3_INT64X64} MATCHES "CAIRO")
    set(INT64X64_USE_CAIRO TRUE)
  endif()

  check_include_file("stdint.h" "HAVE_STDINT_H")
  check_include_file("inttypes.h" "HAVE_INTTYPES_H")
  check_include_file("sys/types.h" "HAVE_SYS_TYPES_H")
  check_include_file("sys/stat.h" "HAVE_SYS_STAT_H")
  check_include_file("dirent.h" "HAVE_DIRENT_H")
  check_include_file("stdlib.h" "HAVE_STDLIB_H")
  check_include_file("signal.h" "HAVE_SIGNAL_H")
  check_include_file("netpacket/packet.h" "HAVE_PACKETH")
  check_function_exists("getenv" "HAVE_GETENV")

  configure_file(
    build-support/core-config-template.h
    ${CMAKE_HEADER_OUTPUT_DIRECTORY}/core-config.h
  )

  if(${NS3_LOG} OR (${build_profile} STREQUAL "debug"))
    add_definitions(-DNS3_LOG_ENABLE)
  endif()
  if(${NS3_ASSERT} OR (${build_profile} STREQUAL "debug"))
    add_definitions(-DNS3_ASSERT_ENABLE)
  endif()

  set(ENABLE_TAP OFF)
  if(${NS3_TAP})
    set(ENABLE_TAP ON)
  endif()

  set(ENABLE_EMU OFF)
  if(${NS3_EMU})
    set(ENABLE_EMU ON)
  endif()

  set(PLATFORM_UNSUPPORTED_PRE "Platform doesn't support")
  set(PLATFORM_UNSUPPORTED_POST "features. Continuing without them.")
  if(APPLE OR WSLv1 OR WIN32)
    set(ENABLE_TAP OFF)
    set(ENABLE_EMU OFF)
    list(REMOVE_ITEM libs_to_build fd-net-device)
    message(
      STATUS
        "${PLATFORM_UNSUPPORTED_PRE} TAP and EMU ${PLATFORM_UNSUPPORTED_POST}"
    )
  endif()

  if(NOT ${ENABLE_MPI})
    list(REMOVE_ITEM libs_to_build mpi)
  endif()

  if(NOT ${ENABLE_MTP})
    list(REMOVE_ITEM libs_to_build mtp)
  endif()

  if(NOT ${ENABLE_VISUALIZER})
    list(REMOVE_ITEM libs_to_build visualizer)
  endif()

  if(NOT ${ENABLE_TAP})
    list(REMOVE_ITEM libs_to_build tap-bridge)
  endif()

  set(ns3-libs)
  set(ns3-all-enabled-modules)
  set(ns3-libs-tests)
  set(ns3-contrib-libs)
  set(lib-ns3-static-objs)
  set(ns3-external-libs)

  foreach(libname ${scanned_modules})
    library_target_name(${libname} targetname)
    mark_as_advanced(lib${libname} lib${libname}-obj)
    set(lib${libname} ${targetname} CACHE INTERNAL "")
    set(lib${libname}-obj ${targetname}-obj CACHE INTERNAL "")
  endforeach()

  unset(optional_visualizer_lib)
  if(${ENABLE_VISUALIZER} AND (visualizer IN_LIST libs_to_build))
    set(optional_visualizer_lib ${libvisualizer})
  endif()

  set(PRECOMPILE_HEADERS_ENABLED OFF)
  if(${NS3_PRECOMPILE_HEADERS})
    if(${NS3_CLANG_TIDY})
      message(
        ${HIGHLIGHTED_STATUS}
        "Clang-tidy is incompatible with precompiled headers. Continuing without them."
      )
    elseif(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
      if((NOT ${NS3_CCACHE}) OR ("${CCACHE}" STREQUAL "CCACHE-NOTFOUND"))
        set(PRECOMPILE_HEADERS_ENABLED ON)
        message(STATUS "Precompiled headers were enabled")
      else()
        execute_process(COMMAND ${CCACHE} -V OUTPUT_VARIABLE CCACHE_OUT)
        if(CCACHE_OUT MATCHES "ccache version ([0-9\.]*)")
          if("${CMAKE_MATCH_1}" VERSION_LESS "4.0.0")
            set(PRECOMPILE_HEADERS_ENABLED OFF)
            message(
              ${HIGHLIGHTED_STATUS}
              "Precompiled headers are incompatible with ccache ${CMAKE_MATCH_1} and will be disabled."
            )
          else()
            set(PRECOMPILE_HEADERS_ENABLED ON)
            message(STATUS "Precompiled headers were enabled.")
          endif()
        else()
          message(
            FATAL_ERROR
              "Failed to extract the ccache version while enabling precompiled headers."
          )
        endif()
      endif()
    else()
      message(
        STATUS
          "CMake ${CMAKE_VERSION} does not support precompiled headers. Continuing without them"
      )
    endif()
  endif()

  if(${PRECOMPILE_HEADERS_ENABLED})
    if(CLANG)
      add_definitions(-Xclang -fno-pch-timestamp)
    endif()
    if(${XCODE})
      add_definitions(-Xclang -fno-validate-pch)
    endif()
    set(precompiled_header_libraries
        <algorithm>
        <cstdlib>
        <cstring>
        <exception>
        <fstream>
        <iostream>
        <limits>
        <list>
        <map>
        <math.h>
        <ostream>
        <set>
        <sstream>
        <stdint.h>
        <stdlib.h>
        <string>
        <unordered_map>
        <vector>
    )
    add_library(
      stdlib_pch${build_profile_suffix} OBJECT
      ${PROJECT_SOURCE_DIR}/build-support/empty.cc
    )
    target_precompile_headers(
      stdlib_pch${build_profile_suffix} PUBLIC
      "${precompiled_header_libraries}"
    )
    add_library(stdlib_pch ALIAS stdlib_pch${build_profile_suffix})

    add_executable(
      stdlib_pch_exec ${PROJECT_SOURCE_DIR}/build-support/empty-main.cc
    )
    target_precompile_headers(
      stdlib_pch_exec PUBLIC "${precompiled_header_libraries}"
    )
    set_runtime_outputdirectory(stdlib_pch_exec ${CMAKE_BINARY_DIR}/ "")
  endif()

  set(lib-ns3-static ns${NS3_VER}-static${build_profile_suffix})

  set(lib-ns3-monolib ns${NS3_VER}-monolib${build_profile_suffix})

  process_contribution("${contrib_libs_to_build}")

  if(${NS3_NETANIM})
    include(FetchContent)
    FetchContent_Declare(
      netanim GIT_REPOSITORY https://gitlab.com/nsnam/netanim.git
      GIT_TAG netanim-3.109
    )
    FetchContent_Populate(netanim)
    file(COPY build-support/3rd-party/netanim-cmakelists.cmake
         DESTINATION ${netanim_SOURCE_DIR}
    )
    file(RENAME ${netanim_SOURCE_DIR}/netanim-cmakelists.cmake
         ${netanim_SOURCE_DIR}/CMakeLists.txt
    )
    add_subdirectory(${netanim_SOURCE_DIR} ${netanim_BINARY_DIR})
  endif()

  if(${NS3_FETCH_OPTIONAL_COMPONENTS})
    include(
      build-support/custom-modules/ns3-fetch-optional-modules-dependencies.cmake
    )
  endif()
endmacro()

function(set_runtime_outputdirectory target_name output_directory target_prefix)
  string(REPLACE "//" "/" output_directory "${output_directory}")

  set(ns3-exec-outputname ns${NS3_VER}-${target_name}${build_profile_suffix})
  set(ns3-execs "${output_directory}${ns3-exec-outputname};${ns3-execs}"
      CACHE INTERNAL "list of c++ executables"
  )
  set(ns3-execs-clean "${target_prefix}${target_name};${ns3-execs-clean}"
      CACHE INTERNAL
            "list of c++ executables without version prefix and build suffix"
  )

  set_target_properties(
    ${target_prefix}${target_name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_directory}
               RUNTIME_OUTPUT_NAME ${ns3-exec-outputname}
  )
  if(${XCODE})
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
      set_target_properties(
        ${target_prefix}${target_name}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${output_directory}
                   RUNTIME_OUTPUT_NAME_${OUTPUTCONFIG} ${ns3-exec-outputname}
      )
    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
  endif()

  if(${ENABLE_TESTS})
    add_dependencies(all-test-targets ${target_prefix}${target_name})
    if(WIN32)
      add_test(
        NAME ctest-${target_prefix}${target_name}
        COMMAND
          ${CMAKE_COMMAND} -E env
          "PATH=$ENV{PATH};${CMAKE_RUNTIME_OUTPUT_DIRECTORY};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
          ${ns3-exec-outputname}
        WORKING_DIRECTORY ${output_directory}
      )
    else()
      add_test(NAME ctest-${target_prefix}${target_name}
               COMMAND ${ns3-exec-outputname}
               WORKING_DIRECTORY ${output_directory}
      )
    endif()
  endif()

  if(${NS3_CLANG_TIMETRACE})
    add_dependencies(timeTraceReport ${target_prefix}${target_name})
  endif()
endfunction(set_runtime_outputdirectory)

function(get_scratch_prefix prefix)
  set(temp ${CMAKE_CURRENT_SOURCE_DIR})
  string(REPLACE "${PROJECT_SOURCE_DIR}/" "" temp "${temp}")
  string(REPLACE "/" "_" temp "${temp}")
  set(${prefix} ${temp}_ PARENT_SCOPE)
endfunction()

function(build_exec)
  set(options IGNORE_PCH STANDALONE)
  set(oneValueArgs EXECNAME EXECNAME_PREFIX EXECUTABLE_DIRECTORY_PATH
                   INSTALL_DIRECTORY_PATH
  )
  set(multiValueArgs SOURCE_FILES HEADER_FILES LIBRARIES_TO_LINK DEFINITIONS)
  cmake_parse_arguments(
    "BEXEC" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )

  string(REPLACE "${PROJECT_SOURCE_DIR}" "" relative_path
                 "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  if("${relative_path}" MATCHES "scratch" AND "${BEXEC_EXECNAME_PREFIX}"
                                              STREQUAL ""
  )
    get_scratch_prefix(BEXEC_EXECNAME_PREFIX)
  endif()

  add_executable(
    ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME} "${BEXEC_SOURCE_FILES}"
  )

  target_compile_definitions(
    ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME} PUBLIC ${BEXEC_DEFINITIONS}
  )

  if(${PRECOMPILE_HEADERS_ENABLED} AND (NOT ${BEXEC_IGNORE_PCH}))
    target_precompile_headers(
      ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME} REUSE_FROM stdlib_pch_exec
    )
  endif()

  if(${NS3_STATIC} AND (NOT BEXEC_STANDALONE))
    target_link_libraries(
      ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME} ${LIB_AS_NEEDED_PRE_STATIC}
      ${lib-ns3-static}
    )
  elseif(${NS3_MONOLIB} AND (NOT BEXEC_STANDALONE))
    target_link_libraries(
      ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME} ${LIB_AS_NEEDED_PRE}
      ${lib-ns3-monolib} ${LIB_AS_NEEDED_POST}
    )
  else()
    target_link_libraries(
      ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME} ${LIB_AS_NEEDED_PRE}
      "${BEXEC_LIBRARIES_TO_LINK}" ${LIB_AS_NEEDED_POST}
    )
  endif()

  set_runtime_outputdirectory(
    "${BEXEC_EXECNAME}" "${BEXEC_EXECUTABLE_DIRECTORY_PATH}/"
    "${BEXEC_EXECNAME_PREFIX}"
  )

  if(BEXEC_INSTALL_DIRECTORY_PATH)
    install(TARGETS ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME}
            EXPORT ns3ExportTargets
            RUNTIME DESTINATION ${BEXEC_INSTALL_DIRECTORY_PATH}
    )
    get_property(
      filename TARGET ${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME}
      PROPERTY RUNTIME_OUTPUT_NAME
    )
    add_custom_target(
      uninstall_${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME}
      COMMAND
        rm ${CMAKE_INSTALL_PREFIX}/${BEXEC_INSTALL_DIRECTORY_PATH}/${filename}
    )
    add_dependencies(
      uninstall uninstall_${BEXEC_EXECNAME_PREFIX}${BEXEC_EXECNAME}
    )
  endif()
endfunction(build_exec)

function(scan_python_examples path)
  if(NOT ${ENABLE_PYTHON_BINDINGS})
    return()
  endif()

  file(GLOB_RECURSE python_examples ${path}/*.py)
  foreach(python_example ${python_examples})
    if(NOT (${python_example} MATCHES "examples-to-run"))
      set(ns3-execs-py "${python_example};${ns3-execs-py}"
          CACHE INTERNAL "list of python scripts"
      )
    endif()
  endforeach()
endfunction()

add_custom_target(copy_all_headers)
function(copy_headers_before_building_lib libname outputdir headers visibility)
  foreach(header ${headers})

    get_filename_component(
      header_name ${CMAKE_CURRENT_SOURCE_DIR}/${header} NAME
    )

    if(NOT (EXISTS ${outputdir}))
      file(MAKE_DIRECTORY ${outputdir})
    endif()

    if(EXISTS ${outputdir}/${header_name})
      continue()
    endif()

    get_filename_component(
      ABSOLUTE_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${header}" ABSOLUTE
    )
    file(WRITE ${outputdir}/${header_name}
         "#include \"${ABSOLUTE_HEADER_PATH}\"\n"
    )
  endforeach()
endfunction(copy_headers_before_building_lib)

function(remove_lib_prefix prefixed_library library)
  string(FIND "${prefixed_library}" "lib" lib_pos)

  if(${lib_pos} EQUAL 0)
    string(LENGTH ${prefixed_library} len)
    if(${len} LESS 4)
      message(FATAL_ERROR "Invalid library name: ${prefixed_library}")
    endif()

    string(SUBSTRING "${prefixed_library}" 3 -1 unprefixed_library)
  else()
    set(unprefixed_library ${prefixed_library})
  endif()

  set(${library} ${unprefixed_library} PARENT_SCOPE)
endfunction()

function(check_for_missing_libraries output_variable_name libraries)
  set(missing_dependencies)
  foreach(lib ${libraries})
    if(EXISTS ${lib})
      continue()
    endif()

    remove_lib_prefix("${lib}" lib)

    if(NOT (${lib} IN_LIST ns3-all-enabled-modules))
      list(APPEND missing_dependencies ${lib})
    endif()
  endforeach()
  set(${output_variable_name} ${missing_dependencies} PARENT_SCOPE)
endfunction()

include(build-support/custom-modules/ns3-module-macros.cmake)

include(build-support/custom-modules/ns3-contributions.cmake)

macro(build_example)
  set(options IGNORE_PCH)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCE_FILES HEADER_FILES LIBRARIES_TO_LINK)
  cmake_parse_arguments(
    "EXAMPLE" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )

  set(filtered_in ON)
  if(NS3_FILTER_MODULE_EXAMPLES_AND_TESTS)
    set(filtered_in OFF)
    foreach(filtered_module NS3_FILTER_MODULE_EXAMPLES_AND_TESTS)
      if(${filtered_module} IN_LIST EXAMPLE_LIBRARIES_TO_LINK)
        set(filtered_in ON)
      endif()
    endforeach()
  endif()

  check_for_missing_libraries(
    missing_dependencies "${EXAMPLE_LIBRARIES_TO_LINK}"
  )

  if((NOT missing_dependencies) AND ${filtered_in})
    if(${EXAMPLE_IGNORE_PCH})
      set(IGNORE_PCH IGNORE_PCH)
    endif()
    build_exec(
      EXECNAME ${EXAMPLE_NAME}
      SOURCE_FILES ${EXAMPLE_SOURCE_FILES}
      HEADER_FILES ${EXAMPLE_HEADER_FILES}
      LIBRARIES_TO_LINK ${EXAMPLE_LIBRARIES_TO_LINK} ${optional_visualizer_lib}
      EXECUTABLE_DIRECTORY_PATH
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/examples/${examplefolder}/
      ${IGNORE_PCH}
    )
  endif()
endmacro()

function(filter_modules modules_to_filter all_modules_list filter_in)
  set(new_modules_to_build)
  foreach(module ${${all_modules_list}})
    if(${filter_in} (${module} IN_LIST ${modules_to_filter}))
      list(APPEND new_modules_to_build ${module})
    endif()
  endforeach()
  set(${all_modules_list} ${new_modules_to_build} PARENT_SCOPE)
endfunction()

function(resolve_dependencies module_name dependencies contrib_dependencies)
  set(dependency_visited "" CACHE INTERNAL "")
  set(contrib_dependency_visited "" CACHE INTERNAL "")
  recursive_dependency(${module_name})
  if(${module_name} IN_LIST dependency_visited)
    set(temp ${dependency_visited})
    list(REMOVE_AT temp 0)
    set(${dependencies} ${temp} PARENT_SCOPE)
    set(${contrib_dependencies} ${contrib_dependency_visited} PARENT_SCOPE)
  else()
    set(temp ${contrib_dependency_visited})
    list(REMOVE_AT temp 0)
    set(${dependencies} ${dependency_visited} PARENT_SCOPE)
    set(${contrib_dependencies} ${temp} PARENT_SCOPE)
  endif()
  unset(dependency_visited CACHE)
  unset(contrib_dependency_visited CACHE)
endfunction()

function(filter_libraries cmakelists_contents libraries)
  string(REGEX MATCHALL "{lib[^}]*[^obj]}" matches "${cmakelists_content}")
  list(REMOVE_ITEM matches "{libraries_to_link}")
  string(REPLACE "{lib\${name" "" matches "${matches}")
  string(REPLACE "{lib" "" matches "${matches}")
  string(REPLACE "}" "" matches "${matches}")
  set(${libraries} ${matches} PARENT_SCOPE)
endfunction()

function(recursive_dependency module_name)
  set(src_cmakelist ${PROJECT_SOURCE_DIR}/src/${module_name}/CMakeLists.txt)
  set(contrib_cmakelist
      ${PROJECT_SOURCE_DIR}/contrib/${module_name}/CMakeLists.txt
  )
  set(contrib FALSE)
  if(EXISTS ${src_cmakelist})
    file(READ ${src_cmakelist} cmakelists_content)
  elseif(EXISTS ${contrib_cmakelist})
    file(READ ${contrib_cmakelist} cmakelists_content)
    set(contrib TRUE)
  else()
    set(cmakelists_content "")
    message(${HIGHLIGHTED_STATUS}
            "The CMakeLists.txt file for module ${module_name} was not found."
    )
  endif()

  filter_libraries("${cmakelists_content}" matches)

  if(contrib)
    set(contrib_dependency_visited
        "${contrib_dependency_visited};${module_name}" CACHE INTERNAL ""
    )
    set(examples_cmakelists ${contrib_cmakelist})
  else()
    set(dependency_visited "${dependency_visited};${module_name}" CACHE INTERNAL
                                                                        ""
    )
    set(examples_cmakelists ${src_cmakelist})
  endif()


  set(matches "${matches};${example_matches}")
  foreach(match ${matches})
    if(NOT ((${match} IN_LIST dependency_visited)
            OR (${match} IN_LIST contrib_dependency_visited))
    )
      recursive_dependency(${match})
    endif()
  endforeach()
endfunction()

macro(
  filter_enabled_and_disabled_modules
  libs_to_build
  contrib_libs_to_build
  NS3_ENABLED_MODULES
  NS3_DISABLED_MODULES
  ns3rc_enabled_modules
  ns3rc_disabled_modules
)
  mark_as_advanced(ns3-all-enabled-modules)

  set(scanned_modules ${${libs_to_build}})

  string(REPLACE "," ";" ${NS3_ENABLED_MODULES} "${${NS3_ENABLED_MODULES}}")
  string(REPLACE "," ";" ${NS3_DISABLED_MODULES} "${${NS3_DISABLED_MODULES}}")

  if(${NS3_ENABLED_MODULES} OR ${ns3rc_enabled_modules})
    if(${NS3_ENABLED_MODULES})
      set(ns3rc_enabled_modules ${${NS3_ENABLED_MODULES}})
    endif()

    filter_modules(ns3rc_enabled_modules libs_to_build "")
    filter_modules(ns3rc_enabled_modules contrib_libs_to_build "")

    foreach(lib ${${contrib_libs_to_build}})
      resolve_dependencies(${lib} dependencies contrib_dependencies)
      list(APPEND ${contrib_libs_to_build} "${contrib_dependencies}")
      list(APPEND ${libs_to_build} "${dependencies}")
      unset(dependencies)
      unset(contrib_dependencies)
    endforeach()

    foreach(lib ${${libs_to_build}})
      resolve_dependencies(${lib} dependencies contrib_dependencies)
      list(APPEND ${libs_to_build} "${dependencies}")
      unset(dependencies)
      unset(contrib_dependencies)
    endforeach()
  endif()

  if(${NS3_DISABLED_MODULES} OR ${ns3rc_disabled_modules})

    if(${NS3_DISABLED_MODULES})
      set(ns3rc_disabled_modules ${${NS3_DISABLED_MODULES}})
    endif()

    set(all_libs ${${libs_to_build}};${${contrib_libs_to_build}})

    foreach(lib ${all_libs})
      resolve_dependencies(${lib} dependencies contrib_dependencies)
      set(${lib}_dependencies "${dependencies};${contrib_dependencies}")
      unset(dependencies)
      unset(contrib_dependencies)
    endforeach()

    set(disabled_libs "${${ns3rc_disabled_modules}}")
    foreach(libo ${all_libs})
      foreach(lib ${all_libs})
        foreach(disabled_lib ${disabled_libs})
          if(${lib} STREQUAL ${disabled_lib})
            continue()
          endif()
          if(${disabled_lib} IN_LIST ${lib}_dependencies)
            list(APPEND disabled_libs ${lib})
            break()
          endif()
        endforeach()
      endforeach()
    endforeach()

    foreach(lib ${all_libs})
      unset(${lib}_dependencies)
    endforeach()

    filter_modules(disabled_libs libs_to_build "NOT")
    filter_modules(disabled_libs contrib_libs_to_build "NOT")

    if(core IN_LIST ${libs_to_build})
      list(APPEND ${libs_to_build} test)
    endif()
  endif()

  if(NOT ${contrib_libs_to_build})
    set(${contrib_libs_to_build} "")
  endif()

  list(REMOVE_DUPLICATES ${libs_to_build})
  list(REMOVE_DUPLICATES ${contrib_libs_to_build})

  set(ns3-all-enabled-modules "${${libs_to_build}};${${contrib_libs_to_build}}"
      CACHE INTERNAL "list with all enabled modules"
  )
endmacro()

macro(parse_ns3rc enabled_modules disabled_modules examples_enabled
      tests_enabled
)
  find_file(NS3RC .ns3rc PATHS /etc $ENV{HOME} $ENV{USERPROFILE}
                               ${PROJECT_SOURCE_DIR} NO_CACHE
  )

  set(${enabled_modules} "")
  set(${disabled_modules} "")
  set(${examples_enabled} "FALSE")
  set(${tests_enabled} "FALSE")

  if(NOT (${NS3RC} STREQUAL "NS3RC-NOTFOUND"))
    message(${HIGHLIGHTED_STATUS}
            "Configuration file .ns3rc being used : ${NS3RC}"
    )
    file(READ ${NS3RC} ns3rc_contents)
    if(ns3rc_contents MATCHES "ns3rc_*")
      include(${NS3RC})
    else()
      parse_python_ns3rc(
        "${ns3rc_contents}" ${enabled_modules} ${examples_enabled}
        ${tests_enabled} ${NS3RC}
      )
    endif()
  endif()
endmacro(parse_ns3rc)

function(parse_python_ns3rc ns3rc_contents enabled_modules examples_enabled
         tests_enabled ns3rc_location
)
  file(WRITE ${ns3rc_location}.backup ${ns3rc_contents})

  if(ns3rc_contents MATCHES "modules_enabled.*\\[(.*).*\\]")
    set(${enabled_modules} ${CMAKE_MATCH_1})
    if(${enabled_modules} MATCHES "all_modules")
      set(${enabled_modules})
    else()
      string(REPLACE "," ";" ${enabled_modules} "${${enabled_modules}}")
      string(REPLACE "'" "" ${enabled_modules} "${${enabled_modules}}")
      string(REPLACE "\"" "" ${enabled_modules} "${${enabled_modules}}")
      string(REPLACE " " "" ${enabled_modules} "${${enabled_modules}}")
      string(REPLACE "\n" ";" ${enabled_modules} "${${enabled_modules}}")
      list(SORT ${enabled_modules})

      list(REMOVE_ITEM ${enabled_modules} "")
      foreach(element ${${enabled_modules}})
        if(${element} MATCHES "#.*")
          list(REMOVE_ITEM ${enabled_modules} ${element})
        endif()
      endforeach()
    endif()
  endif()

  string(REPLACE "True" "ON" ns3rc_contents ${ns3rc_contents})
  string(REPLACE "False" "OFF" ns3rc_contents ${ns3rc_contents})

  if(ns3rc_contents MATCHES "examples_enabled = (ON|OFF)")
    set(${examples_enabled} ${CMAKE_MATCH_1})
  endif()

  if(ns3rc_contents MATCHES "tests_enabled = (ON|OFF)")
    set(${tests_enabled} ${CMAKE_MATCH_1})
  endif()

  set(${enabled_modules} "${${enabled_modules}}" PARENT_SCOPE)
  set(${examples_enabled} "${${examples_enabled}}" PARENT_SCOPE)
  set(${tests_enabled} "${${tests_enabled}}" PARENT_SCOPE)

  message(
    ${HIGHLIGHTED_STATUS}
    "The python-based .ns3rc file format is deprecated and was updated to the CMake format"
  )
  configure_file(
    ${PROJECT_SOURCE_DIR}/build-support/.ns3rc-template ${ns3rc_location} @ONLY
  )
endfunction(parse_python_ns3rc)

function(log_find_searched_paths)
  set(options)
  set(oneValueArgs TARGET_TYPE TARGET_NAME SEARCH_RESULT SEARCH_SYSTEM_PREFIX)
  set(multiValueArgs SEARCH_PATHS SEARCH_SUFFIXES)
  cmake_parse_arguments(
    "LOGFIND" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )

  set(tsearch_paths ${LOGFIND_SEARCH_PATHS})
  if("${LOGFIND_SEARCH_SYSTEM_PREFIX}" STREQUAL "")
    list(APPEND tsearch_paths "${CMAKE_SYSTEM_PREFIX_PATH}")
  endif()

  set(log_find
      "Looking for ${LOGFIND_TARGET_TYPE} ${LOGFIND_TARGET_NAME} in:\n"
  )
  foreach(tsearch_path ${tsearch_paths})
    foreach(suffix ${LOGFIND_SEARCH_SUFFIXES})
      string(APPEND log_find
             "\t${tsearch_path}${suffix}/${LOGFIND_TARGET_NAME}\n"
      )
    endforeach()
  endforeach()

  if("${${LOGFIND_SEARCH_RESULT}}" STREQUAL "${LOGFIND_SEARCH_RESULT}-NOTFOUND")
    string(APPEND log_find
           "\n\t${LOGFIND_TARGET_TYPE} ${LOGFIND_TARGET_NAME} was not found\n"
    )
  else()
    string(
      APPEND
      log_find
      "\n\t${LOGFIND_TARGET_TYPE} ${LOGFIND_TARGET_NAME} was found in ${${LOGFIND_SEARCH_RESULT}}\n"
    )
  endif()

  string(REPLACE "//" "/" log_find "${log_find}")

  message(STATUS ${log_find})
endfunction()

function(find_external_library)
  set(options QUIET)
  set(oneValueArgs DEPENDENCY_NAME HEADER_NAME LIBRARY_NAME)
  set(multiValueArgs HEADER_NAMES LIBRARY_NAMES PATH_SUFFIXES SEARCH_PATHS)
  cmake_parse_arguments(
    "FIND_LIB" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )

  set(name ${FIND_LIB_DEPENDENCY_NAME})

  set(library_names "${FIND_LIB_LIBRARY_NAME};${FIND_LIB_LIBRARY_NAMES}")
  set(header_names "${FIND_LIB_HEADER_NAME};${FIND_LIB_HEADER_NAMES}")

  set(search_paths ${FIND_LIB_SEARCH_PATHS})
  set(path_suffixes "${FIND_LIB_PATH_SUFFIXES}")

  set(not_found_libraries)
  set(library_dirs)
  set(libraries)

  get_filename_component(parent_project_dir ${PROJECT_SOURCE_DIR} DIRECTORY)
  get_filename_component(
    grandparent_project_dir ${parent_project_dir} DIRECTORY
  )
  set(project_parent_dirs ${parent_project_dir} ${grandparent_project_dir})

  set(library_search_paths
      ${search_paths}
      ${project_parent_dirs}
      ${CMAKE_OUTPUT_DIRECTORY}
      ${CMAKE_INSTALL_PREFIX}
      $ENV{LD_LIBRARY_PATH}
      $ENV{PATH}
  )
  string(REPLACE ":" ";" library_search_paths "${library_search_paths}")

  set(suffixes /build /lib /build/lib / /bin ${path_suffixes})

  foreach(library ${library_names})
    mark_as_advanced(${name}_library_internal_${library})

    find_library(
      ${name}_library_internal_${library} ${library}
      HINTS ${library_search_paths} PATH_SUFFIXES ${suffixes}
    )

    if(${NS3_VERBOSE} AND (${CMAKE_VERSION} VERSION_LESS "3.17.0"))
      log_find_searched_paths(
        TARGET_TYPE
        Library
        TARGET_NAME
        ${library}
        SEARCH_RESULT
        ${name}_library_internal_${library}
        SEARCH_PATHS
        ${library_search_paths}
        SEARCH_SUFFIXES
        ${suffixes}
      )
    endif()

    if("${${name}_library_internal_${library}}" STREQUAL
       "${name}_library_internal_${library}-NOTFOUND"
    )
      list(APPEND not_found_libraries ${library})
    else()
      get_filename_component(
        ${name}_library_dir_internal ${${name}_library_internal_${library}}
        DIRECTORY
      )
      list(APPEND library_dirs ${${name}_library_dir_internal})
      list(APPEND libraries ${${name}_library_internal_${library}})
    endif()
  endforeach()

  set(parent_dirs)
  foreach(libdir ${library_dirs})
    get_filename_component(parent_libdir ${libdir} DIRECTORY)
    get_filename_component(parent_parent_libdir ${parent_libdir} DIRECTORY)
    list(APPEND parent_dirs ${libdir} ${parent_libdir} ${parent_parent_libdir})
  endforeach()

  set(header_search_paths
      ${search_paths}
      ${parent_dirs}
      ${project_parent_dirs}
      ${CMAKE_OUTPUT_DIRECTORY}
      ${CMAKE_INSTALL_PREFIX}
  )

  set(not_found_headers)
  set(include_dirs)
  foreach(header ${header_names})
    mark_as_advanced(${name}_header_internal_${header})
    set(suffixes
        /build
        /include
        /build/include
        /build/include/${name}
        /include/${name}
        /${name}
        /
        ${path_suffixes}
    )


    find_file(${name}_header_internal_${header} ${header}
              HINTS ${header_search_paths}
                    ${header_skip_system_prefix} PATH_SUFFIXES ${suffixes}
    )

    if(${NS3_VERBOSE} AND (${CMAKE_VERSION} VERSION_LESS "3.17.0"))
      log_find_searched_paths(
        TARGET_TYPE
        Header
        TARGET_NAME
        ${header}
        SEARCH_RESULT
        ${name}_header_internal_${header}
        SEARCH_PATHS
        ${header_search_paths}
        SEARCH_SUFFIXES
        ${suffixes}
        SEARCH_SYSTEM_PREFIX
        ${header_skip_system_prefix}
      )
    endif()

    if("${${name}_header_internal_${header}}" STREQUAL
       "${name}_header_internal_${header}-NOTFOUND"
    )
      list(APPEND not_found_headers ${header})
    else()
      get_filename_component(
        header_include_dir ${${name}_header_internal_${header}} DIRECTORY
      )
      get_filename_component(
        header_include_dir2 ${header_include_dir} DIRECTORY
      )
      list(APPEND include_dirs ${header_include_dir} ${header_include_dir2})
    endif()
  endforeach()

  if(include_dirs)
    list(REMOVE_DUPLICATES include_dirs)
  endif()

  if((NOT not_found_libraries) AND (NOT not_found_headers))
    set(${name}_INCLUDE_DIRS "${include_dirs}" PARENT_SCOPE)
    set(${name}_LIBRARIES "${libraries}" PARENT_SCOPE)
    set(${name}_HEADER ${${name}_header_internal} PARENT_SCOPE)
    set(${name}_FOUND TRUE PARENT_SCOPE)
    if(NOT ${FIND_LIB_QUIET})
      message(STATUS "find_external_library: ${name} was found.")
    endif()
  else()
    set(${name}_INCLUDE_DIRS PARENT_SCOPE)
    set(${name}_LIBRARIES PARENT_SCOPE)
    set(${name}_HEADER PARENT_SCOPE)
    set(${name}_FOUND FALSE PARENT_SCOPE)
    if(NOT ${FIND_LIB_QUIET})
      message(
        ${HIGHLIGHTED_STATUS}
        "find_external_library: ${name} was not found. Missing headers: \"${not_found_headers}\" and missing libraries: \"${not_found_libraries}\"."
      )
    endif()
  endif()
endfunction()

function(get_target_includes target output)
  set(include_directories)
  get_target_property(include_dirs ${target} INCLUDE_DIRECTORIES)
  list(REMOVE_DUPLICATES include_dirs)
  foreach(include_dir ${include_dirs})
    if(include_dir MATCHES "<")
      continue()
    else()
      set(include_directories ${include_directories} -I${include_dir})
    endif()
  endforeach()
  set(${output} ${include_directories} PARENT_SCOPE)
endfunction()

function(check_python_packages packages missing_packages)
  set(missing)
  foreach(package ${packages})
    execute_process(
      COMMAND ${Python3_EXECUTABLE} -c "import ${package}"
      RESULT_VARIABLE return_code OUTPUT_QUIET ERROR_QUIET
    )
    if(NOT (${return_code} EQUAL 0))
      list(APPEND missing ${package})
    endif()
  endforeach()
  set(${missing_packages} "${missing}" PARENT_SCOPE)
endfunction()

include(build-support/custom-modules/ns3-lock.cmake)
include(build-support/custom-modules/ns3-configtable.cmake)
