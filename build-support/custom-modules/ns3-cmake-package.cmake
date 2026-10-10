
function(build_required_and_libs_lists module_name visibility libraries
         all_ns3_libraries
)
  set(linked_libs_list)
  set(required_modules_list)
  foreach(lib ${libraries})
    if(${lib} IN_LIST all_ns3_libraries)
      get_target_property(lib_real_name ${lib} OUTPUT_NAME)
      remove_lib_prefix(${lib} required_module_name)
      set(required_modules_list
          "${required_modules_list} ns3-${required_module_name}"
      )
      set(lib_real_name "-l${lib_real_name}")
    else()
      if((IS_ABSOLUTE "${lib}") OR ("${lib}" MATCHES "^-l"))
        set(lib_real_name ${lib})
      else()
        set(lib_real_name "-l${lib}")
      endif()
    endif()
    set(linked_libs_list "${linked_libs_list} ${lib_real_name}")
  endforeach()
  set(pkgconfig_${visibility}_libs ${linked_libs_list} PARENT_SCOPE)
  set(pkgconfig_${visibility}_required ${required_modules_list} PARENT_SCOPE)
endfunction()

function(pkgconfig_module libname)
  get_target_property(all_libs ${libname} LINK_LIBRARIES)

  get_target_property(interface_libs ${libname} INTERFACE_LINK_LIBRARIES)

  string(REPLACE "${LIB_AS_NEEDED_PRE}" "" all_libs "${all_libs}")
  string(REPLACE "${LIB_AS_NEEDED_POST}" "" all_libs "${all_libs}")
  string(REPLACE "${LIB_AS_NEEDED_PRE}" "" interface_libs "${interface_libs}")
  string(REPLACE "${LIB_AS_NEEDED_POST}" "" interface_libs "${interface_libs}")

  foreach(interface_lib ${interface_libs})
    list(REMOVE_ITEM all_libs ${interface_lib})
  endforeach()
  set(private_libs ${all_libs})

  remove_lib_prefix(${libname} module_name)

  get_target_property(pkgconfig_target_lib ${libname} OUTPUT_NAME)

  build_required_and_libs_lists(
    "${module_name}" public "${interface_libs}"
    "${ns3-libs};${ns3-contrib-libs}"
  )

  build_required_and_libs_lists(
    "${module_name}" private "${private_libs}"
    "${ns3-libs};${ns3-contrib-libs}"
  )

  set(pkgconfig_interface_include_directories)
  if(${NS3_REEXPORT_THIRD_PARTY_LIBRARIES})
    get_target_includes(${libname} pkgconfig_interface_include_directories)
    string(REPLACE "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/include" ""
                   pkgconfig_interface_include_directories
                   "${pkgconfig_interface_include_directories}"
    )
    string(REPLACE ";-I" " -I" pkgconfig_interface_include_directories
                   "${pkgconfig_interface_include_directories}"
    )
    string(REPLACE ";" "" pkgconfig_interface_include_directories
                   "${pkgconfig_interface_include_directories}"
    )
  endif()

  set(pkgconfig_file ${CMAKE_BINARY_DIR}/pkgconfig/ns3-${module_name}.pc)
  configure_file(
    ${PROJECT_SOURCE_DIR}/build-support/pkgconfig-template.pc.in
    ${pkgconfig_file} @ONLY
  )

  install(FILES ${pkgconfig_file} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  add_custom_target(
    uninstall_pkgconfig_${module_name}
    COMMAND rm ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig/ns3-${module_name}.pc
  )
  add_dependencies(uninstall uninstall_pkgconfig_${module_name})
endfunction()

function(ns3_cmake_package)
  set(enabled_modules "${ns3-libs};${ns3-contrib-libs}")
  if(enabled_modules STREQUAL ";")
    message(
      STATUS
        "No modules were configured, so we cannot create installation artifacts"
    )
    return()
  endif()

  string(REPLACE "-" "." ns3_version "${NS3_VER}")

  foreach(library ${ns3-libs}${ns3-contrib-libs})
    pkgconfig_module(${library})
  endforeach()

  install(
    EXPORT ns3ExportTargets
    NAMESPACE ns3::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ns3
    FILE ns3Targets.cmake
  )

  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    "build-support/Config.cmake.in" "ns3Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ns3
    PATH_VARS CMAKE_INSTALL_LIBDIR
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ns3ConfigVersion.cmake VERSION ${ns3_version}
    COMPATIBILITY ExactVersion
  )

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ns3Config.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/ns3ConfigVersion.cmake"
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ns3
  )
endfunction()

if(WIN32)
  add_custom_target(
    uninstall
    COMMAND
      powershell -Command \" Remove-Item \\"${CMAKE_INSTALL_FULL_LIBDIR}/libns3*\\" -Recurse \" &&
      powershell -Command \" Remove-Item \\"${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig/ns3-*\\" -Recurse \" &&
      powershell -Command \" Remove-Item \\"${CMAKE_INSTALL_FULL_LIBDIR}/cmake/ns3\\" -Recurse \" &&
      powershell -Command \" Remove-Item \\"${CMAKE_INSTALL_FULL_INCLUDEDIR}/ns3\\" -Recurse \"
  )
else()
add_custom_target(
  uninstall
  COMMAND
    rm `ls ${CMAKE_INSTALL_FULL_LIBDIR}/libns3*` &&
    rm `ls ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig/ns3-*` &&
    rm -R ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/ns3 &&
    rm -R ${CMAKE_INSTALL_FULL_INCLUDEDIR}/ns3
)
endif()
