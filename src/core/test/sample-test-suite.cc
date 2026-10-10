
#include "ns3/test.h"

namespace ns3 {

namespace tests {

class SampleTestCase1 : public TestCase {
public:
  SampleTestCase1();
  ~SampleTestCase1() override;

private:
  void DoRun() override;
};

SampleTestCase1::SampleTestCase1()
    : TestCase("Sample test case (does nothing)") {}

SampleTestCase1::~SampleTestCase1() {}

void SampleTestCase1::DoRun() {
  NS_TEST_ASSERT_MSG_EQ(true, true, "true doesn't equal true for some reason");
  NS_TEST_ASSERT_MSG_EQ_TOL(0.01, 0.01, 0.001,
                            "Numbers are not equal within tolerance");
}

class SampleTestSuite : public TestSuite {
public:
  SampleTestSuite();
};

SampleTestSuite::SampleTestSuite() : TestSuite("sample") {
  AddTestCase(new SampleTestCase1);
}

static SampleTestSuite g_sampleTestSuite;

} // namespace tests

} // namespace ns3
