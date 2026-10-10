
#include "hwmp-proactive-regression.h"
#include "hwmp-reactive-regression.h"
#include "hwmp-simplest-regression.h"
#include "hwmp-target-flags-regression.h"
#include "pmp-regression.h"

#include "ns3/test.h"

using namespace ns3;

class Dot11sRegressionSuite : public TestSuite {
public:
  Dot11sRegressionSuite()
      : TestSuite("devices-mesh-dot11s-regression", SYSTEM) {
    SetDataDir(std::string("src/mesh/test/dot11s"));
    AddTestCase(new PeerManagementProtocolRegressionTest, TestCase::QUICK);
    AddTestCase(new HwmpSimplestRegressionTest, TestCase::QUICK);
    AddTestCase(new HwmpReactiveRegressionTest, TestCase::QUICK);
    AddTestCase(new HwmpProactiveRegressionTest, TestCase::QUICK);
    AddTestCase(new HwmpDoRfRegressionTest, TestCase::QUICK);
  }
} g_dot11sRegressionSuite;
