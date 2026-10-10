
#include <ns3/abort.h>
#include <ns3/config.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/double.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/log.h>
#include <ns3/mobility-helper.h>
#include <ns3/node-container.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/string.h>
#include <ns3/test.h>
#include <ns3/three-gpp-antenna-model.h>
#include <ns3/three-gpp-channel-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/two-ray-spectrum-propagation-loss-model.h>
#include <ns3/uinteger.h>
#include <ns3/uniform-planar-array.h>

#include <array>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TwoRaySplmTestSuite");

class FtrFadingModelAverageTest : public TestCase {
public:
  FtrFadingModelAverageTest();

  ~FtrFadingModelAverageTest() override;

private:
  void DoRun() override;

  double FtrSquaredNormAverage(
      const TwoRaySpectrumPropagationLossModel::FtrParams &ftrParams) const;

  constexpr double FtrSquaredNormExpectedMean(double sigma, double k) const;

  static constexpr uint32_t N_MEASUREMENTS{100000};

  static constexpr double TOLERANCE{1e-2};

  static constexpr uint8_t NUM_VALUES{3};

  static constexpr uint16_t MAX_M_VALUE{1000};
};

FtrFadingModelAverageTest::FtrFadingModelAverageTest()
    : TestCase("Check that the average of the Fluctuating Two Ray model is "
               "consistent with the "
               "theoretical expectation") {}

FtrFadingModelAverageTest::~FtrFadingModelAverageTest() {}

double FtrFadingModelAverageTest::FtrSquaredNormAverage(
    const TwoRaySpectrumPropagationLossModel::FtrParams &ftrParams) const {
  NS_LOG_FUNCTION(this);
  double sum = 0.0;
  auto twoRaySplm = CreateObject<TwoRaySpectrumPropagationLossModel>();
  twoRaySplm->AssignStreams(1);
  for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
    double value = twoRaySplm->GetFtrFastFading(ftrParams);
    sum += value;
  }
  double valueMean = sum / N_MEASUREMENTS;
  return valueMean;
}

double constexpr FtrFadingModelAverageTest::FtrSquaredNormExpectedMean(
    double sigma, double k) const {
  return 2 * sigma * (1 + k);
}

void FtrFadingModelAverageTest::DoRun() {
  std::array<double, NUM_VALUES> sigma;
  std::array<double, NUM_VALUES> k;
  std::array<double, NUM_VALUES> delta;

  for (uint8_t j = 0; j < NUM_VALUES; j++) {
    double power = std::pow(2, j);

    sigma[j] = power;
    k[j] = power;
    delta[j] = double(j) / NUM_VALUES;
  }

  auto unifRv = CreateObject<UniformRandomVariable>();

  for (uint8_t l = 0; l < NUM_VALUES; l++) {
    for (uint8_t m = 0; m < NUM_VALUES; m++) {
      for (uint8_t n = 0; n < NUM_VALUES; n++) {
        auto ftrParams = TwoRaySpectrumPropagationLossModel::FtrParams(
            unifRv->GetInteger(1, MAX_M_VALUE), sigma[l], k[m], delta[n]);
        double valueMean = FtrSquaredNormAverage(ftrParams);
        double expectedMean =
            FtrSquaredNormExpectedMean(ftrParams.m_sigma, ftrParams.m_k);

        NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean,
                                  expectedMean * TOLERANCE, "wrong mean value");
      }
    }
  }
}

class ArrayResponseTest : public TestCase {
public:
  ArrayResponseTest(Ptr<AntennaModel> txAntElem, Ptr<AntennaModel> rxAntElem,
                    uint16_t txNumAntennas, uint16_t rxNumAntennas,
                    Vector txPosVec, Vector rxPosVec, double txBearing,
                    double rxBearing, double expectedGain);

