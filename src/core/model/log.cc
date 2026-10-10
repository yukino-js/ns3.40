#include "log.h"

#include "assert.h"
#include "environment-variable.h"
#include "fatal-error.h"
#include "string.h"

#include "ns3/core-config.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace {
const std::map<std::string, ns3::LogLevel> LOG_LABEL_LEVELS = {
    // clang-format off
        {"none",           ns3::LOG_NONE},
        {"error",          ns3::LOG_ERROR},
        {"level_error",    ns3::LOG_LEVEL_ERROR},
        {"warn",           ns3::LOG_WARN},
        {"level_warn",     ns3::LOG_LEVEL_WARN},
        {"debug",          ns3::LOG_DEBUG},
        {"level_debug",    ns3::LOG_LEVEL_DEBUG},
        {"info",           ns3::LOG_INFO},
        {"level_info",     ns3::LOG_LEVEL_INFO},
        {"function",       ns3::LOG_FUNCTION},
        {"level_function", ns3::LOG_LEVEL_FUNCTION},
        {"logic",          ns3::LOG_LOGIC},
        {"level_logic",    ns3::LOG_LEVEL_LOGIC},
        {"all",            ns3::LOG_ALL},
        {"level_all",      ns3::LOG_LEVEL_ALL},
        {"func",           ns3::LOG_PREFIX_FUNC},
        {"prefix_func",    ns3::LOG_PREFIX_FUNC},
        {"time",           ns3::LOG_PREFIX_TIME},
        {"prefix_time",    ns3::LOG_PREFIX_TIME},
        {"node",           ns3::LOG_PREFIX_NODE},
        {"prefix_node",    ns3::LOG_PREFIX_NODE},
        {"level",          ns3::LOG_PREFIX_LEVEL},
        {"prefix_level",   ns3::LOG_PREFIX_LEVEL},
        {"prefix_all",     ns3::LOG_PREFIX_ALL}
    // clang-format on
};

const std::map<ns3::LogLevel, std::string> LOG_LEVEL_LABELS = {[]() {
  std::map<ns3::LogLevel, std::string> labels;
  for (const auto &[label, lev] : LOG_LABEL_LEVELS) {
    if (labels.find(lev) == labels.end()) {
      std::string pad{label};
      if (pad.size() < 5) {
        pad.insert(pad.size(), 5 - pad.size(), ' ');
      }
      std::transform(pad.begin(), pad.end(), pad.begin(), ::toupper);
      labels[lev] = pad;
    }
  }
  return labels;
}()};

} // namespace

