
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/spectrum-value.h>
#include <ns3/test.h>
#include <ns3/tv-spectrum-transmitter.h>

NS_LOG_COMPONENT_DEFINE("TvSpectrumTransmitterTest");

using namespace ns3;

const double TOLERANCE = 1e-15;
double epsilon;

class TvSpectrumTransmitterTestCase : public TestCase {
public:
  TvSpectrumTransmitterTestCase(double startFrequency, double channelBandwidth,
                                double basePsd,
                                TvSpectrumTransmitter::TvType tvType);
  ~TvSpectrumTransmitterTestCase() override;

private:
  void DoRun() override;
  static std::string Name(TvSpectrumTransmitter::TvType tvType,
                          double startFrequency, double channelBandwidth,
                          double basePsd);

  double m_startFrequency;
  double m_channelBandwidth;
  double m_basePsd;
  TvSpectrumTransmitter::TvType m_tvType;
};

std::string
TvSpectrumTransmitterTestCase::Name(TvSpectrumTransmitter::TvType tvType,
                                    double startFrequency,
                                    double channelBandwidth, double basePsd) {
  std::ostringstream oss;
  oss << "TV type = " << tvType << ", "
      << "start frequency = " << startFrequency << " Hz, "
      << "channel bandwidth = " << channelBandwidth << " Hz, "
      << "base PSD = " << basePsd << " dBm per Hz";
  return oss.str();
}

TvSpectrumTransmitterTestCase::TvSpectrumTransmitterTestCase(
    double startFrequency, double channelBandwidth, double basePsd,
    TvSpectrumTransmitter::TvType tvType)
    : TestCase(Name(tvType, startFrequency, channelBandwidth, basePsd)),
      m_startFrequency(startFrequency), m_channelBandwidth(channelBandwidth),
      m_basePsd(basePsd), m_tvType(tvType) {}

TvSpectrumTransmitterTestCase::~TvSpectrumTransmitterTestCase() {}

void TvSpectrumTransmitterTestCase::DoRun() {
  NS_LOG_FUNCTION(m_startFrequency << m_basePsd << m_tvType);

  Ptr<TvSpectrumTransmitter> phy = CreateObject<TvSpectrumTransmitter>();
  phy->SetAttribute("StartFrequency", DoubleValue(m_startFrequency));
  phy->SetAttribute("ChannelBandwidth", DoubleValue(m_channelBandwidth));
  phy->SetAttribute("BasePsd", DoubleValue(m_basePsd));
  phy->SetAttribute("TvType", EnumValue(m_tvType));
  phy->CreateTvPsd();

  Ptr<SpectrumValue> psd = phy->GetTxPsd();
  auto psdIter = psd->ConstValuesBegin();
  double maxValue = 0;
  while (psdIter != psd->ConstValuesEnd()) {
    if (*psdIter > maxValue) {
      maxValue = *psdIter;
    }
    ++psdIter;
  }
  double basePsdWattsHz = pow(10.0, (m_basePsd - 30) / 10.0);
  if (m_tvType == TvSpectrumTransmitter::TVTYPE_8VSB) {
    double expectedPsd = (0.502 * basePsdWattsHz) + (21.577 * basePsdWattsHz);
    epsilon = TOLERANCE * std::max(1.0, std::max(maxValue, expectedPsd));
    NS_TEST_ASSERT_MSG_EQ_TOL(maxValue, expectedPsd, epsilon,
                              "peak PSD value (" << maxValue
                                                 << ") is incorrect");
  } else {
    epsilon = TOLERANCE * std::max(1.0, std::max(maxValue, basePsdWattsHz));
    NS_TEST_ASSERT_MSG_EQ_TOL(maxValue, basePsdWattsHz, epsilon,
                              "peak PSD value (" << maxValue
                                                 << ") is incorrect");
  }

  auto bandStart = psd->ConstBandsBegin();
  auto bandEnd = psd->ConstBandsEnd();
  epsilon =
      TOLERANCE * std::max(1.0, std::max((*bandStart).fc, m_startFrequency));
  NS_TEST_ASSERT_MSG_EQ_TOL((*bandStart).fc, m_startFrequency, epsilon,
                            "start frequency value (" << (*bandStart).fc
                                                      << ") is incorrect");
  epsilon = TOLERANCE *
            std::max(1.0, std::max((*bandStart).fc,
                                   (m_startFrequency + m_channelBandwidth)));
  NS_TEST_ASSERT_MSG_EQ_TOL(
      (*(bandEnd - 1)).fc, m_startFrequency + m_channelBandwidth, epsilon,
      "end frequency value (" << (*(bandEnd - 1)).fc << ") is incorrect");
}

class TvSpectrumTransmitterTestSuite : public TestSuite {
public:
  TvSpectrumTransmitterTestSuite();
};

TvSpectrumTransmitterTestSuite::TvSpectrumTransmitterTestSuite()
    : TestSuite("tv-spectrum-transmitter", UNIT) {
  NS_LOG_INFO("creating TvSpectrumTransmitterTestSuite");
  for (double startFreq = 100; startFreq < 1e15; startFreq *= 10) {
    for (double bandwidth = 100; bandwidth < 1e15; bandwidth *= 10) {
      for (double psd = -100; psd <= 100; psd += 20) {
        AddTestCase(
            new TvSpectrumTransmitterTestCase(
                startFreq, bandwidth, psd, TvSpectrumTransmitter::TVTYPE_8VSB),
            TestCase::QUICK);
      }
    }
  }
  for (double startFreq = 100; startFreq < 1e15; startFreq *= 10) {
    for (double bandwidth = 100; bandwidth < 1e15; bandwidth *= 10) {
      for (double psd = -100; psd <= 100; psd += 20) {
        AddTestCase(
            new TvSpectrumTransmitterTestCase(
                startFreq, bandwidth, psd, TvSpectrumTransmitter::TVTYPE_COFDM),
            TestCase::QUICK);
      }
    }
  }
  for (double startFreq = 100; startFreq < 1e15; startFreq *= 10) {
    for (double bandwidth = 100; bandwidth < 1e15; bandwidth *= 10) {
      for (double psd = -100; psd <= 100; psd += 20) {
        AddTestCase(new TvSpectrumTransmitterTestCase(
                        startFreq, bandwidth, psd,
                        TvSpectrumTransmitter::TVTYPE_ANALOG),
                    TestCase::QUICK);
      }
    }
  }
}

static TvSpectrumTransmitterTestSuite g_tvSpectrumTransmitterTestSuite;