  ~ArrayResponseTest() override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-8};

  Ptr<AntennaModel> m_txAntElem;
  Ptr<AntennaModel> m_rxAntElem;
  uint16_t m_txNumAntennas;
  uint16_t m_rxNumAntennas;
  Vector m_txPosVec;
  Vector m_rxPosVec;
  double m_txBearing;
  double m_rxBearing;
  double m_expectedGain;
};

ArrayResponseTest::ArrayResponseTest(Ptr<AntennaModel> txAntElem,
                                     Ptr<AntennaModel> rxAntElem,
                                     uint16_t txNumAntennas,
                                     uint16_t rxNumAntennas, Vector txPosVec,
                                     Vector rxPosVec, double txBearing,
                                     double rxBearing, double expectedGain)
    : TestCase("Check that the overall array response gain has the proper "
               "trend with respect to"
               "the number of antennas and the type of single element antenna"),
      m_txAntElem(txAntElem), m_rxAntElem(rxAntElem),
      m_txNumAntennas(txNumAntennas), m_rxNumAntennas(rxNumAntennas),
      m_txPosVec(txPosVec), m_rxPosVec(rxPosVec), m_txBearing(txBearing),
      m_rxBearing(rxBearing), m_expectedGain(expectedGain) {}

ArrayResponseTest::~ArrayResponseTest() {}

void ArrayResponseTest::DoRun() {
  auto twoRaySplm = CreateObject<TwoRaySpectrumPropagationLossModel>();
  twoRaySplm->AssignStreams(1);

  auto channelConditionModel = CreateObject<AlwaysLosChannelConditionModel>();
  twoRaySplm->SetAttribute("ChannelConditionModel",
                           PointerValue(channelConditionModel));

  auto txArray = CreateObject<UniformPlanarArray>();
  auto rxArray = CreateObject<UniformPlanarArray>();
  txArray->SetAttribute("AntennaElement", PointerValue(m_txAntElem));
  rxArray->SetAttribute("AntennaElement", PointerValue(m_rxAntElem));

  auto txPos = CreateObject<ConstantPositionMobilityModel>();
  auto rxPos = CreateObject<ConstantPositionMobilityModel>();
  txPos->SetAttribute("Position", VectorValue(m_txPosVec));
  rxPos->SetAttribute("Position", VectorValue(m_rxPosVec));

  txArray->SetAttribute("BearingAngle", DoubleValue(m_txBearing));
  rxArray->SetAttribute("BearingAngle", DoubleValue(m_rxBearing));

  txArray->SetAttribute("NumRows", UintegerValue(std::sqrt(m_txNumAntennas)));
  txArray->SetAttribute("NumColumns",
                        UintegerValue(std::sqrt(m_txNumAntennas)));
  rxArray->SetAttribute("NumRows", UintegerValue(std::sqrt(m_rxNumAntennas)));
  rxArray->SetAttribute("NumColumns",
                        UintegerValue(std::sqrt(m_rxNumAntennas)));

  auto txBfVec = txArray->GetBeamformingVector(Angles(m_rxPosVec, m_txPosVec));
  auto rxBfVec = rxArray->GetBeamformingVector(Angles(m_txPosVec, m_rxPosVec));
  txArray->SetBeamformingVector(txBfVec);
  rxArray->SetBeamformingVector(rxBfVec);

  double gainTxRx =
      twoRaySplm->CalcBeamformingGain(txPos, rxPos, txArray, rxArray);
  double gainRxTx =
      twoRaySplm->CalcBeamformingGain(rxPos, txPos, rxArray, txArray);

  NS_TEST_EXPECT_MSG_EQ_TOL(gainTxRx, gainRxTx, gainTxRx * TOLERANCE,
                            "gain should be symmetric");
  NS_TEST_EXPECT_MSG_EQ_TOL(
      10 * log10(gainTxRx), m_expectedGain, m_expectedGain * TOLERANCE,
      "gain different from the theoretically expected value");
}

