
include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
  "
    #include <iostream>
    #include <cstddef>
    inline std::ostream& operator << (std::ostream& os, std::nullptr_t ptr)
    {
      return os << \"nullptr\"; //whatever you want nullptr to show up as in the console
    }
    int main()
    {
        std::ostream os(NULL);
        os << std::nullptr_t();
        return 0;
    }
    "
  MISSING_OSTREAM_NULLPTR_OPERATOR
)

if(${MISSING_OSTREAM_NULLPTR_OPERATOR})
  message(
    ${HIGHLIGHTED_STATUS}
    "Using compiler workaround: compiling in \"ostream& operator<<(ostream&, nullptr_t)\""
  )
  add_definitions(
    -include
    ${CMAKE_CURRENT_SOURCE_DIR}/build-support/compiler-workarounds/ostream-operator-nullptr.h
  )
endif()

check_cxx_source_compiles(
  "
  #ifdef __has_include
    #if __has_include(<filesystem>)
      #include <filesystem>
      namespace fs = std::filesystem;
    #elif __has_include(<experimental/filesystem>)
      #include <experimental/filesystem>
      namespace fs = std::experimental::filesystem;
    #else
      #error \"No support for filesystem library\"
    #endif
  #endif
  int main()
  {
    std::string path = \"/\";
    return !fs::exists(path);
  }
  "
  FILESYSTEM_LIBRARY_IS_LINKED
)
