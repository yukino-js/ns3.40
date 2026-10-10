
#ifndef NS3_LOG_H
#define NS3_LOG_H

#include "log-macros-disabled.h"
#include "log-macros-enabled.h"
#include "node-printer.h"
#include "time-printer.h"

#include <iostream>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ns3 {

enum LogLevel {
  LOG_NONE = 0x00000000,

  LOG_ERROR = 0x00000001,
  LOG_LEVEL_ERROR = 0x00000001,

  LOG_WARN = 0x00000002,
  LOG_LEVEL_WARN = 0x00000003,

  LOG_INFO = 0x00000004,
  LOG_LEVEL_INFO = 0x00000007,

  LOG_FUNCTION = 0x00000008,
  LOG_LEVEL_FUNCTION = 0x0000000f,

  LOG_LOGIC = 0x00000010,
  LOG_LEVEL_LOGIC = 0x0000001f,

  LOG_DEBUG = 0x00000020,
  LOG_LEVEL_DEBUG = 0x0000003f,

  LOG_ALL = 0x0fffffff,
  LOG_LEVEL_ALL = LOG_ALL,

  LOG_PREFIX_FUNC = 0x80000000,
  LOG_PREFIX_TIME = 0x40000000,
  LOG_PREFIX_NODE = 0x20000000,
  LOG_PREFIX_LEVEL = 0x10000000,
  LOG_PREFIX_ALL = 0xf0000000
};

void LogComponentEnable(const std::string &name, LogLevel level);

void LogComponentEnableAll(LogLevel level);

void LogComponentDisable(const std::string &name, LogLevel level);

void LogComponentDisableAll(LogLevel level);

} // namespace ns3

#define NS_LOG_COMPONENT_DEFINE(name)                                          \
  static ns3::LogComponent g_log = ns3::LogComponent(name, __FILE__)

#define NS_LOG_COMPONENT_DEFINE_MASK(name, mask)                               \
  static ns3::LogComponent g_log = ns3::LogComponent(name, __FILE__, mask)

#define NS_LOG_TEMPLATE_DECLARE LogComponent &g_log

#define NS_LOG_TEMPLATE_DEFINE(name) g_log(GetLogComponent(name))

#define NS_LOG_STATIC_TEMPLATE_DEFINE(name)                                    \
  static LogComponent &g_log [[maybe_unused]] = GetLogComponent(name)

#define NS_LOG_ERROR(msg) NS_LOG(ns3::LOG_ERROR, msg)

#define NS_LOG_WARN(msg) NS_LOG(ns3::LOG_WARN, msg)

#define NS_LOG_DEBUG(msg) NS_LOG(ns3::LOG_DEBUG, msg)

#define NS_LOG_INFO(msg) NS_LOG(ns3::LOG_INFO, msg)

#define NS_LOG_LOGIC(msg) NS_LOG(ns3::LOG_LOGIC, msg)

namespace ns3 {

void LogComponentPrintList();

void LogSetTimePrinter(TimePrinter lp);
TimePrinter LogGetTimePrinter();

void LogSetNodePrinter(NodePrinter np);
NodePrinter LogGetNodePrinter();

class LogComponent {
public:
  LogComponent(const std::string &name, const std::string &file,
               const LogLevel mask = LOG_NONE);
  bool IsEnabled(const LogLevel level) const;
  bool IsNoneEnabled() const;
  void Enable(const LogLevel level);
  void Disable(const LogLevel level);
  std::string Name() const;
  std::string File() const;
  static std::string GetLevelLabel(const LogLevel level);
  void SetMask(const LogLevel level);

  using ComponentList = std::unordered_map<std::string, LogComponent *>;

  static ComponentList *GetComponentList();

private:
  void EnvVarCheck();

  int32_t m_levels;
  int32_t m_mask;
  std::string m_name;
  std::string m_file;
};

LogComponent &GetLogComponent(const std::string name);

class ParameterLogger {
public:
  ParameterLogger(std::ostream &os);

  template <typename T, typename U = std::enable_if_t<std::is_arithmetic_v<T>>>
  ParameterLogger &operator<<(T param);

  template <typename T, typename U = std::enable_if_t<!std::is_arithmetic_v<T>>>
  ParameterLogger &operator<<(const T &param);

  template <typename T>
  ParameterLogger &operator<<(const std::vector<T> &vector);

  ParameterLogger &operator<<(const char *param);

private:
  void CommaRest();

  bool m_first{true};
  std::ostream &m_os;
};

template <typename T, typename U>
ParameterLogger &ParameterLogger::operator<<(T param) {
  CommaRest();
  m_os << param;
  return *this;
}

template <typename T, typename U>
ParameterLogger &ParameterLogger::operator<<(const T &param) {
  CommaRest();
  m_os << param;
  return *this;
}

template <typename T>
ParameterLogger &ParameterLogger::operator<<(const std::vector<T> &vector) {
  for (const auto &i : vector) {
    *this << i;
  }
  return *this;
}

template <>
ParameterLogger &
    ParameterLogger::operator<< <std::string>(const std::string &param);

template <>
ParameterLogger &ParameterLogger::operator<< <int8_t>(const int8_t param);

template <>
ParameterLogger &ParameterLogger::operator<< <uint8_t>(const uint8_t param);

} // namespace ns3

#endif
