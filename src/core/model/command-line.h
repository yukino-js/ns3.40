#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "callback.h"
#include "nstime.h"
#include "type-id.h"

#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace ns3 {

class CommandLine {
public:
  CommandLine();
  CommandLine(const std::string &filename);
  CommandLine(const CommandLine &cmd);
  CommandLine &operator=(const CommandLine &cmd);
  ~CommandLine();

  void Usage(const std::string &usage);

  template <typename T>
  void AddValue(const std::string &name, const std::string &help, T &value);

  void AddValue(const std::string &name, const std::string &help, char *value,
                std::size_t num);
  typedef bool (*Callback)(const std::string &value);

  void AddValue(const std::string &name, const std::string &help,
                ns3::Callback<bool, const std::string &> callback,
                const std::string &defaultValue = "");

  void AddValue(const std::string &name, const std::string &attributePath);

  template <typename T>
  void AddNonOption(const std::string &name, const std::string &help, T &value);

  std::string GetExtraNonOption(std::size_t i) const;

  std::size_t GetNExtraNonOptions() const;

  void Parse(int argc, char *argv[]);

  void Parse(std::vector<std::string> args);

  std::string GetName() const;

  void PrintHelp(std::ostream &os) const;

  std::string GetVersion() const;

  void PrintVersion(std::ostream &os) const;

private:
  class Item {
  public:
    std::string m_name;
    std::string m_help;
    virtual ~Item();
    virtual bool Parse(const std::string &value) const = 0;
    virtual bool HasDefault() const;
    virtual std::string GetDefault() const = 0;
  };

  template <typename T> class UserItem : public Item {
  public:
    bool Parse(const std::string &value) const override;
    bool HasDefault() const override;
    std::string GetDefault() const override;

    T *m_valuePtr;
    std::string m_default;
  };

  class StringItem : public Item {
  public:
    bool Parse(const std::string &value) const override;
    bool HasDefault() const override;
    std::string GetDefault() const override;

    mutable std::string m_value;
  };

  class CharStarItem : public Item {
  public:
    bool Parse(const std::string &value) const override;
    bool HasDefault() const override;
    std::string GetDefault() const override;

    char *m_buffer;
    std::size_t m_size;
    std::string m_default;
  };

  class CallbackItem : public Item {
  public:
    bool Parse(const std::string &value) const override;
    bool HasDefault() const override;
    std::string GetDefault() const override;

    ns3::Callback<bool, const std::string &> m_callback;
    std::string m_default;
  };

  using HasOptionName = std::tuple<bool, std::string, std::string>;

  HasOptionName GetOptionName(const std::string &param) const;
  void HandleHardOptions(const std::vector<std::string> &args) const;

  bool HandleOption(const std::string &param) const;

  bool HandleNonOption(const std::string &value);

  bool HandleArgument(const std::string &name, const std::string &value) const;
  static bool HandleAttribute(const std::string &name,
                              const std::string &value);

  void PrintGlobals(std::ostream &os) const;
  void PrintAttributes(std::ostream &os, const std::string &type) const;
  void PrintAttributeList(std::ostream &os, const TypeId tid,
                          std::stringstream &header) const;
  void PrintGroup(std::ostream &os, const std::string &group) const;
  void PrintTypeIds(std::ostream &os) const;
  void PrintGroups(std::ostream &os) const;
  void Copy(const CommandLine &cmd);
  void Clear();
  void PrintDoxygenUsage() const;

  using Items = std::vector<std::shared_ptr<Item>>;

  Items m_options;
  Items m_nonOptions;

  std::size_t m_NNonOptions;
  std::size_t m_nonOptionCount;
  std::string m_usage;
  std::string m_shortName;
};

namespace CommandLineHelper {

template <typename T> bool UserItemParse(const std::string &value, T &dest);
template <> bool UserItemParse<bool>(const std::string &value, bool &dest);
template <>
bool UserItemParse<uint8_t>(const std::string &value, uint8_t &dest);

template <typename T> std::string GetDefault(const std::string &defaultValue);
template <> std::string GetDefault<bool>(const std::string &defaultValue);
template <> std::string GetDefault<Time>(const std::string &defaultValue);

} // namespace CommandLineHelper

} // namespace ns3

namespace ns3 {

template <typename T>
void CommandLine::AddValue(const std::string &name, const std::string &help,
                           T &value) {
  auto item = std::make_shared<UserItem<T>>();
  item->m_name = name;
  item->m_help = help;
  item->m_valuePtr = &value;

  std::stringstream ss;
  ss << value;
  ss >> item->m_default;

  m_options.push_back(item);
}

template <typename T>
void CommandLine::AddNonOption(const std::string &name, const std::string &help,
                               T &value) {
  auto item = std::make_shared<UserItem<T>>();
  item->m_name = name;
  item->m_help = help;
  item->m_valuePtr = &value;

  std::stringstream ss;
  ss << value;
  ss >> item->m_default;
  m_nonOptions.push_back(item);
  ++m_NNonOptions;
}

template <typename T> bool CommandLine::UserItem<T>::HasDefault() const {
  return !m_default.empty();
}

template <typename T> std::string CommandLine::UserItem<T>::GetDefault() const {
  return CommandLineHelper::GetDefault<T>(m_default);
}

template <typename T>
std::string CommandLineHelper::GetDefault(const std::string &defaultValue) {
  return defaultValue;
}

template <typename T>
bool CommandLine::UserItem<T>::Parse(const std::string &value) const {
  return CommandLineHelper::UserItemParse<T>(value, *m_valuePtr);
}

template <typename T>
bool CommandLineHelper::UserItemParse(const std::string &value, T &dest) {
  std::istringstream iss;
  iss.str(value);
  iss >> dest;
  return !iss.bad() && !iss.fail();
}

std::ostream &operator<<(std::ostream &os, const CommandLine &cmd);

} // namespace ns3

#endif
