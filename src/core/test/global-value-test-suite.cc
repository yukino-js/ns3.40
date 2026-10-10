#include "ns3/global-value.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

namespace ns3 {

namespace tests {

class GlobalValueTestCase : public TestCase {
public:
  GlobalValueTestCase();

  ~GlobalValueTestCase() override {}

private:
  void DoRun() override;
};

GlobalValueTestCase::GlobalValueTestCase()
    : TestCase("Check GlobalValue mechanism") {}

void GlobalValueTestCase::DoRun() {
  GlobalValue uint = GlobalValue("TestUint", "help text", UintegerValue(10),
                                 MakeUintegerChecker<uint32_t>());

  UintegerValue uv;
  uint.GetValue(uv);
  NS_TEST_ASSERT_MSG_EQ(uv.Get(), 10,
                        "GlobalValue \"TestUint\" not initialized as expected");

  GlobalValue::Vector *vector = GlobalValue::GetVector();
  for (auto i = vector->begin(); i != vector->end(); ++i) {
    if ((*i) == &uint) {
      vector->erase(i);
      break;
    }
  }
}

class GlobalValueTestSuite : public TestSuite {
public:
  GlobalValueTestSuite();
};

GlobalValueTestSuite::GlobalValueTestSuite() : TestSuite("global-value") {
  AddTestCase(new GlobalValueTestCase);
}

static GlobalValueTestSuite g_globalValueTestSuite;

} // namespace tests

} // namespace ns3
