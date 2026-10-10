
#include "ns3/abort.h"
#include "ns3/angles.h"
#include "ns3/channel-condition-model.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"
#include "ns3/ism-spectrum-value-helper.h"
#include "ns3/isotropic-antenna-model.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/pointer.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/spectrum-signal-parameters.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/uinteger.h"
#include "ns3/uniform-planar-array.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThreeGppChannelTestSuite");

class ThreeGppChannelMatrixComputationTest : public TestCase {
public:
  ThreeGppChannelMatrixComputationTest();

  ~ThreeGppChannelMatrixComputationTest() override;

private:
  void DoRun() override;

  void DoComputeNorm(Ptr<ThreeGppChannelModel> channelModel,
                     Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob,
                     Ptr<PhasedArrayModel> txAntenna,
                     Ptr<PhasedArrayModel> rxAntenna);

  std::vector<double> m_normVector;
};

ThreeGppChannelMatrixComputationTest::ThreeGppChannelMatrixComputationTest()
    : TestCase("Check the dimensions and the norm of the channel matrix") {}

ThreeGppChannelMatrixComputationTest::~ThreeGppChannelMatrixComputationTest() {}

void ThreeGppChannelMatrixComputationTest::DoComputeNorm(
    Ptr<ThreeGppChannelModel> channelModel, Ptr<MobilityModel> txMob,
    Ptr<MobilityModel> rxMob, Ptr<PhasedArrayModel> txAntenna,
    Ptr<PhasedArrayModel> rxAntenna) {
  uint64_t txAntennaElements = txAntenna->GetNumberOfElements();
  uint64_t rxAntennaElements = rxAntenna->GetNumberOfElements();

  Ptr<const ThreeGppChannelModel::ChannelMatrix> channelMatrix =
      channelModel->GetChannel(txMob, rxMob, txAntenna, rxAntenna);

  double channelNorm = 0;
  uint16_t numTotalClusters = channelMatrix->m_channel.GetNumPages();
  for (uint16_t cIndex = 0; cIndex < numTotalClusters; cIndex++) {
    double clusterNorm = 0;
    for (uint64_t sIndex = 0; sIndex < txAntennaElements; sIndex++) {
      for (uint64_t uIndex = 0; uIndex < rxAntennaElements; uIndex++) {
        clusterNorm += std::pow(
            std::abs(channelMatrix->m_channel(uIndex, sIndex, cIndex)), 2);
      }
    }
    channelNorm += clusterNorm;
  }
  m_normVector.push_back(channelNorm);
}

void ThreeGppChannelMatrixComputationTest::DoRun() {
  uint8_t txAntennaElements[]{2, 2};
  uint8_t rxAntennaElements[]{2, 2};
  uint32_t updatePeriodMs = 100;

  Ptr<ChannelConditionModel> channelConditionModel =
      CreateObject<NeverLosChannelConditionModel>();

  Ptr<ThreeGppChannelModel> channelModel = CreateObject<ThreeGppChannelModel>();
  channelModel->SetAttribute("Frequency", DoubleValue(60.0e9));
  channelModel->SetAttribute("Scenario", StringValue("RMa"));
  channelModel->SetAttribute("ChannelConditionModel",
                             PointerValue(channelConditionModel));
  channelModel->SetAttribute("UpdatePeriod",
                             TimeValue(MilliSeconds(updatePeriodMs - 1)));

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();

  nodes.Get(0)->AddDevice(txDev);
  txDev->SetNode(nodes.Get(0));
  nodes.Get(1)->AddDevice(rxDev);
  rxDev->SetNode(nodes.Get(1));

  Ptr<MobilityModel> txMob = CreateObject<ConstantPositionMobilityModel>();
  txMob->SetPosition(Vector(0.0, 0.0, 10.0));
  Ptr<MobilityModel> rxMob = CreateObject<ConstantPositionMobilityModel>();
  rxMob->SetPosition(Vector(100.0, 0.0, 10.0));

  nodes.Get(0)->AggregateObject(txMob);
  nodes.Get(1)->AggregateObject(rxMob);

  Ptr<PhasedArrayModel> txAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(txAntennaElements[0]), "NumRows",
          UintegerValue(txAntennaElements[1]), "AntennaElement",
          PointerValue(CreateObject<IsotropicAntennaModel>()));
  Ptr<PhasedArrayModel> rxAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(rxAntennaElements[0]), "NumRows",
          UintegerValue(rxAntennaElements[1]), "AntennaElement",
          PointerValue(CreateObject<IsotropicAntennaModel>()));

  Ptr<const ThreeGppChannelModel::ChannelMatrix> channelMatrix =
      channelModel->GetChannel(txMob, rxMob, txAntenna, rxAntenna);

  NS_TEST_ASSERT_MSG_EQ(channelMatrix->m_channel.GetNumCols(),
                        txAntennaElements[0] * txAntennaElements[1],
                        "The third dimension of H should be equal to the "
                        "number of tx antenna elements");
  NS_TEST_ASSERT_MSG_EQ(channelMatrix->m_channel.GetNumRows(),
                        rxAntennaElements[0] * rxAntennaElements[1],
                        "The second dimension of H should be equal to the "
                        "number of rx antenna elements");

  uint16_t numIt = 1000;
  for (uint16_t i = 0; i < numIt; i++) {
    Simulator::Schedule(MilliSeconds(updatePeriodMs * i),
                        &ThreeGppChannelMatrixComputationTest::DoComputeNorm,
                        this, channelModel, txMob, rxMob, txAntenna, rxAntenna);
  }

  Simulator::Run();

  double sampleMean = 0;
  for (auto i : m_normVector) {
    sampleMean += i;
  }
  sampleMean /= numIt;

  double sampleStd = 0;
  for (auto i : m_normVector) {
    sampleStd += ((i - sampleMean) * (i - sampleMean));
  }
  sampleStd = std::sqrt(sampleStd / (numIt - 1));

  double t = (sampleMean - txAntennaElements[0] * txAntennaElements[1] *
                               rxAntennaElements[0] * rxAntennaElements[1]) /
             (sampleMean / std::sqrt(numIt));

  NS_TEST_ASSERT_MSG_EQ_TOL(std::abs(t), 0, 1.65,
                            "We reject the hypothesis E[|H|^2] = M*N with a "
                            "significance level of 0.05");

  Simulator::Destroy();
}

