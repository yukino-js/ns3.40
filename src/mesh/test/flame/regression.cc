
#include "flame-regression.h"

#include "ns3/test.h"

using namespace ns3;

class FlameRegressionSuite : public TestSuite {
public:
  FlameRegressionSuite() : TestSuite("devices-mesh-flame-regression", SYSTEM) {
    SetDataDir(std::string("src/mesh/test/flame"));
    AddTestCase(new FlameRegressionTest, TestCase::QUICK);
  }
} g_flameRegressionSuite;
