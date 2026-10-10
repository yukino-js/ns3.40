



function(_GTK3_GET_VERSION _OUT_major _OUT_minor _OUT_micro _gtkversion_hdr)
  file(READ ${_gtkversion_hdr} _contents)
  if(_contents)
    string(REGEX REPLACE ".*#define GTK_MAJOR_VERSION[ \t]+\\(([0-9]+)\\).*"
                         "\\1" ${_OUT_major} "${_contents}"
    )

    string(REGEX REPLACE ".*#define GTK_MINOR_VERSION[ \t]+\\(([0-9]+)\\).*"
                         "\\1" ${_OUT_minor} "${_contents}"
    )

    string(REGEX REPLACE ".*#define GTK_MICRO_VERSION[ \t]+\\(([0-9]+)\\).*"
                         "\\1" ${_OUT_micro} "${_contents}"
    )
    if(NOT ${_OUT_major} MATCHES "[0-9]+")
      message(FATAL_ERROR "Version parsing failed for GTK3_MAJOR_VERSION!")
    endif()
    if(NOT ${_OUT_minor} MATCHES "[0-9]+")
      message(FATAL_ERROR "Version parsing failed for GTK3_MINOR_VERSION!")
    endif()
    if(NOT ${_OUT_micro} MATCHES "[0-9]+")
      message(FATAL_ERROR "Version parsing failed for GTK3_MICRO_VERSION!")
    endif()

    set(${_OUT_major} ${${_OUT_major}} PARENT_SCOPE)
    set(${_OUT_minor} ${${_OUT_minor}} PARENT_SCOPE)
    set(${_OUT_micro} ${${_OUT_micro}} PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Include file ${_gtkversion_hdr} does not exist")
  endif()
endfunction()

function(_GTK3_FIND_INCLUDE_DIR _var _hdr)

  if(GTK3_DEBUG)
    message(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}] "
                   "_GTK3_FIND_INCLUDE_DIR( ${_var} ${_hdr} )"
    )
  endif()

  set(_relatives
      ${GTK3_ADDITIONAL_SUFFIXES}
      glibmm-2.0
      glib-2.0
      atk-1.0
      atkmm-1.0
      cairo
      cairomm-1.0
      gdk-pixbuf-2.0
      gdkmm-2.4
      giomm-2.4
      gtk-3.0
      gtkmm-2.4
      libglade-2.0
      libglademm-2.4
      harfbuzz
      pango-1.0
      pangomm-1.4
      sigc++-2.2
      gtk-unix-print-2.0
  )

  set(_suffixes)
  foreach(_d ${_relatives})
    list(APPEND _suffixes ${_d})
    list(APPEND _suffixes ${_d}/include)
  endforeach()

  if(GTK3_DEBUG)
    message(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
                   "include suffixes = ${_suffixes}"
    )
  endif()

  find_path(
    ${_var} ${_hdr}
    PATHS
          C:/GTK/lib/glib-2.0/include
          C:/msys64/$ENV{MSYSTEM}/lib/glib-2.0
          /usr/local/lib64
          /usr/local/lib
          /usr/lib/i386-linux-gnu/
          /usr/lib/x86_64-linux-gnu/
          /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
          /usr/lib64
          /usr
          /opt/gnome
          /usr/openwin
          /sw
          /opt/local
          /opt/local/lib
          $ENV{GTKMM_BASEPATH}
          [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
          [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
    PATH_SUFFIXES ${_suffixes} include lib
  )

  if(${_var})
    set(GTK3_INCLUDE_DIRS ${GTK3_INCLUDE_DIRS} ${${_var}} PARENT_SCOPE)
    if(NOT GTK3_SKIP_MARK_AS_ADVANCED)
      mark_as_advanced(${_var})
    endif()
  endif()

endfunction()

function(_GTK3_FIND_LIBRARY _var _lib _expand_vc _append_version)

  if(GTK3_DEBUG)
    message(
      STATUS
        "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}] "
        "_GTK3_FIND_LIBRARY( ${_var} ${_lib} ${_expand_vc} ${_append_version} )"
    )
  endif()

  set(_versions
      3.0
      3
      2.20
      2.18
      2.16
      2.14
      2.12
      2.10
      2.8
      2.6
      2.4
      2.2
      2.0
      1.20
      1.18
      1.16
      1.14
      1.12
      1.10
      1.8
      1.6
      1.4
      1.2
      1.0
  )

  set(_library)
  set(_library_d)

  set(_library ${_lib})

  if(_expand_vc AND MSVC)
    if(MSVC80)
      set(_library ${_library}-vc80)
    elseif(MSVC90)
      set(_library ${_library}-vc90)
    elseif(MSVC10)
      set(_library ${_library}-vc100)
    endif()
    set(_library_d ${_library}-d)
  endif()

  if(GTK3_DEBUG)
    message(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
                   "After midfix addition = ${_library} and ${_library_d}"
    )
  endif()

  set(_lib_list)
  set(_libd_list)
  if(_append_version)
    foreach(_ver ${_versions})
      list(APPEND _lib_list "${_library}-${_ver}")
      list(APPEND _libd_list "${_library_d}-${_ver}")
    endforeach()
  else()
    set(_lib_list ${_library})
    set(_libd_list ${_library_d})
  endif()

  if(GTK3_DEBUG)
    message(
      STATUS
        "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
        "library list = ${_lib_list} and library debug list = ${_libd_list}"
    )
  endif()

  if(_expand_vc AND MSVC)
    set(_no_dots_lib_list)
    set(_no_dots_libd_list)
    foreach(_l ${_lib_list})
      string(REPLACE "." "_" _no_dots_library ${_l})
      list(APPEND _no_dots_lib_list ${_no_dots_library})
    endforeach()
    set(_no_dots_libsd_list)
    foreach(_l ${_libd_list})
      string(REPLACE "." "_" _no_dots_libraryd ${_l})
      list(APPEND _no_dots_libd_list ${_no_dots_libraryd})
    endforeach()
    set(_lib_list ${_no_dots_lib_list})
    set(_libd_list ${_no_dots_libd_list})
  endif()

  if(GTK3_DEBUG)
    message(
      STATUS
        "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
        "While searching for ${_var}, our proposed library list is ${_lib_list}"
    )
  endif()

  find_library(
    ${_var}
    NAMES ${_lib_list}
    PATHS /opt/gnome /usr/openwin /sw $ENV{GTKMM_BASEPATH}
          [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
          [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
    PATH_SUFFIXES lib lib64
  )

  if(_expand_vc AND MSVC)
    if(GTK3_DEBUG)
      message(
        STATUS
          "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
          "While searching for ${_var}_DEBUG our proposed library list is ${_libd_list}"
      )
    endif()

    find_library(
      ${_var}_DEBUG
      NAMES ${_libd_list}
      PATHS $ENV{GTKMM_BASEPATH} [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
      PATH_SUFFIXES lib
    )

    if(${_var} AND ${_var}_DEBUG)
      if(NOT GTK3_SKIP_MARK_AS_ADVANCED)
        mark_as_advanced(${_var}_DEBUG)
      endif()
      set(GTK3_LIBRARIES ${GTK3_LIBRARIES} optimized ${${_var}} debug
                         ${${_var}_DEBUG}
      )
      set(GTK3_LIBRARIES ${GTK3_LIBRARIES} PARENT_SCOPE)
    endif()
  else()
    if(NOT GTK3_SKIP_MARK_AS_ADVANCED)
      mark_as_advanced(${_var})
    endif()
    set(GTK3_LIBRARIES ${GTK3_LIBRARIES} ${${_var}})
    set(GTK3_LIBRARIES ${GTK3_LIBRARIES} PARENT_SCOPE)
    set(${_var}_DEBUG ${${_var}})
    set(${_var}_DEBUG ${${_var}} PARENT_SCOPE)
  endif()
endfunction()



set(GTK3_FOUND)
set(GTK3_INCLUDE_DIRS)
set(GTK3_LIBRARIES)

if(NOT GTK3_FIND_COMPONENTS)
  set(GTK3_FIND_COMPONENTS gtk)
endif()

if(GTK3_FIND_VERSION)
  if(NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 2.6.2)
  endif()
  set(GTK3_FAILED_VERSION_CHECK true)
  if(GTK3_DEBUG)
    message(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}] "
                   "Searching for version ${GTK3_FIND_VERSION}"
    )
  endif()
  _gtk3_find_include_dir(GTK3_GTK_INCLUDE_DIR gtk/gtk.h)
  if(GTK3_GTK_INCLUDE_DIR)
    _gtk3_get_version(
      GTK3_MAJOR_VERSION GTK3_MINOR_VERSION GTK3_PATCH_VERSION
      ${GTK3_GTK_INCLUDE_DIR}/gtk/gtkversion.h
    )
    set(GTK3_VERSION
        ${GTK3_MAJOR_VERSION}.${GTK3_MINOR_VERSION}.${GTK3_PATCH_VERSION}
    )
    if(GTK3_FIND_VERSION_EXACT)
      if(GTK3_VERSION VERSION_EQUAL GTK3_FIND_VERSION)
        set(GTK3_FAILED_VERSION_CHECK false)
      endif()
    else()
      if(GTK3_VERSION VERSION_EQUAL GTK3_FIND_VERSION
         OR GTK3_VERSION VERSION_GREATER GTK3_FIND_VERSION
      )
        set(GTK3_FAILED_VERSION_CHECK false)
      endif()
    endif()
  else()
    if(GTK3_FIND_REQUIRED AND NOT GTK3_FIND_QUIETLY)
      message(FATAL_ERROR "Could not find GTK3 include directory")
    endif()
    return()
  endif()

  if(GTK3_FAILED_VERSION_CHECK)
    if(GTK3_FIND_REQUIRED AND NOT GTK3_FIND_QUIETLY)
      if(GTK3_FIND_VERSION_EXACT)
        message(
          FATAL_ERROR
            "GTK3 version check failed.
Version ${GTK3_VERSION} was found, \
version ${GTK3_FIND_VERSION} is needed exactly."
        )
      else()
        message(
          FATAL_ERROR
            "GTK3 version check failed.
Version ${GTK3_VERSION} was found, \
at least version ${GTK3_FIND_VERSION} is required"
        )
      endif()
    endif()

    return()
  endif()
endif()


find_package(Freetype QUIET)
list(APPEND GTK3_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
list(APPEND GTK3_LIBRARIES ${FREETYPE_LIBRARIES})

foreach(_GTK3_component ${GTK3_FIND_COMPONENTS})
  if(_GTK3_component STREQUAL "gtk")
    _gtk3_find_include_dir(GTK3_GLIB_INCLUDE_DIR glib.h)
    _gtk3_find_include_dir(GTK3_GLIBCONFIG_INCLUDE_DIR glibconfig.h)
    _gtk3_find_library(GTK3_GLIB_LIBRARY glib false true)

    _gtk3_find_library(GTK3_GTHREAD_LIBRARY gthread false true)

    _gtk3_find_include_dir(GTK3_GOBJECT_INCLUDE_DIR gobject/gobject.h)
    _gtk3_find_library(GTK3_GOBJECT_LIBRARY gobject false true)

    _gtk3_find_include_dir(GTK3_GIO_INCLUDE_DIR gio/gio.h)
    _gtk3_find_library(GTK3_GIO_LIBRARY gio false true)

    _gtk3_find_include_dir(GTK3_GDK_PIXBUF_INCLUDE_DIR gdk-pixbuf/gdk-pixbuf.h)
    _gtk3_find_library(GTK3_GDK_PIXBUF_LIBRARY gdk_pixbuf false true)

    _gtk3_find_include_dir(GTK3_GDK_INCLUDE_DIR gdk/gdk.h)
    _gtk3_find_include_dir(GTK3_GDKCONFIG_INCLUDE_DIR gdk/gdkconfig.h)
    _gtk3_find_include_dir(GTK3_GTK_INCLUDE_DIR gtk/gtk.h)

    if(UNIX)
      _gtk3_find_library(GTK3_GDK_LIBRARY gdk false true)
      _gtk3_find_library(GTK3_GTK_LIBRARY gtk false true)
    else()
      _gtk3_find_library(GTK3_GDK_LIBRARY gdk false true)
      _gtk3_find_library(GTK3_GTK_LIBRARY gtk false true)
      _gtk3_find_library(GTK3_GDK_LIBRARY gdk-win32 false true)
      _gtk3_find_library(GTK3_GTK_LIBRARY gtk-win32 false true)
    endif()

    _gtk3_find_include_dir(GTK3_CAIRO_INCLUDE_DIR cairo.h)
    _gtk3_find_library(GTK3_CAIRO_LIBRARY cairo false false)

    _gtk3_find_include_dir(GTK3_FONTCONFIG_INCLUDE_DIR fontconfig/fontconfig.h)

    _gtk3_find_include_dir(GTK3_PANGO_INCLUDE_DIR pango/pango.h)
    _gtk3_find_library(GTK3_PANGO_LIBRARY pango false true)

    _gtk3_find_include_dir(GTK3_HARFBUZZ_INCLUDE_DIR hb.h)

    _gtk3_find_include_dir(GTK3_ATK_INCLUDE_DIR atk/atk.h)
    _gtk3_find_library(GTK3_ATK_LIBRARY atk false true)

  elseif(_GTK3_component STREQUAL "gtkmm")

    _gtk3_find_include_dir(GTK3_GLIBMM_INCLUDE_DIR glibmm.h)
    _gtk3_find_include_dir(GTK3_GLIBMMCONFIG_INCLUDE_DIR glibmmconfig.h)
    _gtk3_find_library(GTK3_GLIBMM_LIBRARY glibmm true true)

    _gtk3_find_include_dir(GTK3_GDKMM_INCLUDE_DIR gdkmm.h)
    _gtk3_find_include_dir(GTK3_GDKMMCONFIG_INCLUDE_DIR gdkmmconfig.h)
    _gtk3_find_library(GTK3_GDKMM_LIBRARY gdkmm true true)

    _gtk3_find_include_dir(GTK3_GTKMM_INCLUDE_DIR gtkmm.h)
    _gtk3_find_include_dir(GTK3_GTKMMCONFIG_INCLUDE_DIR gtkmmconfig.h)
    _gtk3_find_library(GTK3_GTKMM_LIBRARY gtkmm true true)

    _gtk3_find_include_dir(GTK3_CAIROMM_INCLUDE_DIR cairomm/cairomm.h)
    _gtk3_find_library(GTK3_CAIROMM_LIBRARY cairomm true true)

    _gtk3_find_include_dir(GTK3_PANGOMM_INCLUDE_DIR pangomm.h)
    _gtk3_find_include_dir(GTK3_PANGOMMCONFIG_INCLUDE_DIR pangommconfig.h)
    _gtk3_find_library(GTK3_PANGOMM_LIBRARY pangomm true true)

    _gtk3_find_include_dir(GTK3_SIGC++_INCLUDE_DIR sigc++/sigc++.h)
    _gtk3_find_include_dir(GTK3_SIGC++CONFIG_INCLUDE_DIR sigc++config.h)
    _gtk3_find_library(GTK3_SIGC++_LIBRARY sigc true true)

    _gtk3_find_include_dir(GTK3_GIOMM_INCLUDE_DIR giomm.h)
    _gtk3_find_include_dir(GTK3_GIOMMCONFIG_INCLUDE_DIR giommconfig.h)
    _gtk3_find_library(GTK3_GIOMM_LIBRARY giomm true true)

    _gtk3_find_include_dir(GTK3_ATKMM_INCLUDE_DIR atkmm.h)
    _gtk3_find_library(GTK3_ATKMM_LIBRARY atkmm true true)

  elseif(_GTK3_component STREQUAL "glade")

    _gtk3_find_include_dir(GTK3_GLADE_INCLUDE_DIR glade/glade.h)
    _gtk3_find_library(GTK3_GLADE_LIBRARY glade false true)

  elseif(_GTK3_component STREQUAL "glademm")

    _gtk3_find_include_dir(GTK3_GLADEMM_INCLUDE_DIR libglademm.h)
    _gtk3_find_include_dir(GTK3_GLADEMMCONFIG_INCLUDE_DIR libglademmconfig.h)
    _gtk3_find_library(GTK3_GLADEMM_LIBRARY glademm true true)

  else()
    message(FATAL_ERROR "Unknown GTK3 component ${_component}")
  endif()
endforeach()

if(NOT GTK3_FIND_VERSION AND GTK3_GTK_INCLUDE_DIR)
  _gtk3_get_version(
    GTK3_MAJOR_VERSION GTK3_MINOR_VERSION GTK3_PATCH_VERSION
    ${GTK3_GTK_INCLUDE_DIR}/gtk/gtkversion.h
  )
  set(GTK3_VERSION
      ${GTK3_MAJOR_VERSION}.${GTK3_MINOR_VERSION}.${GTK3_PATCH_VERSION}
  )

endif()


set(_GTK3_did_we_find_everything true)

include(FindPackageHandleStandardArgs)

foreach(_GTK3_component ${GTK3_FIND_COMPONENTS})
  string(TOUPPER ${_GTK3_component} _COMPONENT_UPPER)

  if(_GTK3_component STREQUAL "gtk")
    find_package_handle_standard_args(
      GTK3_${_COMPONENT_UPPER}
      "Some or all of the gtk libraries were not found."
      GTK3_GTK_LIBRARY
      GTK3_GTK_INCLUDE_DIR
      GTK3_GLIB_INCLUDE_DIR
      GTK3_GLIBCONFIG_INCLUDE_DIR
      GTK3_GLIB_LIBRARY
      GTK3_GTHREAD_LIBRARY
      GTK3_GDK_INCLUDE_DIR
      GTK3_GDKCONFIG_INCLUDE_DIR
      GTK3_GDK_LIBRARY
    )
  elseif(_GTK3_component STREQUAL "gtkmm")
    find_package_handle_standard_args(
      GTK3_${_COMPONENT_UPPER}
      "Some or all of the gtkmm libraries were not found."
      GTK3_GTKMM_LIBRARY
      GTK3_GTKMM_INCLUDE_DIR
      GTK3_GTKMMCONFIG_INCLUDE_DIR
      GTK3_GLIBMM_INCLUDE_DIR
      GTK3_GLIBMMCONFIG_INCLUDE_DIR
      GTK3_GLIBMM_LIBRARY
      GTK3_GDKMM_INCLUDE_DIR
      GTK3_GDKMMCONFIG_INCLUDE_DIR
      GTK3_GDKMM_LIBRARY
    )
  elseif(_GTK3_component STREQUAL "glade")
    find_package_handle_standard_args(
      GTK3_${_COMPONENT_UPPER} "The glade library was not found."
      GTK3_GLADE_LIBRARY GTK3_GLADE_INCLUDE_DIR
    )
  elseif(_GTK3_component STREQUAL "glademm")
    find_package_handle_standard_args(
      GTK3_${_COMPONENT_UPPER} "The glademm library was not found."
      GTK3_GLADEMM_LIBRARY GTK3_GLADEMM_INCLUDE_DIR
      GTK3_GLADEMMCONFIG_INCLUDE_DIR
    )
  endif()

  if(NOT GTK3_${_COMPONENT_UPPER}_FOUND)
    set(_GTK3_did_we_find_everything false)
  endif()
endforeach()

if(_GTK3_did_we_find_everything AND NOT GTK3_VERSION_CHECK_FAILED)
  set(GTK3_FOUND true)
else()
  set(GTK3_FOUND false)
  set(GTK3_VERSION)
  set(GTK3_VERSION_MAJOR)
  set(GTK3_VERSION_MINOR)
  set(GTK3_VERSION_PATCH)
  set(GTK3_INCLUDE_DIRS)
  set(GTK3_LIBRARIES)
endif()

if(GTK3_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES GTK3_INCLUDE_DIRS)
endif()