class ThreeGppChannelMatrixUpdateTest : public TestCase {
public:
  ThreeGppChannelMatrixUpdateTest();

  ~ThreeGppChannelMatrixUpdateTest() override;

private:
  void DoRun() override;

  void DoGetChannel(Ptr<ThreeGppChannelModel> channelModel,
                    Ptr<MobilityModel> txMob, Ptr<MobilityModel> rxMob,
                    Ptr<PhasedArrayModel> txAntenna,
                    Ptr<PhasedArrayModel> rxAntenna, bool update);

  Ptr<const ThreeGppChannelModel::ChannelMatrix> m_currentChannel;
};

ThreeGppChannelMatrixUpdateTest::ThreeGppChannelMatrixUpdateTest()
    : TestCase("Check if the channel realizations are correctly updated during "
               "the simulation") {}

ThreeGppChannelMatrixUpdateTest::~ThreeGppChannelMatrixUpdateTest() {}

void ThreeGppChannelMatrixUpdateTest::DoGetChannel(
    Ptr<ThreeGppChannelModel> channelModel, Ptr<MobilityModel> txMob,
    Ptr<MobilityModel> rxMob, Ptr<PhasedArrayModel> txAntenna,
    Ptr<PhasedArrayModel> rxAntenna, bool update) {
  Ptr<const ThreeGppChannelModel::ChannelMatrix> channelMatrix =
      channelModel->GetChannel(txMob, rxMob, txAntenna, rxAntenna);

  if (!m_currentChannel) {
    m_currentChannel = channelMatrix;
  } else {
    NS_TEST_ASSERT_MSG_EQ(
        (m_currentChannel != channelMatrix), update,
        Simulator::Now().GetMilliSeconds()
            << " The channel matrix is not correctly updated");
  }
}

