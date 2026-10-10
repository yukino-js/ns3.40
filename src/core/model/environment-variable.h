
#ifndef ENVIRONMENT_VARIABLE_H
#define ENVIRONMENT_VARIABLE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace ns3 {

namespace tests {
class EnvVarTestCase;
}

class EnvironmentVariable {
public:
  using KeyFoundType = std::pair<bool, std::string>;

  static KeyFoundType Get(const std::string &envvar,
                          const std::string &key = "",
                          const std::string &delim = ";");

  class Dictionary;

  static std::shared_ptr<Dictionary>
  GetDictionary(const std::string &envvar, const std::string &delim = ";");

  class Dictionary {
  public:
    Dictionary(const std::string &envvar, const std::string &delim = ";");

    KeyFoundType Get(const std::string &key = "") const;

    using KeyValueStore = std::unordered_map<std::string, std::string>;

    KeyValueStore GetStore() const;

  private:
    bool m_exists;
    std::string m_variable;
    KeyValueStore m_dict;
  };

  static bool Set(const std::string &variable, const std::string &value);

  static bool Unset(const std::string &variable);

  EnvironmentVariable() = delete;
  EnvironmentVariable(const EnvironmentVariable &) = delete;
  EnvironmentVariable &operator=(const EnvironmentVariable &) = delete;
  EnvironmentVariable(EnvironmentVariable &&) = delete;
  EnvironmentVariable &operator=(EnvironmentVariable &&) = delete;

private:
  using DictionaryList =
      std::unordered_map<std::string, std::shared_ptr<Dictionary>>;

  static DictionaryList &Instance();

  friend class tests::EnvVarTestCase;

  static void Clear();
};

} // namespace ns3

#endif