class OverallGainAverageTest : public TestCase {
public:
  OverallGainAverageTest(Ptr<AntennaModel> txAntElem,
                         Ptr<AntennaModel> rxAntElem, uint16_t txNumAntennas,
                         uint16_t rxNumAntennas, double fc,
                         std::string threeGppScenario);

  double ComputePowerSpectralDensityOverallPower(Ptr<const SpectrumValue> psd);

  Ptr<SpectrumValue> CreateTxPowerSpectralDensity(double fc);

  ~OverallGainAverageTest() override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{0.02};

  static constexpr uint32_t N_MEASUREMENTS{1000};

  static constexpr double M_BW{200e6};

  static constexpr double M_RB_WIDTH{1e6};

  Ptr<AntennaModel> m_txAntElem;
  Ptr<AntennaModel> m_rxAntElem;
  uint16_t m_txNumAntennas;
  uint16_t m_rxNumAntennas;
  double m_fc;
  std::string m_threeGppScenario;
};

OverallGainAverageTest::OverallGainAverageTest(Ptr<AntennaModel> txAntElem,
                                               Ptr<AntennaModel> rxAntElem,
                                               uint16_t txNumAntennas,
                                               uint16_t rxNumAntennas,
                                               double fc,
                                               std::string threeGppScenario)
    : TestCase("Check that the overall array response gain has the proper "
               "trend with respect to"
               "the number of antennas and the type of single element antenna"),
      m_txAntElem(txAntElem), m_rxAntElem(rxAntElem),
      m_txNumAntennas(txNumAntennas), m_rxNumAntennas(rxNumAntennas), m_fc(fc),
      m_threeGppScenario(threeGppScenario) {}

OverallGainAverageTest::~OverallGainAverageTest() {}

double OverallGainAverageTest::ComputePowerSpectralDensityOverallPower(
    Ptr<const SpectrumValue> psd) {
  return Integral(*psd);
}

Ptr<SpectrumValue>
OverallGainAverageTest::CreateTxPowerSpectralDensity(double fc) {
  uint32_t numRbs = std::floor(M_BW / M_RB_WIDTH);
  double f = fc - (numRbs * M_RB_WIDTH / 2.0);
  double powerTx = 0.0;

  Bands rbs;
  std::vector<int> rbsId;
  rbs.reserve(numRbs);
  rbsId.reserve(numRbs);
  for (uint32_t numrb = 0; numrb < numRbs; ++numrb) {
    BandInfo rb;
    rb.fl = f;
    f += M_RB_WIDTH / 2;
    rb.fc = f;
    f += M_RB_WIDTH / 2;
    rb.fh = f;

    rbs.push_back(rb);
    rbsId.push_back(numrb);
  }
  Ptr<SpectrumModel> model = Create<SpectrumModel>(rbs);
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(model);

  double powerTxW = std::pow(10., (powerTx - 30) / 10);
  double txPowerDensity = powerTxW / M_BW;

  for (const auto &rbId : rbsId) {
    (*txPsd)[rbId] = txPowerDensity;
  }

  return txPsd;
}