namespace ns3 {

static TimePrinter g_logTimePrinter = nullptr;
static NodePrinter g_logNodePrinter = nullptr;

class PrintList {
public:
  PrintList();
};

static PrintList g_printList;

LogComponent::ComponentList *LogComponent::GetComponentList() {
  static LogComponent::ComponentList components;
  return &components;
}

PrintList::PrintList() {
  auto [found, value] = EnvironmentVariable::Get("NS_LOG", "print-list", ":");
  if (found) {
    LogComponentPrintList();
    exit(0);
  }
}

LogComponent::LogComponent(const std::string &name, const std::string &file,
                           const LogLevel mask)
    : m_levels(0), m_mask(mask), m_name(name), m_file(file) {
  EnvVarCheck();

  LogComponent::ComponentList *components = GetComponentList();

  if (components->find(name) != components->end()) {
    NS_FATAL_ERROR("Log component \""
                   << name << "\" has already been registered once.");
  }

  components->insert(std::make_pair(name, this));
}

LogComponent &GetLogComponent(const std::string name) {
  LogComponent::ComponentList *components = LogComponent::GetComponentList();
  LogComponent *ret;

  try {
    ret = components->at(name);
  } catch (std::out_of_range &) {
    NS_FATAL_ERROR("Log component \"" << name << "\" does not exist.");
  }
  return *ret;
}

void LogComponent::EnvVarCheck() {
  auto [found, value] = EnvironmentVariable::Get("NS_LOG", m_name, ":");
  if (!found) {
    std::tie(found, value) = EnvironmentVariable::Get("NS_LOG", "*", ":");
  }
  if (!found) {
    std::tie(found, value) = EnvironmentVariable::Get("NS_LOG", "***", ":");
  }

  if (!found) {
    return;
  }

  if (value.empty()) {
    value = "**";
  }

  int level = 0;
  StringVector flags = SplitString(value, "|");
  NS_ASSERT_MSG(!flags.empty(), "Unexpected empty flags from non-empty value");
  bool pre_pipe{true};

  for (const auto &lev : flags) {
    if (lev == "**") {
      level |= LOG_LEVEL_ALL | LOG_PREFIX_ALL;
    } else if (lev == "all" || lev == "*") {
      level |= (pre_pipe ? LOG_LEVEL_ALL : LOG_PREFIX_ALL);
    } else if (LOG_LABEL_LEVELS.find(lev) != LOG_LABEL_LEVELS.end()) {
      level |= LOG_LABEL_LEVELS.at(lev);
    }
    pre_pipe = false;
  }
  Enable((LogLevel)level);
}

bool LogComponent::IsEnabled(const LogLevel level) const {
  return level & m_levels;
}

bool LogComponent::IsNoneEnabled() const { return m_levels == 0; }

void LogComponent::SetMask(const LogLevel level) { m_mask |= level; }

void LogComponent::Enable(const LogLevel level) {
  m_levels |= (level & ~m_mask);
}

void LogComponent::Disable(const LogLevel level) { m_levels &= ~level; }

std::string LogComponent::Name() const { return m_name; }

std::string LogComponent::File() const { return m_file; }

std::string LogComponent::GetLevelLabel(const LogLevel level) {
  auto it = LOG_LEVEL_LABELS.find(level);
  if (it != LOG_LEVEL_LABELS.end()) {
    return it->second;
  }
  return "unknown";
}

void LogComponentEnable(const std::string &name, LogLevel level) {
  LogComponent::ComponentList *components = LogComponent::GetComponentList();
  auto logComponent = components->find(name);

  if (logComponent == components->end()) {
    NS_LOG_UNCOND("Logging component \"" << name << "\" not found.");
    LogComponentPrintList();
    NS_FATAL_ERROR("Logging component \""
                   << name << "\" not found."
                   << " See above for a list of available log components");
  }

  logComponent->second->Enable(level);
}

void LogComponentEnableAll(LogLevel level) {
  LogComponent::ComponentList *components = LogComponent::GetComponentList();
  for (auto i = components->begin(); i != components->end(); i++) {
    i->second->Enable(level);
  }
}

void LogComponentDisable(const std::string &name, LogLevel level) {
  LogComponent::ComponentList *components = LogComponent::GetComponentList();
  auto logComponent = components->find(name);

  if (logComponent != components->end()) {
    logComponent->second->Disable(level);
  }
}

void LogComponentDisableAll(LogLevel level) {
  LogComponent::ComponentList *components = LogComponent::GetComponentList();
  for (auto i = components->begin(); i != components->end(); i++) {
    i->second->Disable(level);
  }
}

void LogComponentPrintList() {
  std::map<std::string, LogComponent *> componentsSorted;

  for (const auto &component : *LogComponent::GetComponentList()) {
    componentsSorted.insert(component);
  }

  for (const auto &[name, component] : componentsSorted) {
    std::cout << name << "=";
    if (component->IsNoneEnabled()) {
      std::cout << "0" << std::endl;
      continue;
    }
    if (component->IsEnabled(LOG_LEVEL_ALL)) {
      std::cout << "all";
    } else {
      if (component->IsEnabled(LOG_ERROR)) {
        std::cout << "error";
      }
      if (component->IsEnabled(LOG_WARN)) {
        std::cout << "|warn";
      }
      if (component->IsEnabled(LOG_DEBUG)) {
        std::cout << "|debug";
      }
      if (component->IsEnabled(LOG_INFO)) {
        std::cout << "|info";
      }
      if (component->IsEnabled(LOG_FUNCTION)) {
        std::cout << "|function";
      }
      if (component->IsEnabled(LOG_LOGIC)) {
        std::cout << "|logic";
      }
    }
    if (component->IsEnabled(LOG_PREFIX_ALL)) {
      std::cout << "|prefix_all";
    } else {
      if (component->IsEnabled(LOG_PREFIX_FUNC)) {
        std::cout << "|func";
      }
      if (component->IsEnabled(LOG_PREFIX_TIME)) {
        std::cout << "|time";
      }
      if (component->IsEnabled(LOG_PREFIX_NODE)) {
        std::cout << "|node";
      }
      if (component->IsEnabled(LOG_PREFIX_LEVEL)) {
        std::cout << "|level";
      }
    }
    std::cout << std::endl;
  }
}

static bool ComponentExists(std::string componentName) {
  LogComponent::ComponentList *components = LogComponent::GetComponentList();

  return components->find(componentName) != components->end();
}

static void CheckEnvironmentVariables() {
  auto dict = EnvironmentVariable::GetDictionary("NS_LOG", ":")->GetStore();

  for (auto &[component, value] : dict) {
    if (component != "*" && component != "***" && !ComponentExists(component)) {
      NS_LOG_UNCOND("Invalid or unregistered component name \"" << component
                                                                << "\"");
      LogComponentPrintList();
      NS_FATAL_ERROR("Invalid or unregistered component name \""
                     << component
                     << "\" in env variable NS_LOG, see above for a list of "
                        "valid components");
    }

    if (!value.empty()) {
      StringVector flags = SplitString(value, "|");
      for (const auto &flag : flags) {
        if (flag == "*" || flag == "**") {
          continue;
        }
        bool ok = LOG_LABEL_LEVELS.find(flag) != LOG_LABEL_LEVELS.end();
        if (!ok) {
          NS_FATAL_ERROR("Invalid log level \""
                         << flag
                         << "\" in env variable NS_LOG for component name "
                         << component);
        }
      }
    }
  }
}

void LogSetTimePrinter(TimePrinter printer) {
  g_logTimePrinter = printer;
  CheckEnvironmentVariables();
}

TimePrinter LogGetTimePrinter() { return g_logTimePrinter; }

void LogSetNodePrinter(NodePrinter printer) { g_logNodePrinter = printer; }

NodePrinter LogGetNodePrinter() { return g_logNodePrinter; }

ParameterLogger::ParameterLogger(std::ostream &os) : m_os(os) {}

void ParameterLogger::CommaRest() {
  if (m_first) {
    m_first = false;
  } else {
    m_os << ", ";
  }
}

template <>
ParameterLogger &
    ParameterLogger::operator<< <std::string>(const std::string &param) {
  CommaRest();
  m_os << "\"" << param << "\"";
  return *this;
}

ParameterLogger &ParameterLogger::operator<<(const char *param) {
  (*this) << std::string(param);
  return *this;
}

template <>
ParameterLogger &ParameterLogger::operator<< <int8_t>(const int8_t param) {
  (*this) << static_cast<int16_t>(param);
  return *this;
}

template <>
ParameterLogger &ParameterLogger::operator<< <uint8_t>(const uint8_t param) {
  (*this) << static_cast<uint16_t>(param);
  return *this;
}

} // namespace ns3
