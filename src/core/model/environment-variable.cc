
#include "environment-variable.h"

#include "string.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdlib.h>

#ifdef __WIN32__
#include <cerrno>

int setenv(const char *var_name, const char *new_value, int change_flag) {
  std::string variable{var_name};
  std::string value{new_value};

  if (variable.empty() || value.empty()) {
    errno = EINVAL;
    return -1;
  }

  if (variable.find('=') != std::string::npos) {
    errno = EINVAL;
    return -1;
  }

  if (change_flag == 0) {
    char *old_value = std::getenv(var_name);
    if (old_value != nullptr) {
      return 0;
    }
  }

  return _putenv_s(var_name, new_value);
}

int unsetenv(const char *var_name) { return _putenv_s(var_name, ""); }

#endif

namespace ns3 {

#if 0
#define NS_LOCAL_LOG(msg)                                                      \
  std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__              \
            << "(): " << msg << std::endl

#define NS_LOCAL_ASSERT(cond, msg)                                             \
  do {                                                                         \
    if (!(cond)) {                                                             \
      NS_LOCAL_LOG("assert failed. cond=\"" << #cond << "\", " << msg);        \
    }                                                                          \
  } while (false)

#else
#define NS_LOCAL_LOG(msg)
#define NS_LOCAL_ASSERT(cond, msg)
#endif

EnvironmentVariable::DictionaryList &EnvironmentVariable::Instance() {
  static DictionaryList instance;
  return instance;
}

void EnvironmentVariable::Clear() { Instance().clear(); }

std::shared_ptr<EnvironmentVariable::Dictionary>
EnvironmentVariable::GetDictionary(const std::string &envvar,
                                   const std::string &delim) {
  NS_LOCAL_LOG(envvar << ", " << delim);
  std::shared_ptr<Dictionary> dict;
  auto loc = Instance().find(envvar);
  if (loc != Instance().end()) {
    NS_LOCAL_LOG("found envvar in cache");
    dict = loc->second;
  } else {
    NS_LOCAL_LOG("envvar not in cache, checking environment");
    dict = std::make_shared<Dictionary>(envvar, delim);
    Instance().insert({envvar, dict});
  }

  return dict;
}

EnvironmentVariable::KeyFoundType
EnvironmentVariable::Get(const std::string &envvar, const std::string &key,
                         const std::string &delim) {
  auto dict = GetDictionary(envvar, delim);
  return dict->Get(key);
}

bool EnvironmentVariable::Set(const std::string &variable,
                              const std::string &value) {
  int fail = setenv(variable.c_str(), value.c_str(), 1);
  return !fail;
}

bool EnvironmentVariable::Unset(const std::string &variable) {
  int fail = unsetenv(variable.c_str());
  return !fail;
}

EnvironmentVariable::KeyFoundType
EnvironmentVariable::Dictionary::Get(const std::string &key) const {
  NS_LOCAL_LOG(key);

  if (!m_exists) {
    return {false, ""};
  }

  if (key.empty()) {
    return {true, m_variable};
  }

  auto loc = m_dict.find(key);
  if (loc != m_dict.end()) {
    NS_LOCAL_LOG("found key in dictionary");
    NS_LOCAL_LOG("found: key '" << key << "', value: '" << loc->second << "'");
    return {true, loc->second};
  }

  return {false, ""};
}

EnvironmentVariable::Dictionary::Dictionary(const std::string &envvar,
                                            const std::string &delim) {
  NS_LOCAL_LOG(envvar << ", " << delim);

  const char *envCstr = std::getenv(envvar.c_str());
  if (!envCstr) {
    m_exists = false;
    return;
  }

  m_exists = true;
  m_variable = envCstr;
  NS_LOCAL_LOG("found envvar in environment with value '" << m_variable << "'");

  if (m_variable.empty()) {
    return;
  }

  StringVector keyvals = SplitString(m_variable, delim);
  NS_LOCAL_ASSERT(keyvals.empty(),
                  "Unexpected empty keyvals from non-empty m_variable");
  for (const auto &keyval : keyvals) {
    if (keyval.empty()) {
      continue;
    }

    std::size_t equals = keyval.find_first_of('=');
    std::string key{keyval, 0, equals};
    std::string value;
    if (equals < keyval.size() - 1) {
      value = keyval.substr(equals + 1, keyval.size());
    }
    NS_LOCAL_LOG("found key '" << key << "' with value '" << value << "'");
    m_dict.insert({key, value});
  }
}

EnvironmentVariable::Dictionary::KeyValueStore
EnvironmentVariable::Dictionary::GetStore() const {
  return m_dict;
}

} // namespace ns3