void OverallGainAverageTest::DoRun() {
  auto twoRaySplm = CreateObject<TwoRaySpectrumPropagationLossModel>();
  auto threeGppSplm = CreateObject<ThreeGppSpectrumPropagationLossModel>();
  auto threeGppChannelModel = CreateObject<ThreeGppChannelModel>();
  auto channelConditionModel = CreateObject<AlwaysLosChannelConditionModel>();
  twoRaySplm->AssignStreams(1);
  threeGppChannelModel->AssignStreams(1);

  threeGppSplm->SetAttribute("ChannelModel",
                             PointerValue(threeGppChannelModel));
  threeGppChannelModel->SetAttribute("ChannelConditionModel",
                                     PointerValue(channelConditionModel));
  twoRaySplm->SetAttribute("ChannelConditionModel",
                           PointerValue(channelConditionModel));

  NodeContainer nodes;
  nodes.Create(2);
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  Vector txPosVec(0.0, 0.0, 0.0);
  Vector rxPosVec(5.0, 0.0, 0.0);
  positionAlloc->Add(txPosVec);
  positionAlloc->Add(rxPosVec);
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  auto txArray = CreateObject<UniformPlanarArray>();
  txArray->SetAttribute("AntennaElement", PointerValue(m_txAntElem));

  txArray->SetAttribute("BearingAngle", DoubleValue(0));

  txArray->SetAttribute("NumRows", UintegerValue(std::sqrt(m_txNumAntennas)));
  txArray->SetAttribute("NumColumns",
                        UintegerValue(std::sqrt(m_txNumAntennas)));

  threeGppChannelModel->SetAttribute("Frequency", DoubleValue(m_fc));
  twoRaySplm->SetAttribute("Frequency", DoubleValue(m_fc));
  threeGppChannelModel->SetAttribute("Scenario",
                                     StringValue(m_threeGppScenario));
  twoRaySplm->SetAttribute("Scenario", StringValue(m_threeGppScenario));

  threeGppChannelModel->SetAttribute("Blockage", BooleanValue(false));

  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity(m_fc);
  double txPower = ComputePowerSpectralDensityOverallPower(txPsd);

  Ptr<SpectrumSignalParameters> signalParams =
      Create<SpectrumSignalParameters>();
  signalParams->psd = txPsd;

  auto txBfVec = txArray->GetBeamformingVector(Angles(rxPosVec, txPosVec));
  txArray->SetBeamformingVector(txBfVec);

  Ptr<MobilityModel> txMob = nodes.Get(0)->GetObject<MobilityModel>();
  Ptr<MobilityModel> rxMob = nodes.Get(1)->GetObject<MobilityModel>();

  double threeGppGainMean = 0;
  double twoRayGainMean = 0;

  for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
    auto rxArray = CreateObject<UniformPlanarArray>();

    rxArray->SetAttribute("AntennaElement", PointerValue(m_rxAntElem));
    rxArray->SetAttribute("BearingAngle", DoubleValue(-M_PI));
    rxArray->SetAttribute("NumRows", UintegerValue(std::sqrt(m_rxNumAntennas)));
    rxArray->SetAttribute("NumColumns",
                          UintegerValue(std::sqrt(m_rxNumAntennas)));

    auto rxBfVec = rxArray->GetBeamformingVector(Angles(txPosVec, rxPosVec));
    rxArray->SetBeamformingVector(rxBfVec);

    auto twoRayRxPsd = twoRaySplm->DoCalcRxPowerSpectralDensity(
        signalParams, txMob, rxMob, txArray, rxArray);
    auto threeGppRayRxPsd = threeGppSplm->DoCalcRxPowerSpectralDensity(
        signalParams, txMob, rxMob, txArray, rxArray);
    double twoRayRxPower = ComputePowerSpectralDensityOverallPower(twoRayRxPsd);
    double threeGppRxPower =
        ComputePowerSpectralDensityOverallPower(threeGppRayRxPsd);

    twoRayGainMean += (twoRayRxPower / txPower);
    threeGppGainMean += (threeGppRxPower / txPower);
  }

  NS_TEST_EXPECT_MSG_EQ_TOL(twoRayGainMean / N_MEASUREMENTS,
                            threeGppGainMean / N_MEASUREMENTS,
                            twoRayGainMean * TOLERANCE,
                            "The 3GPP and Two Ray models should provide "
                            "similar average channel gains");
}

class TwoRaySplmTestSuite : public TestSuite {
public:
  TwoRaySplmTestSuite();
};