void ThreeGppChannelMatrixUpdateTest::DoRun() {

  uint8_t txAntennaElements[]{2, 2};
  uint8_t rxAntennaElements[]{4, 4};
  uint32_t updatePeriodMs = 100;

  Ptr<ChannelConditionModel> channelConditionModel =
      CreateObject<AlwaysLosChannelConditionModel>();

  Ptr<ThreeGppChannelModel> channelModel = CreateObject<ThreeGppChannelModel>();
  channelModel->SetAttribute("Frequency", DoubleValue(60.0e9));
  channelModel->SetAttribute("Scenario", StringValue("UMa"));
  channelModel->SetAttribute("ChannelConditionModel",
                             PointerValue(channelConditionModel));
  channelModel->SetAttribute("UpdatePeriod",
                             TimeValue(MilliSeconds(updatePeriodMs)));

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();

  nodes.Get(0)->AddDevice(txDev);
  txDev->SetNode(nodes.Get(0));
  nodes.Get(1)->AddDevice(rxDev);
  rxDev->SetNode(nodes.Get(1));

  Ptr<MobilityModel> txMob = CreateObject<ConstantPositionMobilityModel>();
  txMob->SetPosition(Vector(0.0, 0.0, 10.0));
  Ptr<MobilityModel> rxMob = CreateObject<ConstantPositionMobilityModel>();
  rxMob->SetPosition(Vector(100.0, 0.0, 1.6));

  nodes.Get(0)->AggregateObject(txMob);
  nodes.Get(1)->AggregateObject(rxMob);

  Ptr<PhasedArrayModel> txAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(txAntennaElements[0]), "NumRows",
          UintegerValue(txAntennaElements[1]), "AntennaElement",
          PointerValue(CreateObject<IsotropicAntennaModel>()));
  Ptr<PhasedArrayModel> rxAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(rxAntennaElements[0]), "NumRows",
          UintegerValue(rxAntennaElements[1]), "AntennaElement",
          PointerValue(CreateObject<IsotropicAntennaModel>()));

  uint32_t firstTimeMs = 1;
  Simulator::Schedule(MilliSeconds(firstTimeMs),
                      &ThreeGppChannelMatrixUpdateTest::DoGetChannel, this,
                      channelModel, txMob, rxMob, txAntenna, rxAntenna, true);

  Simulator::Schedule(MilliSeconds(firstTimeMs + updatePeriodMs / 2),
                      &ThreeGppChannelMatrixUpdateTest::DoGetChannel, this,
                      channelModel, txMob, rxMob, txAntenna, rxAntenna, false);

  Simulator::Schedule(MilliSeconds(firstTimeMs + updatePeriodMs + 1),
                      &ThreeGppChannelMatrixUpdateTest::DoGetChannel, this,
                      channelModel, txMob, rxMob, txAntenna, rxAntenna, true);

  Simulator::Run();
  Simulator::Destroy();
}

struct CheckLongTermUpdateParams {
  Ptr<ThreeGppSpectrumPropagationLossModel> lossModel;
  Ptr<SpectrumSignalParameters> txParams;
  Ptr<MobilityModel> txMob;
  Ptr<MobilityModel> rxMob;
  Ptr<SpectrumValue> rxPsdOld;
  Ptr<PhasedArrayModel> txAntenna;
  Ptr<PhasedArrayModel> rxAntenna;
};

class ThreeGppSpectrumPropagationLossModelTest : public TestCase {
public:
  ThreeGppSpectrumPropagationLossModelTest();

  ~ThreeGppSpectrumPropagationLossModelTest() override;

private:
  void DoRun() override;

  void DoBeamforming(Ptr<NetDevice> thisDevice,
                     Ptr<PhasedArrayModel> thisAntenna,
                     Ptr<NetDevice> otherDevice,
                     Ptr<PhasedArrayModel> otherAntenna);

  void CheckLongTermUpdate(const CheckLongTermUpdateParams &params);

  static bool ArePsdEqual(Ptr<SpectrumValue> first, Ptr<SpectrumValue> second);
};

ThreeGppSpectrumPropagationLossModelTest::
    ThreeGppSpectrumPropagationLossModelTest()
    : TestCase("Test case for the ThreeGppSpectrumPropagationLossModel class") {
}

ThreeGppSpectrumPropagationLossModelTest::
    ~ThreeGppSpectrumPropagationLossModelTest() {}

void ThreeGppSpectrumPropagationLossModelTest::DoBeamforming(
    Ptr<NetDevice> thisDevice, Ptr<PhasedArrayModel> thisAntenna,
    Ptr<NetDevice> otherDevice, Ptr<PhasedArrayModel> otherAntenna) {
  Vector aPos =
      thisDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();
  Vector bPos =
      otherDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();

  Angles completeAngle(bPos, aPos);

  PhasedArrayModel::ComplexVector antennaWeights =
      thisAntenna->GetBeamformingVector(completeAngle);
  thisAntenna->SetBeamformingVector(antennaWeights);
}

bool ThreeGppSpectrumPropagationLossModelTest::ArePsdEqual(
    Ptr<SpectrumValue> first, Ptr<SpectrumValue> second) {
  bool ret = true;
  for (uint8_t i = 0; i < first->GetSpectrumModel()->GetNumBands(); i++) {
    if ((*first)[i] != (*second)[i]) {
      ret = false;
      continue;
    }
  }
  return ret;
}

void ThreeGppSpectrumPropagationLossModelTest::CheckLongTermUpdate(
    const CheckLongTermUpdateParams &params) {
  Ptr<SpectrumValue> rxPsdNew = params.lossModel->DoCalcRxPowerSpectralDensity(
      params.txParams, params.txMob, params.rxMob, params.txAntenna,
      params.rxAntenna);
  NS_TEST_ASSERT_MSG_EQ(
      ArePsdEqual(params.rxPsdOld, rxPsdNew), false,
      "The long term is not updated when the channel matrix is recomputed");
}

