#include "system-path.h"

#include "assert.h"
#include "environment-variable.h"
#include "fatal-error.h"
#include "log.h"
#include "string.h"

#include <algorithm>
#include <ctime>
#include <regex>
#include <sstream>
#include <tuple>

#ifdef __has_include
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "No support for filesystem library"
#endif
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#ifdef __linux__
#include <cstring>
#include <unistd.h>
#endif

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <regex>
#include <windows.h>
#endif

#if defined(__WIN32__)
constexpr auto SYSTEM_PATH_SEP = "\\";
#else
constexpr auto SYSTEM_PATH_SEP = "/";
#endif

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SystemPath");

namespace {
std::tuple<std::list<std::string>, bool> ReadFilesNoThrow(std::string path) {
  NS_LOG_FUNCTION(path);
  std::list<std::string> files;
  if (!fs::exists(path)) {
    return std::make_tuple(files, true);
  }
  for (auto &it : fs::directory_iterator(path)) {
    if (!fs::is_directory(it.path())) {
      files.push_back(it.path().filename().string());
    }
  }
  return std::make_tuple(files, false);
}

} // namespace

namespace SystemPath {

std::string Dirname(std::string path) {
  NS_LOG_FUNCTION(path);
  std::list<std::string> elements = Split(path);
  auto last = elements.end();
  last--;
  return Join(elements.begin(), last);
}

std::string FindSelfDirectory() {
  NS_LOG_FUNCTION_NOARGS();
  std::string filename;
#if defined(__linux__)
  {
    ssize_t size = 1024;
    char *buffer = (char *)malloc(size);
    memset(buffer, 0, size);
    int status;
    while (true) {
      status = readlink("/proc/self/exe", buffer, size);
      if (status != 1 || (status == -1 && errno != ENAMETOOLONG)) {
        break;
      }
      size *= 2;
      free(buffer);
      buffer = (char *)malloc(size);
      memset(buffer, 0, size);
    }
    if (status == -1) {
      NS_FATAL_ERROR("Oops, could not find self directory.");
    }
    filename = buffer;
    free(buffer);
  }
#elif defined(__WIN32__)
  {
    DWORD size = 1024;
    LPTSTR lpFilename = (LPTSTR)malloc(sizeof(TCHAR) * size);
    DWORD status = GetModuleFileName(nullptr, lpFilename, size);
    while (status == size) {
      size = size * 2;
      free(lpFilename);
      lpFilename = (LPTSTR)malloc(sizeof(TCHAR) * size);
      status = GetModuleFileName(nullptr, lpFilename, size);
    }
    NS_ASSERT(status != 0);
    filename = lpFilename;
    free(lpFilename);
  }
#elif defined(__APPLE__)
  {
    uint32_t bufsize = 1024;
    char *buffer = (char *)malloc(bufsize);
    NS_ASSERT(buffer);
    int status = _NSGetExecutablePath(buffer, &bufsize);
    if (status == -1) {
      free(buffer);
      buffer = (char *)malloc(bufsize);
      status = _NSGetExecutablePath(buffer, &bufsize);
    }
    NS_ASSERT(status == 0);
    filename = buffer;
    free(buffer);
  }
#elif defined(__FreeBSD__)
  {
    int mib[4];
    std::size_t bufSize = 1024;
    char *buf = (char *)malloc(bufSize);

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;

    sysctl(mib, 4, buf, &bufSize, nullptr, 0);
    filename = buf;
  }
#endif
  return Dirname(filename);
}

std::string Append(std::string left, std::string right) {
  NS_LOG_FUNCTION(left << right);
  while (true) {
    std::string::size_type lastSep = left.rfind(SYSTEM_PATH_SEP);
    if (lastSep != left.size() - 1) {
      break;
    }
    left = left.substr(0, left.size() - 1);
  }
  std::string retval = left + SYSTEM_PATH_SEP + right;
  return retval;
}

std::list<std::string> Split(std::string path) {
  NS_LOG_FUNCTION(path);
  std::vector<std::string> items = SplitString(path, SYSTEM_PATH_SEP);
  std::list<std::string> retval(items.begin(), items.end());
  return retval;
}

std::string Join(std::list<std::string>::const_iterator begin,
                 std::list<std::string>::const_iterator end) {
  NS_LOG_FUNCTION(*begin << *end);
  std::string retval = "";
  for (auto i = begin; i != end; i++) {
    if ((*i).empty()) {
      continue;
    } else if (i == begin) {
      retval = *i;
    } else {
      retval = retval + SYSTEM_PATH_SEP + *i;
    }
  }
  return retval;
}

std::list<std::string> ReadFiles(std::string path) {
  NS_LOG_FUNCTION(path);
  bool err;
  std::list<std::string> files;
  std::tie(files, err) = ReadFilesNoThrow(path);
  if (err) {
    NS_FATAL_ERROR("Could not open directory=" << path);
  }
  return files;
}

std::string MakeTemporaryDirectoryName() {
  NS_LOG_FUNCTION_NOARGS();
  auto [found, path] = EnvironmentVariable::Get("TMP");
  if (!found) {
    std::tie(found, path) = EnvironmentVariable::Get("TEMP");
    if (!found) {
      path = "/tmp";
    }
  }

  time_t now = time(nullptr);
  struct tm *tm_now = localtime(&now);
  srand(time(nullptr));
  long int n = rand();

  std::ostringstream oss;
  oss << path << SYSTEM_PATH_SEP << "ns-3." << tm_now->tm_hour << "."
      << tm_now->tm_min << "." << tm_now->tm_sec << "." << n;

  return oss.str();
}

void MakeDirectories(std::string path) {
  NS_LOG_FUNCTION(path);

  std::error_code ec;
  if (!fs::exists(path)) {
    fs::create_directories(path, ec);
  }

  if (ec.value()) {
    NS_FATAL_ERROR("failed creating directory " << path);
  }
}

bool Exists(const std::string path) {
  NS_LOG_FUNCTION(path);

  bool err;
  auto dirpath = Dirname(path);
  std::list<std::string> files;
  tie(files, err) = ReadFilesNoThrow(dirpath);
  if (err) {
    NS_LOG_LOGIC("directory doesn't exist: " << dirpath);
    return false;
  }
  NS_LOG_LOGIC("directory exists: " << dirpath);

  auto tokens = Split(path);
  std::string file = tokens.back();

  if (file.empty()) {
    NS_LOG_LOGIC("directory path exists: " << path);
    return true;
  }

  files = ReadFiles(dirpath);

  auto it = std::find(files.begin(), files.end(), file);
  if (it == files.end()) {
    NS_LOG_LOGIC("file itself doesn't exist: " << file);
    return false;
  }

  NS_LOG_LOGIC("file itself exists: " << file);
  return true;
}

std::string CreateValidSystemPath(const std::string path) {
  std::regex incompatible_characters(" |:[^\\\\]|<|>|\\*");
  std::string valid_path;
  std::regex_replace(std::back_inserter(valid_path), path.begin(), path.end(),
                     incompatible_characters, "_");
  return valid_path;
}

} // namespace SystemPath

} // namespace ns3
