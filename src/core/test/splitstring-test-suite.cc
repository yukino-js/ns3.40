
#include "ns3/string.h"
#include "ns3/test.h"

namespace ns3 {

namespace tests {

class SplitStringTestCase : public TestCase {
public:
  SplitStringTestCase();

  ~SplitStringTestCase() override = default;

private:
  void DoRun() override;

  void Check(const std::string &where, const std::string &str,
             const StringVector &expect);

  const std::string m_delimiter{":|:"};
};

SplitStringTestCase::SplitStringTestCase() : TestCase("split-string") {}

void SplitStringTestCase::Check(const std::string &where,
                                const std::string &str,
                                const StringVector &expect) {
  const StringVector res = SplitString(str, m_delimiter);

  std::cout << where << ": '" << str << "'\nindex\texpect[" << expect.size()
            << "]\t\tresult[" << res.size() << "]"
            << (expect.size() != res.size() ? "\tFAIL SIZE" : "") << "\n    ";
  NS_TEST_EXPECT_MSG_EQ(expect.size(), res.size(),
                        "res and expect have different number of entries");
  for (std::size_t i = 0; i < std::max(res.size(), expect.size()); ++i) {
    const std::string r = (i < res.size() ? res[i] : "''" + std::to_string(i));
    const std::string e =
        (i < expect.size() ? expect[i] : "''" + std::to_string(i));
    const bool ok = (r == e);
    std::cout << i << "\t'" << e << (ok ? "'\t== '" : "'\t!= '") << r
              << (!ok ? "'\tFAIL MATCH" : "'") << "\n    ";
    NS_TEST_EXPECT_MSG_EQ(e, r, "res[i] does not match expect[i]");
  }
  std::cout << std::endl;
}

void SplitStringTestCase::DoRun() {

  Check("empty", "", {""});

  Check("no-delim", "token", {"token"});

  Check("front-:|:", ":|:token", {"", "token"});
  Check("back-:|:", "token:|:", {"token", ""});

  Check("front-:|::|:", ":|::|:token", {"", "", "token"});
  Check("back-:|::|:", "token:|::|:", {"token", "", ""});

  Check("two", "token1:|:token2", {"token1", "token2"});

  Check(":|::|:two", ":|::|:token1:|:token2", {"", "", "token1", "token2"});
  Check(":|:one:|:two", ":|:token1:|:token2", {"", "token1", "token2"});
  Check("double:|:", "token1:|::|:token2", {"token1", "", "token2"});
  Check("two:|:", "token1:|:token2:|::|:", {"token1", "token2", "", ""});
}

class SplitStringTestSuite : public TestSuite {
public:
  SplitStringTestSuite();
};

SplitStringTestSuite::SplitStringTestSuite() : TestSuite("split-string") {
  AddTestCase(new SplitStringTestCase);
}

static SplitStringTestSuite g_SplitStringTestSuite;

} // namespace tests

} // namespace ns3
