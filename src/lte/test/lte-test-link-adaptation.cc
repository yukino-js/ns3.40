
#include "lte-test-link-adaptation.h"

#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/lte-ue-phy.h"
#include "ns3/mobility-helper.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include <ns3/enum.h>
#include <ns3/lte-chunk-processor.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteLinkAdaptationTest");

void LteTestDlSchedulingCallback(LteLinkAdaptationTestCase *testcase,
                                 std::string path,
                                 DlSchedulingCallbackInfo dlInfo) {
  testcase->DlScheduling(dlInfo);
}

LteLinkAdaptationTestSuite::LteLinkAdaptationTestSuite()
    : TestSuite("lte-link-adaptation", SYSTEM) {
  NS_LOG_INFO("Creating LteLinkAdaptionTestSuite");

  struct SnrEfficiencyMcs {
    double snrDb;
    double efficiency;
    int mcsIndex;
  };

  SnrEfficiencyMcs snrEfficiencyMcs[] = {
      {-5.00000, 0.08024, -1}, {-4.00000, 0.10030, -1}, {-3.00000, 0.12518, -1},
      {-2.00000, 0.15589, 0},  {-1.00000, 0.19365, 0},  {0.00000, 0.23983, 2},
      {1.00000, 0.29593, 2},   {2.00000, 0.36360, 2},   {3.00000, 0.44451, 4},
      {4.00000, 0.54031, 4},   {5.00000, 0.65251, 6},   {6.00000, 0.78240, 6},
      {7.00000, 0.93086, 8},   {8.00000, 1.09835, 8},   {9.00000, 1.28485, 10},
      {10.00000, 1.48981, 12}, {11.00000, 1.71229, 12}, {12.00000, 1.95096, 14},
      {13.00000, 2.20429, 14}, {14.00000, 2.47062, 16}, {15.00000, 2.74826, 18},
      {16.00000, 3.03560, 18}, {17.00000, 3.33115, 20}, {18.00000, 3.63355, 20},
      {19.00000, 3.94163, 22}, {20.00000, 4.25439, 22}, {21.00000, 4.57095, 24},
      {22.00000, 4.89060, 24}, {23.00000, 5.21276, 26}, {24.00000, 5.53693, 26},
      {25.00000, 5.86271, 28}, {26.00000, 6.18980, 28}, {27.00000, 6.51792, 28},
      {28.00000, 6.84687, 28}, {29.00000, 7.17649, 28}, {30.00000, 7.50663, 28},
  };
  int numOfTests = sizeof(snrEfficiencyMcs) / sizeof(SnrEfficiencyMcs);

  double txPowerDbm = 30;
  double ktDbm = -174;
  double noisePowerDbm = ktDbm + 10 * std::log10(25 * 180000);
  double receiverNoiseFigureDb = 9.0;

  for (int i = 0; i < numOfTests; i++) {
    double lossDb = txPowerDbm - snrEfficiencyMcs[i].snrDb - noisePowerDbm -
                    receiverNoiseFigureDb;

    std::ostringstream name;
    name << " snr= " << snrEfficiencyMcs[i].snrDb << " dB, "
         << " mcs= " << snrEfficiencyMcs[i].mcsIndex;
    AddTestCase(new LteLinkAdaptationTestCase(name.str(),
                                              snrEfficiencyMcs[i].snrDb, lossDb,
                                              snrEfficiencyMcs[i].mcsIndex),
                TestCase::QUICK);
  }
}

static LteLinkAdaptationTestSuite lteLinkAdaptationTestSuite;

LteLinkAdaptationTestCase::LteLinkAdaptationTestCase(std::string name,
                                                     double snrDb, double loss,
                                                     uint16_t mcsIndex)
    : TestCase(name), m_snrDb(snrDb), m_loss(loss), m_mcsIndex(mcsIndex) {
  std::ostringstream sstream1;
  std::ostringstream sstream2;
  sstream1 << " snr=" << snrDb << " mcs=" << mcsIndex;

  NS_LOG_INFO("Creating LteLinkAdaptationTestCase: " + sstream1.str());
}

LteLinkAdaptationTestCase::~LteLinkAdaptationTestCase() {}

void LteLinkAdaptationTestCase::DoRun() {
  Config::Reset();
  Config::SetDefault("ns3::LteAmc::AmcModel", EnumValue(LteAmc::PiroEW2010));
  Config::SetDefault("ns3::LteAmc::Ber", DoubleValue(0.00005));
  Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(2));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  Config::SetDefault("ns3::MacStatsCalculator::DlOutputFilename",
                     StringValue(CreateTempDirFilename("DlMacStats.txt")));
  Config::SetDefault("ns3::MacStatsCalculator::UlOutputFilename",
                     StringValue(CreateTempDirFilename("UlMacStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("DlRlcStats.txt")));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename",
                     StringValue(CreateTempDirFilename("UlRlcStats.txt")));

  Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl",
                     BooleanValue(false));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  lteHelper->SetAttribute(
      "PathlossModel",
      StringValue("ns3::ConstantSpectrumPropagationLossModel"));
  NS_LOG_INFO("SNR = " << m_snrDb << "  LOSS = " << m_loss);
  lteHelper->SetPathlossModelAttribute("Loss", DoubleValue(m_loss));

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);
  NodeContainer allNodes = NodeContainer(enbNodes, ueNodes);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(allNodes);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  EpsBearer::Qci q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
  EpsBearer bearer(q);
  lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  Ptr<LtePhy> uePhy =
      ueDevs.Get(0)->GetObject<LteUeNetDevice>()->GetPhy()->GetObject<LtePhy>();
  Ptr<LteChunkProcessor> testSinr = Create<LteChunkProcessor>();
  LteSpectrumValueCatcher sinrCatcher;
  testSinr->AddCallback(
      MakeCallback(&LteSpectrumValueCatcher::ReportValue, &sinrCatcher));
  uePhy->GetDownlinkSpectrumPhy()->AddCtrlSinrChunkProcessor(testSinr);

  Config::Connect(
      "/NodeList/0/DeviceList/0/ComponentCarrierMap/*/LteEnbMac/DlScheduling",
      MakeBoundCallback(&LteTestDlSchedulingCallback, this));

  lteHelper->EnableMacTraces();
  lteHelper->EnableRlcTraces();

  Simulator::Stop(Seconds(0.040));
  Simulator::Run();

  double calculatedSinrDb =
      10.0 * std::log10(sinrCatcher.GetValue()->operator[](0));
  NS_TEST_ASSERT_MSG_EQ_TOL(calculatedSinrDb, m_snrDb, 0.0000001,
                            "Wrong SINR !");
  Simulator::Destroy();
}

void LteLinkAdaptationTestCase::DlScheduling(DlSchedulingCallbackInfo dlInfo) {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;
    NS_LOG_INFO("SNR\tRef_MCS\tCalc_MCS");
  }

  if (Simulator::Now().GetSeconds() > 0.030) {
    NS_LOG_INFO(m_snrDb << "\t" << m_mcsIndex << "\t"
                        << (uint16_t)dlInfo.mcsTb1);

    NS_TEST_ASSERT_MSG_EQ((uint16_t)dlInfo.mcsTb1, m_mcsIndex,
                          "Wrong MCS index");
  }
}
