
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/integer.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/log.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-helper.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/mobility-helper.h>
#include <ns3/point-to-point-epc-helper.h>
#include <ns3/simulator.h>
#include <ns3/test.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteSecondaryCellHandoverTest");

class LteSecondaryCellHandoverTestCase : public TestCase {
public:
  LteSecondaryCellHandoverTestCase(std::string name, bool useIdealRrc);

  void ShutdownCell(uint32_t cellId);

  void UeHandoverStartCallback(uint64_t imsi, uint16_t sourceCellId,
                               uint16_t rnti, uint16_t targetCellId);

private:
  void DoRun() override;

  void DoTeardown() override;

  bool m_useIdealRrc;
  uint8_t m_numberOfComponentCarriers;

  Ptr<LteEnbNetDevice> m_sourceEnbDev;

  bool m_hasUeHandoverStarted;
};

LteSecondaryCellHandoverTestCase::LteSecondaryCellHandoverTestCase(
    std::string name, bool useIdealRrc)
    : TestCase{name}, m_useIdealRrc{useIdealRrc},
      m_numberOfComponentCarriers{2}, m_hasUeHandoverStarted{false} {}

void LteSecondaryCellHandoverTestCase::ShutdownCell(uint32_t cellId) {
  Ptr<LteEnbPhy> phy = m_sourceEnbDev->GetPhy(cellId - 1);
  phy->SetTxPower(1);
}

void LteSecondaryCellHandoverTestCase::UeHandoverStartCallback(
    uint64_t imsi, uint16_t sourceCellId, uint16_t rnti,
    uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << imsi << sourceCellId << rnti << targetCellId);
  m_hasUeHandoverStarted = true;
}

void LteSecondaryCellHandoverTestCase::DoRun() {
  NS_LOG_FUNCTION(this << GetName());

  Config::SetDefault("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue(100));
  Config::SetDefault("ns3::LteEnbNetDevice::UlEarfcn",
                     UintegerValue(100 + 18000));
  Config::SetDefault("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue(25));
  Config::SetDefault("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue(25));
  Config::SetDefault("ns3::LteUeNetDevice::DlEarfcn", UintegerValue(100));

  auto lteHelper = CreateObject<LteHelper>();
  lteHelper->SetAttribute(
      "PathlossModel",
      TypeIdValue(ns3::FriisSpectrumPropagationLossModel::GetTypeId()));
  lteHelper->SetAttribute("UseIdealRrc", BooleanValue(m_useIdealRrc));
  lteHelper->SetAttribute("NumberOfComponentCarriers",
                          UintegerValue(m_numberOfComponentCarriers));

  lteHelper->SetHandoverAlgorithmType("ns3::A3RsrpHandoverAlgorithm");
  lteHelper->SetHandoverAlgorithmAttribute("Hysteresis", DoubleValue(1.5));
  lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",
                                           TimeValue(MilliSeconds(128)));

  auto epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  auto enbNode = CreateObject<Node>();
  auto ueNode = CreateObject<Node>();

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNode);
  mobility.Install(ueNode);

  m_sourceEnbDev =
      DynamicCast<LteEnbNetDevice>(lteHelper->InstallEnbDevice(enbNode).Get(0));
  auto ueDevs = lteHelper->InstallUeDevice(ueNode);
  auto ueDev = DynamicCast<LteUeNetDevice>(ueDevs.Get(0));

  InternetStackHelper internet;
  internet.Install(ueNode);
  epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDev));

  uint16_t sourceCellId = m_sourceEnbDev->GetCellId();
  Simulator::Schedule(Seconds(0.5),
                      &LteSecondaryCellHandoverTestCase::ShutdownCell, this,
                      sourceCellId);

  ueDev->GetRrc()->TraceConnectWithoutContext(
      "HandoverStart",
      MakeCallback(&LteSecondaryCellHandoverTestCase::UeHandoverStartCallback,
                   this));

  std::map<uint8_t, Ptr<ComponentCarrierUe>> ueCcMap = ueDev->GetCcMap();
  ueDev->SetDlEarfcn(ueCcMap.at(0)->GetDlEarfcn());
  lteHelper->Attach(ueDev, m_sourceEnbDev, 0);

  Simulator::Stop(Seconds(1));
  Simulator::Run();
  Simulator::Destroy();
}

void LteSecondaryCellHandoverTestCase::DoTeardown() {
  NS_LOG_FUNCTION(this);
  NS_TEST_ASSERT_MSG_EQ(m_hasUeHandoverStarted, true, "Handover did not occur");
}

class LteSecondaryCellHandoverTestSuite : public TestSuite {
public:
  LteSecondaryCellHandoverTestSuite();
};

LteSecondaryCellHandoverTestSuite::LteSecondaryCellHandoverTestSuite()
    : TestSuite{"lte-secondary-cell-handover", SYSTEM} {
  AddTestCase(new LteSecondaryCellHandoverTestCase("Ideal RRC", true),
              TestCase::QUICK);
  AddTestCase(new LteSecondaryCellHandoverTestCase("Real RRC", false),
              TestCase::QUICK);
}

static LteSecondaryCellHandoverTestSuite
    g_lteSecondaryCellHandoverTestSuiteInstance;
