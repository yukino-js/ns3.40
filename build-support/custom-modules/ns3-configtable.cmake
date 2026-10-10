



set(ON ON)
macro(check_on_or_off user_config_switch confirmation_flag)
  if(${${user_config_switch}})
    if(${${confirmation_flag}})
      string(APPEND out "${Green}ON${ColourReset}\n")
    else()
      if(${confirmation_flag}_REASON)
        string(APPEND out
               "${Red}OFF (${${confirmation_flag}_REASON})${ColourReset}\n"
        )
      else()
        string(APPEND out "${Red}OFF (missing dependency)${ColourReset}\n")
      endif()
    endif()
  else()
    string(APPEND out "OFF (not requested)\n")
  endif()
endmacro()

function(print_formatted_table_with_modules table_name modules output)
  set(temp)
  string(APPEND temp "${table_name}:\n")
  set(count 0)
  set(width 26)
  string(REPLACE ";lib" ";" modules_to_print ";${modules}")
  string(SUBSTRING "${modules_to_print}" 1 -1 modules_to_print)
  list(SORT modules_to_print)
  set(modules_with_large_names)
  foreach(module ${modules_to_print})
    string(LENGTH ${module} module_name_length)

    if(${module_name_length} GREATER_EQUAL ${width})
      list(APPEND modules_with_large_names ${module})
      continue()
    endif()

    math(EXPR num_trailing_spaces "${width} - ${module_name_length}")

    string(RANDOM LENGTH ${num_trailing_spaces} ALPHABET " " trailing_spaces)

    string(APPEND temp "${module}${trailing_spaces}")
    math(EXPR count "${count} + 1")

    if(${count} EQUAL 3)
      string(APPEND temp "\n")
      set(count 0)
    endif()
  endforeach()

  foreach(module ${modules_with_large_names})
    string(APPEND temp "${module}\n")
  endforeach()
  string(APPEND temp "\n")

  set(${output} ${${output}}${temp} PARENT_SCOPE)
endfunction()

macro(write_configtable)
  set(out "---- Summary of ns-3 settings:\n")
  string(APPEND out "Build profile                 : ${build_profile}\n")
  string(APPEND out
         "Build directory               : ${CMAKE_OUTPUT_DIRECTORY}\n"
  )

  string(APPEND out "Build with runtime asserts    : ")
  check_on_or_off("NS3_ASSERT" "NS3_ASSERT")

  string(APPEND out "Build with runtime logging    : ")
  check_on_or_off("NS3_LOG" "NS3_LOG")

  string(APPEND out "Build version embedding       : ")
  check_on_or_off("NS3_ENABLE_BUILD_VERSION" "ENABLE_BUILD_VERSION")

  string(APPEND out "BRITE Integration             : ")
  check_on_or_off("ON" "NS3_BRITE")

  string(APPEND out "DES Metrics event collection  : ")
  check_on_or_off("NS3_DES_METRICS" "NS3_DES_METRICS")

  string(APPEND out "DPDK NetDevice                : ")
  check_on_or_off("NS3_DPDK" "ENABLE_DPDKDEVNET")

  string(APPEND out "Emulation FdNetDevice         : ")
  check_on_or_off("ENABLE_EMU" "ENABLE_EMUNETDEV")

  string(APPEND out "Examples                      : ")
  check_on_or_off("ENABLE_EXAMPLES" "ENABLE_EXAMPLES")

  string(APPEND out "File descriptor NetDevice     : ")
  check_on_or_off("ON" "ENABLE_FDNETDEV")

  string(APPEND out "GNU Scientific Library (GSL)  : ")
  check_on_or_off("NS3_GSL" "GSL_FOUND")

  string(APPEND out "GtkConfigStore                : ")
  check_on_or_off("NS3_GTK3" "GTK3_FOUND")

  string(APPEND out "LibXml2 support               : ")
  check_on_or_off("ON" "LIBXML2_FOUND")

  string(APPEND out "MPI Support                   : ")
  check_on_or_off("NS3_MPI" "MPI_FOUND")

  string(APPEND out "Multithreaded Simulation      : ")
  check_on_or_off("${NS3_MTP}" "ON")

  string(APPEND out "ns-3 Click Integration        : ")
  check_on_or_off("ON" "NS3_CLICK")

  string(APPEND out "ns-3 OpenFlow Integration     : ")
  check_on_or_off("ON" "NS3_OPENFLOW")

  string(APPEND out "Netmap emulation FdNetDevice  : ")
  check_on_or_off("ENABLE_EMU" "ENABLE_NETMAP_EMU")

  string(APPEND out "PyViz visualizer              : ")
  check_on_or_off("NS3_VISUALIZER" "ENABLE_VISUALIZER")

  string(APPEND out "Python Bindings               : ")
  check_on_or_off("NS3_PYTHON_BINDINGS" "ENABLE_PYTHON_BINDINGS")

  string(APPEND out "SQLite support                : ")
  check_on_or_off("NS3_SQLITE" "ENABLE_SQLITE")

  string(APPEND out "Eigen3 support                : ")
  check_on_or_off("NS3_EIGEN" "ENABLE_EIGEN")

  string(APPEND out "Tap Bridge                    : ")
  check_on_or_off("ENABLE_TAP" "ENABLE_TAP")

  string(APPEND out "Tap FdNetDevice               : ")
  check_on_or_off("ENABLE_TAP" "ENABLE_TAPNETDEV")

  string(APPEND out "Tests                         : ")
  check_on_or_off("ENABLE_TESTS" "ENABLE_TESTS")

  string(APPEND out "\n\n")

  set(really-enabled-modules ${ns3-libs};${ns3-contrib-libs})
  if(${ENABLE_TESTS})
    list(APPEND really-enabled-modules libtest)
  endif()
  if(really-enabled-modules)
    print_formatted_table_with_modules(
      "Modules configured to be built" "${really-enabled-modules}" "out"
    )
    string(APPEND out "\n")
  endif()

  set(disabled-modules)
  foreach(module ${ns3-all-enabled-modules})
    if(NOT (lib${module} IN_LIST really-enabled-modules))
      list(APPEND disabled-modules ${module})
    endif()
  endforeach()

  if(disabled-modules)
    print_formatted_table_with_modules(
      "Modules that cannot be built" "${disabled-modules}" "out"
    )
    string(APPEND out "\n")
  endif()

  file(WRITE ${PROJECT_BINARY_DIR}/ns3config.txt ${out})
  message(STATUS ${out})

  if(NOT (${NS3RC} STREQUAL "NS3RC-NOTFOUND"))
    message(STATUS "Applying configuration override from: ${NS3RC}")
  endif()
endmacro()
