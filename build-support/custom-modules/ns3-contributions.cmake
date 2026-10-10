
macro(process_contribution contribution_list)
  foreach(libname ${contribution_list})
    library_target_name(${libname} targetname)
    set(lib${libname} ${targetname} CACHE INTERNAL "")
    set(lib${libname}-obj ${targetname}-obj CACHE INTERNAL "")
  endforeach()

  foreach(contribname ${contribution_list})
    set(folder "contrib/${contribname}")
    if(EXISTS ${PROJECT_SOURCE_DIR}/${folder}/CMakeLists.txt)
      message(STATUS "Processing ${folder}")
      add_subdirectory(${folder})
    else()
      message(${HIGHLIGHTED_STATUS}
              "Skipping ${folder} : it does not contain a CMakeLists.txt file"
      )
    endif()
  endforeach()
endmacro()