TwoRaySplmTestSuite::TwoRaySplmTestSuite()
    : TestSuite("two-ray-splm-suite", UNIT) {
  AddTestCase(new FtrFadingModelAverageTest, TestCase::QUICK);

  auto iso = CreateObject<IsotropicAntennaModel>();
  auto tgpp = CreateObject<ThreeGppAntennaModel>();
  const double maxTgppGain = tgpp->GetGainDb(Angles(0.0, M_PI / 2));

  AddTestCase(new ArrayResponseTest(iso, iso, 1, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI, 0.0),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(iso, iso, 4, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(4)),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(iso, iso, 16, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(16)),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(iso, iso, 64, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(64)),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(iso, iso, 4, 4, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    2 * 10 * log10(4)),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(iso, iso, 16, 16, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    2 * 10 * log10(16)),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(iso, iso, 64, 64, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    2 * 10 * log10(64)),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, iso, 1, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, iso, 4, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(4) + maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, iso, 16, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(16) + maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, iso, 64, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(64) + maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, tgpp, 1, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    2 * maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, tgpp, 4, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(4) + 2 * maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, tgpp, 16, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(16) + 2 * maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, tgpp, 64, 1, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    10 * log10(64) + 2 * maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, tgpp, 4, 4, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    2 * 10 * log10(4) + 2 * maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, tgpp, 16, 16, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    2 * 10 * log10(16) + 2 * maxTgppGain),
              TestCase::QUICK);
  AddTestCase(new ArrayResponseTest(tgpp, tgpp, 64, 64, Vector(0.0, 0.0, 0.0),
                                    Vector(5.0, 0.0, 0.0), 0.0, -M_PI,
                                    2 * 10 * log10(64) + 2 * maxTgppGain),
              TestCase::QUICK);

  AddTestCase(new OverallGainAverageTest(iso, iso, 1, 1, 10e9, "RMa"),
              TestCase::EXTENSIVE);
  AddTestCase(new OverallGainAverageTest(tgpp, tgpp, 1, 1, 10e9, "RMa"),
              TestCase::EXTENSIVE);
  AddTestCase(new OverallGainAverageTest(iso, iso, 4, 4, 10e9, "RMa"),
              TestCase::EXTENSIVE);
  AddTestCase(new OverallGainAverageTest(tgpp, tgpp, 4, 4, 10e9, "RMa"),
              TestCase::EXTENSIVE);
  AddTestCase(new OverallGainAverageTest(iso, iso, 1, 1, 10e9, "UMa"),
              TestCase::EXTENSIVE);
  AddTestCase(new OverallGainAverageTest(tgpp, tgpp, 1, 1, 10e9, "UMa"),
              TestCase::EXTENSIVE);
  AddTestCase(new OverallGainAverageTest(iso, iso, 4, 4, 10e9, "UMa"),
              TestCase::EXTENSIVE);
  AddTestCase(new OverallGainAverageTest(tgpp, tgpp, 4, 4, 10e9, "UMa"),
              TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(iso, iso, 1, 1, 60e9, "UMi-StreetCanyon"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(tgpp, tgpp, 1, 1, 60e9, "UMi-StreetCanyon"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(iso, iso, 4, 4, 60e9, "UMi-StreetCanyon"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(tgpp, tgpp, 4, 4, 60e9, "UMi-StreetCanyon"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(iso, iso, 1, 1, 60e9, "InH-OfficeOpen"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(tgpp, tgpp, 1, 1, 60e9, "InH-OfficeOpen"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(iso, iso, 4, 4, 60e9, "InH-OfficeOpen"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(tgpp, tgpp, 4, 4, 60e9, "InH-OfficeOpen"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(iso, iso, 1, 1, 100e9, "InH-OfficeMixed"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(tgpp, tgpp, 1, 1, 100e9, "InH-OfficeMixed"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(iso, iso, 4, 4, 100e9, "InH-OfficeMixed"),
      TestCase::EXTENSIVE);
  AddTestCase(
      new OverallGainAverageTest(tgpp, tgpp, 4, 4, 100e9, "InH-OfficeMixed"),
      TestCase::EXTENSIVE);
}

static TwoRaySplmTestSuite myTestSuite;
