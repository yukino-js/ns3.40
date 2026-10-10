
#include "ns3/environment-variable.h"
#include "ns3/test.h"

#include <cstdlib>

namespace ns3 {

namespace tests {

class EnvVarTestCase : public TestCase {
public:
  EnvVarTestCase();

  ~EnvVarTestCase() override;

private:
  void DoRun() override;

  using KeyValueStore = EnvironmentVariable::Dictionary::KeyValueStore;

  using KeyFoundType = EnvironmentVariable::KeyFoundType;

  void SetVariable(const std::string &where, const std::string &value);

  void UnsetVariable(const std::string &where);

  void Check(const std::string &where, const std::string &envValue,
             KeyValueStore expect);

  void SetAndCheck(const std::string &where, const std::string &envValue,
                   KeyValueStore expect);

  void CheckGet(const std::string &where, const std::string &key,
                KeyFoundType expect);

  void SetCheckAndGet(const std::string &where, const std::string &envValue,
                      KeyValueStore expectDict, const std::string &key,
                      KeyFoundType expectValue);

  const std::string m_delimiter{"|"};

  const std::string m_variable{"NS_ENVVAR_TEST"};
};

EnvVarTestCase::EnvVarTestCase() : TestCase("environment-variable-cache") {}

EnvVarTestCase::~EnvVarTestCase() { UnsetVariable("destructor"); }

void EnvVarTestCase::SetVariable(const std::string &where,
                                 const std::string &value) {
  EnvironmentVariable::Clear();
  bool ok = EnvironmentVariable::Set(m_variable, value);
  NS_TEST_EXPECT_MSG_EQ(ok, true, where << ": failed to set variable");

  const char *envCstr = std::getenv(m_variable.c_str());
  NS_TEST_EXPECT_MSG_NE(envCstr, nullptr,
                        where << ": failed to retrieve variable just set");
  NS_TEST_EXPECT_MSG_EQ(envCstr, value,
                        where << ": failed to retrieve value just set");
}

void EnvVarTestCase::UnsetVariable(const std::string &where) {
  EnvironmentVariable::Clear();
  bool ok = EnvironmentVariable::Unset(where);
  NS_TEST_EXPECT_MSG_EQ(ok, true, where << ": failed to unset variable");
}

void EnvVarTestCase::Check(const std::string &where,
                           const std::string &envValue, KeyValueStore expect) {
  auto dict =
      EnvironmentVariable::GetDictionary(m_variable, m_delimiter)->GetStore();

  std::cout << "\n"
            << where << " variable: '" << envValue << "', expect["
            << expect.size() << "]"
            << ", dict[" << dict.size() << "]\n";

  NS_TEST_EXPECT_MSG_EQ(dict.size(), expect.size(),
                        where << ": unequal dictionary sizes");

  std::size_t i{0};
  for (const auto &kv : expect) {
    std::cout << "    [" << i++ << "] '" << kv.first << "'\t'" << kv.second
              << "'";

    auto loc = dict.find(kv.first);
    bool found = loc != dict.end();
    std::cout << (found ? "\tfound" : "\tNOT FOUND");
    NS_TEST_EXPECT_MSG_EQ(found, true,
                          where << ": expected key not found: " << kv.second);

    if (found) {
      bool match = kv.second == loc->second;
      if (match) {
        std::cout << ", match";
      } else {
        std::cout << ", NO MATCH: '" << loc->second << "'";
      }
      NS_TEST_EXPECT_MSG_EQ(kv.second, loc->second,
                            where << ": key found, value mismatch");
    }
    std::cout << "\n";
    ++i;
  }

  i = 0;
  bool first{true};
  for (const auto &kv : dict) {
    bool found = expect.find(kv.first) != expect.end();
    if (!found) {
      std::cout << (first ? "Unexpected keys:" : "");
      first = false;
      std::cout << "    [" << i << "] '" << kv.first << "'\t'" << kv.second
                << "'"
                << " unexpected key, value\n";
    }
    ++i;
  }
}

void EnvVarTestCase::SetAndCheck(const std::string &where,
                                 const std::string &envValue,
                                 KeyValueStore expect) {
  SetVariable(where, envValue);
  Check(where, envValue, expect);
}

void EnvVarTestCase::CheckGet(const std::string &where, const std::string &key,
                              KeyFoundType expect) {
  auto [found, value] = EnvironmentVariable::Get(m_variable, key, m_delimiter);
  NS_TEST_EXPECT_MSG_EQ(found, expect.first,
                        where << ": key '" << key << "' "
                              << (expect.first ? "not " : "")
                              << "found unexpectedly");
  NS_TEST_EXPECT_MSG_EQ(value, expect.second,
                        where << ": incorrect value for key '" << key << "'");
}

void EnvVarTestCase::SetCheckAndGet(const std::string &where,
                                    const std::string &envValue,
                                    KeyValueStore expectDict,
                                    const std::string &key,
                                    KeyFoundType expectValue) {
  SetAndCheck(where, envValue, expectDict);
  CheckGet(where, key, expectValue);
}

void EnvVarTestCase::DoRun() {
  UnsetVariable("unset");
  Check("unset", "", {});
  auto [found, value] = EnvironmentVariable::Get(m_variable);
  NS_TEST_EXPECT_MSG_EQ(found, false, "unset: variable found when not set");
  NS_TEST_EXPECT_MSG_EQ(value.empty(), true,
                        "unset: non-empty value from unset variable");

#ifndef __WIN32__
  SetCheckAndGet("empty", "", {}, "", {true, ""});
#endif

  SetCheckAndGet("no-key", "not|the|right=value",
                 {{"not", ""}, {"the", ""}, {"right", "value"}}, "key",
                 {false, ""});

  SetCheckAndGet("key-only", "key", {{"key", ""}}, "key", {true, ""});

  SetCheckAndGet("front-|", "|key", {{"key", ""}}, "key", {true, ""});
  SetCheckAndGet("back-|", "key|", {{"key", ""}}, "key", {true, ""});

  SetCheckAndGet("front-||", "||key", {{"key", ""}}, "key", {true, ""});
  SetCheckAndGet("back-||", "key||", {{"key", ""}}, "key", {true, ""});

  SetCheckAndGet("two keys", "key1|key2", {{"key1", ""}, {"key2", ""}}, "key1",
                 {true, ""});
  CheckGet("two keys", "key2", {true, ""});

  SetCheckAndGet("||two keys", "||key1|key2", {{"key1", ""}, {"key2", ""}},
                 "key1", {true, ""});
  CheckGet("||two keys", "key2", {true, ""});
  SetCheckAndGet("two keys||", "key1|key2||", {{"key1", ""}, {"key2", ""}},
                 "key1", {true, ""});
  CheckGet("two keys||", "key2", {true, ""});

  SetCheckAndGet("key-val", "key=value", {{"key", "value"}}, "key",
                 {true, "value"});

  SetCheckAndGet(
      "mixed", "key1|key2=value|key3|key4=value",
      {{"key1", ""}, {"key2", "value"}, {"key3", ""}, {"key4", "value"}},
      "key1", {true, ""});
  CheckGet("mixed", "key2", {true, "value"});
  CheckGet("mixed", "key3", {true, ""});
  CheckGet("mixed", "key4", {true, "value"});

  SetCheckAndGet("key=", "key=", {{"key", ""}}, "key", {true, ""});

  SetCheckAndGet("key==", "key==", {{"key", "="}}, "key", {true, "="});

  std::cout << std::endl;
}

class EnvironmentVariableTestSuite : public TestSuite {
public:
  EnvironmentVariableTestSuite();
};

EnvironmentVariableTestSuite::EnvironmentVariableTestSuite()
    : TestSuite("environment-variables") {
  AddTestCase(new EnvVarTestCase);
}

static EnvironmentVariableTestSuite g_EnvironmentVariableTestSuite;

} // namespace tests

} // namespace ns3
