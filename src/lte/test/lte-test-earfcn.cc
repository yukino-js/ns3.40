
#include "ns3/log.h"
#include "ns3/lte-spectrum-value-helper.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteTestEarfcn");

class LteEarfcnTestCase : public TestCase {
public:
  LteEarfcnTestCase(const char *str, uint32_t earfcn, double f);
  ~LteEarfcnTestCase() override;

protected:
  uint32_t m_earfcn;
  double m_f;

private:
  void DoRun() override;
};

LteEarfcnTestCase::LteEarfcnTestCase(const char *str, uint32_t earfcn, double f)
    : TestCase(str), m_earfcn(earfcn), m_f(f) {
  NS_LOG_FUNCTION(this << str << earfcn << f);
}

LteEarfcnTestCase::~LteEarfcnTestCase() {}

void LteEarfcnTestCase::DoRun() {
  double f = LteSpectrumValueHelper::GetCarrierFrequency(m_earfcn);
  NS_TEST_ASSERT_MSG_EQ_TOL(f, m_f, 0.0000001, "wrong frequency");
}

class LteEarfcnDlTestCase : public LteEarfcnTestCase {
public:
  LteEarfcnDlTestCase(const char *str, uint32_t earfcn, double f);

private:
  void DoRun() override;
};

LteEarfcnDlTestCase::LteEarfcnDlTestCase(const char *str, uint32_t earfcn,
                                         double f)
    : LteEarfcnTestCase(str, earfcn, f) {}

void LteEarfcnDlTestCase::DoRun() {

  double f = LteSpectrumValueHelper::GetDownlinkCarrierFrequency(m_earfcn);
  NS_TEST_ASSERT_MSG_EQ_TOL(f, m_f, 0.0000001, "wrong frequency");
}

class LteEarfcnUlTestCase : public LteEarfcnTestCase {
public:
  LteEarfcnUlTestCase(const char *str, uint32_t earfcn, double f);

private:
  void DoRun() override;
};

LteEarfcnUlTestCase::LteEarfcnUlTestCase(const char *str, uint32_t earfcn,
                                         double f)
    : LteEarfcnTestCase(str, earfcn, f) {}

void LteEarfcnUlTestCase::DoRun() {
  double f = LteSpectrumValueHelper::GetUplinkCarrierFrequency(m_earfcn);
  NS_TEST_ASSERT_MSG_EQ_TOL(f, m_f, 0.0000001, "wrong frequency");
}

class LteEarfcnTestSuite : public TestSuite {
public:
  LteEarfcnTestSuite();
};

static LteEarfcnTestSuite g_lteEarfcnTestSuite;

LteEarfcnTestSuite::LteEarfcnTestSuite() : TestSuite("lte-earfcn", UNIT) {
  NS_LOG_FUNCTION(this);

  AddTestCase(new LteEarfcnDlTestCase("DL EARFCN=500", 500, 2160e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnDlTestCase("DL EARFCN=1000", 1000, 1970e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnDlTestCase("DL EARFCN=1301", 1301, 1815.1e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnDlTestCase("DL EARFCN=7000", 7000, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnDlTestCase("DL EARFCN=20000", 20000, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnDlTestCase("DL EARFCN=50000", 50000, 0.0),
              TestCase::QUICK);

  AddTestCase(new LteEarfcnUlTestCase("UL EARFCN=18100", 18100, 1930e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnUlTestCase("UL EARFCN=19000", 19000, 1890e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnUlTestCase("UL EARFCN=19400", 19400, 1730e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnUlTestCase("UL EARFCN=10", 10, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnUlTestCase("UL EARFCN=1000", 1000, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnUlTestCase("UL EARFCN=50000", 50000, 0.0),
              TestCase::QUICK);

  AddTestCase(new LteEarfcnTestCase("EARFCN=500", 500, 2160e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=1000", 1000, 1970e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=1301", 1301, 1815.1e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=8000", 8000, 0.0), TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=50000", 50000, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=18100", 18100, 1930e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=19000", 19000, 1890e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=19400", 19400, 1730e6),
              TestCase::QUICK);
  AddTestCase(new LteEarfcnTestCase("EARFCN=50000", 50000, 0.0),
              TestCase::QUICK);
}
