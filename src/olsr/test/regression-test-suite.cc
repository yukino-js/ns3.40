
#include "bug780-test.h"
#include "hello-regression-test.h"
#include "tc-regression-test.h"

using namespace ns3;
using namespace olsr;

class RegressionTestSuite : public TestSuite {
public:
  RegressionTestSuite() : TestSuite("routing-olsr-regression", SYSTEM) {
    SetDataDir(NS_TEST_SOURCEDIR);
    AddTestCase(new HelloRegressionTest, TestCase::QUICK);
    AddTestCase(new TcRegressionTest, TestCase::QUICK);
    AddTestCase(new Bug780Test, TestCase::QUICK);
  }
};

static RegressionTestSuite g_olsrRegressionTestSuite;
