

#include "ns3/mtp-module.h"

#include "ns3/test.h"

using namespace ns3;

class MtpTestCase1 : public TestCase {
public:
  MtpTestCase1();
  virtual ~MtpTestCase1();

private:
  virtual void DoRun(void);
};

MtpTestCase1::MtpTestCase1() : TestCase("Mtp test case (does nothing)") {}

MtpTestCase1::~MtpTestCase1() {}

void MtpTestCase1::DoRun(void) {
  NS_TEST_ASSERT_MSG_EQ(true, true, "true doesn't equal true for some reason");
  NS_TEST_ASSERT_MSG_EQ_TOL(0.01, 0.01, 0.001,
                            "Numbers are not equal within tolerance");
}

class MtpTestSuite : public TestSuite {
public:
  MtpTestSuite();
};

MtpTestSuite::MtpTestSuite() : TestSuite("mtp", UNIT) {
  AddTestCase(new MtpTestCase1, TestCase::QUICK);
}

static MtpTestSuite smtpTestSuite;
