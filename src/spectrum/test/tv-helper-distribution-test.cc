
#include <ns3/log.h>
#include <ns3/test.h>
#include <ns3/tv-spectrum-transmitter-helper.h>

NS_LOG_COMPONENT_DEFINE("TvHelperDistributionTest");

using namespace ns3;

class TvHelperDistributionTestCase : public TestCase {
public:
  TvHelperDistributionTestCase(uint32_t maxNumTransmitters);
  ~TvHelperDistributionTestCase() override;

private:
  void DoRun() override;
  static std::string Name(uint32_t maxNumTransmitters);
  uint32_t m_maxNumTransmitters;
};

std::string TvHelperDistributionTestCase::Name(uint32_t maxNumTransmitters) {
  std::ostringstream oss;
  oss << "Max Number of Transmitters = " << maxNumTransmitters;
  return oss.str();
}

TvHelperDistributionTestCase::TvHelperDistributionTestCase(
    uint32_t maxNumTransmitters)
    : TestCase(Name(maxNumTransmitters)),
      m_maxNumTransmitters(maxNumTransmitters) {}

TvHelperDistributionTestCase::~TvHelperDistributionTestCase() {}

void TvHelperDistributionTestCase::DoRun() {
  NS_LOG_FUNCTION(m_maxNumTransmitters);
  TvSpectrumTransmitterHelper tvTransHelper;
  uint32_t rand;
  uint32_t maxLow = 0;
  uint32_t minMid = m_maxNumTransmitters;
  uint32_t maxMid = 0;
  uint32_t minHigh = m_maxNumTransmitters;
  for (int i = 0; i < 30; i++) {
    rand = tvTransHelper.GetRandomNumTransmitters(
        TvSpectrumTransmitterHelper::DENSITY_LOW, m_maxNumTransmitters);
    NS_TEST_ASSERT_MSG_GT(rand, 0, "lower bound exceeded");
    if (rand > maxLow) {
      maxLow = rand;
    }
  }
  for (int i = 0; i < 30; i++) {
    rand = tvTransHelper.GetRandomNumTransmitters(
        TvSpectrumTransmitterHelper::DENSITY_MEDIUM, m_maxNumTransmitters);
    if (rand < minMid) {
      minMid = rand;
    }
    if (rand > maxMid) {
      maxMid = rand;
    }
  }
  for (int i = 0; i < 30; i++) {
    rand = tvTransHelper.GetRandomNumTransmitters(
        TvSpectrumTransmitterHelper::DENSITY_HIGH, m_maxNumTransmitters);
    NS_TEST_ASSERT_MSG_LT(rand, m_maxNumTransmitters + 1,
                          "upper bound exceeded");
    if (rand < minHigh) {
      minHigh = rand;
    }
  }
  NS_TEST_ASSERT_MSG_LT(maxLow, minMid,
                        "low density overlaps with medium density");
  NS_TEST_ASSERT_MSG_LT(maxMid, minHigh,
                        "medium density overlaps with high density");
}

class TvHelperDistributionTestSuite : public TestSuite {
public:
  TvHelperDistributionTestSuite();
};

TvHelperDistributionTestSuite::TvHelperDistributionTestSuite()
    : TestSuite("tv-helper-distribution", UNIT) {
  NS_LOG_INFO("creating TvHelperDistributionTestSuite");
  for (uint32_t maxNumTransmitters = 3; maxNumTransmitters <= 203;
       maxNumTransmitters += 10) {
    AddTestCase(new TvHelperDistributionTestCase(maxNumTransmitters),
                TestCase::QUICK);
  }
}

static TvHelperDistributionTestSuite g_TvHelperDistributionTestSuite;
