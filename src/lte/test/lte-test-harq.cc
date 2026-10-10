
#include "lte-test-harq.h"

#include <ns3/boolean.h>
#include <ns3/buildings-helper.h>
#include <ns3/config.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/eps-bearer.h>
#include <ns3/ff-mac-scheduler.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>
#include <ns3/log.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-enb-phy.h>
#include <ns3/lte-helper.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/lte-ue-phy.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/mobility-building-info.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/radio-bearer-stats-calculator.h>
#include <ns3/simulator.h>
#include <ns3/spectrum-error-model.h>
#include <ns3/spectrum-interference.h>
#include <ns3/string.h>
#include <ns3/test.h>

#include <cmath>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaTestHarq");

LenaTestHarqSuite::LenaTestHarqSuite() : TestSuite("lte-harq", SYSTEM) {
  NS_LOG_INFO("creating LenaTestHarqTestCase");

  AddTestCase(new LenaHarqTestCase(2, 2400, 66, 0.12, 31822), TestCase::QUICK);

  AddTestCase(new LenaHarqTestCase(1, 770, 472, 0.06, 209964), TestCase::QUICK);
}

static LenaTestHarqSuite lenaTestHarqSuite;

std::string LenaHarqTestCase::BuildNameString(uint16_t nUser, uint16_t dist,
                                              uint16_t tbSize) {
  std::ostringstream oss;
  oss << nUser << " UEs, distance " << dist << " m, TB size " << tbSize;
  return oss.str();
}

LenaHarqTestCase::LenaHarqTestCase(uint16_t nUser, uint16_t dist,
                                   uint16_t tbSize, double amcBer,
                                   double thrRef)
    : TestCase(BuildNameString(nUser, dist, tbSize)), m_nUser(nUser),
      m_dist(dist), m_amcBer(amcBer), m_throughputRef(thrRef) {}

LenaHarqTestCase::~LenaHarqTestCase() {}

void LenaHarqTestCase::DoRun() {
  Config::SetDefault("ns3::LteAmc::Ber", DoubleValue(m_amcBer));
  Config::SetDefault("ns3::LteAmc::AmcModel", EnumValue(LteAmc::PiroEW2010));
  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                     BooleanValue(false));
  Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                     BooleanValue(true));
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

  Ptr<LteHelper> lena = CreateObject<LteHelper>();

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(m_nUser);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  BuildingsHelper::Install(enbNodes);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);
  BuildingsHelper::Install(ueNodes);

  lena->SetAttribute("PathlossModel",
                     StringValue("ns3::HybridBuildingsPropagationLossModel"));
  lena->SetPathlossModelAttribute("ShadowSigmaOutdoor", DoubleValue(0.0));
  lena->SetPathlossModelAttribute("ShadowSigmaIndoor", DoubleValue(0.0));
  lena->SetPathlossModelAttribute("ShadowSigmaExtWalls", DoubleValue(0.0));

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lena->SetSchedulerType("ns3::RrFfMacScheduler");
  lena->SetSchedulerAttribute("UlCqiFilter",
                              EnumValue(FfMacScheduler::PUSCH_UL_CQI));

  enbDevs = lena->InstallEnbDevice(enbNodes);
  ueDevs = lena->InstallUeDevice(ueNodes);

  lena->Attach(ueDevs, enbDevs.Get(0));

  EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer(q);
  lena->ActivateDataRadioBearer(ueDevs, bearer);

  Ptr<LteEnbNetDevice> lteEnbDev = enbDevs.Get(0)->GetObject<LteEnbNetDevice>();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy();
  enbPhy->SetAttribute("TxPower", DoubleValue(43.0));
  enbPhy->SetAttribute("NoiseFigure", DoubleValue(5.0));
  Ptr<MobilityModel> mm = enbNodes.Get(0)->GetObject<MobilityModel>();
  mm->SetPosition(Vector(0.0, 0.0, 30.0));

  for (int i = 0; i < m_nUser; i++) {
    Ptr<MobilityModel> mm = ueNodes.Get(i)->GetObject<MobilityModel>();
    mm->SetPosition(Vector(m_dist, 0.0, 1.0));
    Ptr<LteUeNetDevice> lteUeDev = ueDevs.Get(i)->GetObject<LteUeNetDevice>();
    Ptr<LteUePhy> uePhy = lteUeDev->GetPhy();
    uePhy->SetAttribute("TxPower", DoubleValue(23.0));
    uePhy->SetAttribute("NoiseFigure", DoubleValue(9.0));
  }

  double statsStartTime = 0.050;
  double statsDuration = 2.0;
  Simulator::Stop(Seconds(statsStartTime + statsDuration - 0.0001));

  lena->EnableRlcTraces();
  Ptr<RadioBearerStatsCalculator> rlcStats = lena->GetRlcStats();
  rlcStats->SetAttribute("StartTime", TimeValue(Seconds(statsStartTime)));
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(statsDuration)));

  lena->EnableMacTraces();

  Simulator::Run();

  NS_LOG_INFO("\tTest on downlink data shared channels (PDSCH)");
  NS_LOG_INFO("Test with " << m_nUser << " user(s) at distance " << m_dist
                           << " expected Thr " << m_throughputRef);
  for (int i = 0; i < m_nUser; i++) {
    uint64_t imsi = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
    uint8_t lcId = 3;
    double txed = rlcStats->GetDlTxData(imsi, lcId);
    double rxed = rlcStats->GetDlRxData(imsi, lcId);
    double tolerance = 0.1;

    NS_LOG_INFO(" User " << i << " imsi " << imsi << " bytes rxed/t "
                         << rxed / statsDuration << " txed/t "
                         << txed / statsDuration << " thr Ref "
                         << m_throughputRef << " Err "
                         << (std::abs(txed / statsDuration - m_throughputRef)) /
                                m_throughputRef);

    NS_TEST_ASSERT_MSG_EQ_TOL(txed / statsDuration, m_throughputRef,
                              m_throughputRef * tolerance,
                              " Unexpected Throughput!");
    NS_TEST_ASSERT_MSG_EQ_TOL(rxed / statsDuration, m_throughputRef,
                              m_throughputRef * tolerance,
                              " Unexpected Throughput!");
  }

  Simulator::Destroy();
}