void ThreeGppSpectrumPropagationLossModelTest::DoRun() {
  Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod",
                     TimeValue(MilliSeconds(100)));

  uint8_t txAntennaElements[]{4, 4};
  uint8_t rxAntennaElements[]{4, 4};

  Ptr<ChannelConditionModel> condModel =
      CreateObject<AlwaysLosChannelConditionModel>();

  Ptr<ThreeGppSpectrumPropagationLossModel> lossModel =
      CreateObject<ThreeGppSpectrumPropagationLossModel>();
  lossModel->SetChannelModelAttribute("Frequency", DoubleValue(2.4e9));
  lossModel->SetChannelModelAttribute("Scenario", StringValue("UMa"));
  lossModel->SetChannelModelAttribute("ChannelConditionModel",
                                      PointerValue(condModel));

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();

  nodes.Get(0)->AddDevice(txDev);
  txDev->SetNode(nodes.Get(0));
  nodes.Get(1)->AddDevice(rxDev);
  rxDev->SetNode(nodes.Get(1));

  Ptr<MobilityModel> txMob = CreateObject<ConstantPositionMobilityModel>();
  txMob->SetPosition(Vector(0.0, 0.0, 10.0));
  Ptr<MobilityModel> rxMob = CreateObject<ConstantPositionMobilityModel>();
  rxMob->SetPosition(Vector(15.0, 0.0, 10.0));

  nodes.Get(0)->AggregateObject(txMob);
  nodes.Get(1)->AggregateObject(rxMob);

  Ptr<PhasedArrayModel> txAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(txAntennaElements[0]), "NumRows",
          UintegerValue(txAntennaElements[1]), "AntennaElement",
          PointerValue(CreateObject<IsotropicAntennaModel>()));
  Ptr<PhasedArrayModel> rxAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(rxAntennaElements[0]), "NumRows",
          UintegerValue(rxAntennaElements[1]), "AntennaElement",
          PointerValue(CreateObject<IsotropicAntennaModel>()));

  DoBeamforming(txDev, txAntenna, rxDev, rxAntenna);
  DoBeamforming(rxDev, rxAntenna, txDev, txAntenna);

  SpectrumValue5MhzFactory sf;
  double txPower = 0.1;
  uint32_t channelNumber = 1;
  Ptr<SpectrumValue> txPsd =
      sf.CreateTxPowerSpectralDensity(txPower, channelNumber);
  Ptr<SpectrumSignalParameters> txParams = Create<SpectrumSignalParameters>();
  txParams->psd = txPsd->Copy();

  Ptr<SpectrumValue> rxPsdOld = lossModel->DoCalcRxPowerSpectralDensity(
      txParams, txMob, rxMob, txAntenna, rxAntenna);

  Ptr<SpectrumValue> rxPsdNew = lossModel->DoCalcRxPowerSpectralDensity(
      txParams, rxMob, txMob, rxAntenna, txAntenna);
  NS_TEST_ASSERT_MSG_EQ(
      ArePsdEqual(rxPsdOld, rxPsdNew), true,
      "The long term for the direct and the reverse channel are different");

  rxMob->SetPosition(Vector(10.0, 5.0, 10.0));
  PhasedArrayModel::ComplexVector txBfVector =
      txAntenna->GetBeamformingVector();
  txBfVector[0] = std::complex<double>(0.0, 0.0);
  txAntenna->SetBeamformingVector(txBfVector);

  rxPsdNew = lossModel->DoCalcRxPowerSpectralDensity(txParams, rxMob, txMob,
                                                     rxAntenna, txAntenna);
  NS_TEST_ASSERT_MSG_EQ(ArePsdEqual(rxPsdOld, rxPsdNew), false,
                        "Changing the BF vectors the rx PSD does not change");

  rxPsdOld = rxPsdNew;

  CheckLongTermUpdateParams params{lossModel, txParams,  txMob,    rxMob,
                                   rxPsdOld,  txAntenna, rxAntenna};
  Simulator::Schedule(
      MilliSeconds(101),
      &ThreeGppSpectrumPropagationLossModelTest::CheckLongTermUpdate, this,
      params);

  Simulator::Run();
  Simulator::Destroy();
}

class ThreeGppChannelTestSuite : public TestSuite {
public:
  ThreeGppChannelTestSuite();
};

ThreeGppChannelTestSuite::ThreeGppChannelTestSuite()
    : TestSuite("three-gpp-channel", UNIT) {
  AddTestCase(new ThreeGppChannelMatrixComputationTest, TestCase::QUICK);
  AddTestCase(new ThreeGppChannelMatrixUpdateTest, TestCase::QUICK);
  AddTestCase(new ThreeGppSpectrumPropagationLossModelTest, TestCase::QUICK);
}

static ThreeGppChannelTestSuite myTestSuite;
