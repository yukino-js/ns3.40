
#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/config.h>
#include <ns3/data-rate.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-static-routing.h>
#include <ns3/log.h>
#include <ns3/lte-helper.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/point-to-point-epc-helper.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/position-allocator.h>
#include <ns3/rng-seed-manager.h>
#include <ns3/simulator.h>
#include <ns3/test.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteHandoverFailureTest");

class LteHandoverFailureTestCase : public TestCase {
public:
  LteHandoverFailureTestCase(std::string name, bool useIdealRrc,
                             Time handoverTime, Time simulationDuration,
                             uint8_t numberOfRaPreambles,
                             uint8_t preambleTransMax,
                             uint8_t raResponseWindowSize,
                             Time handoverJoiningTimeout,
                             Time handoverLeavingTimeout,
                             uint16_t targeteNodeBPosition)
      : TestCase(name), m_useIdealRrc(useIdealRrc),
        m_handoverTime(handoverTime), m_simulationDuration(simulationDuration),
        m_numberOfRaPreambles(numberOfRaPreambles),
        m_preambleTransMax(preambleTransMax),
        m_raResponseWindowSize(raResponseWindowSize),
        m_handoverJoiningTimeout(handoverJoiningTimeout),
        m_handoverLeavingTimeout(handoverLeavingTimeout),
        m_targeteNodeBPosition(targeteNodeBPosition),
        m_hasHandoverFailureOccurred(false) {}

private:
  void DoRun() override;

  void DoTeardown() override;

  void UeHandoverStartCallback(std::string context, uint64_t imsi,
                               uint16_t sourceCellId, uint16_t rnti,
                               uint16_t targetCellId);

  void HandoverFailureMaxRach(std::string context, uint64_t imsi, uint16_t rnti,
                              uint16_t targetCellId);

  void HandoverFailureNoPreamble(std::string context, uint64_t imsi,
                                 uint16_t rnti, uint16_t targetCellId);

  void HandoverFailureJoining(std::string context, uint64_t imsi, uint16_t rnti,
                              uint16_t targetCellId);

  void HandoverFailureLeaving(std::string context, uint64_t imsi, uint16_t rnti,
                              uint16_t targetCellId);

  bool m_useIdealRrc;
  Time m_handoverTime;
  Time m_simulationDuration;
  uint8_t m_numberOfRaPreambles;
  uint8_t m_preambleTransMax;
  uint8_t m_raResponseWindowSize;
  Time m_handoverJoiningTimeout;
  Time m_handoverLeavingTimeout;
  uint16_t m_targeteNodeBPosition;
  bool m_hasHandoverFailureOccurred;
};

void LteHandoverFailureTestCase::DoRun() {
  NS_LOG_INFO(this << " " << GetName());
  uint32_t previousSeed = RngSeedManager::GetSeed();
  uint64_t previousRun = RngSeedManager::GetRun();
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(2);

  auto epcHelper = CreateObject<PointToPointEpcHelper>();

  auto lteHelper = CreateObject<LteHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  lteHelper->SetAttribute("UseIdealRrc", BooleanValue(m_useIdealRrc));
  Config::SetDefault("ns3::LteEnbMac::NumberOfRaPreambles",
                     UintegerValue(m_numberOfRaPreambles));
  Config::SetDefault("ns3::LteEnbMac::PreambleTransMax",
                     UintegerValue(m_preambleTransMax));
  Config::SetDefault("ns3::LteEnbMac::RaResponseWindowSize",
                     UintegerValue(m_raResponseWindowSize));
  Config::SetDefault("ns3::LteEnbRrc::HandoverJoiningTimeoutDuration",
                     TimeValue(m_handoverJoiningTimeout));
  Config::SetDefault("ns3::LteEnbRrc::HandoverLeavingTimeoutDuration",
                     TimeValue(m_handoverLeavingTimeout));

  lteHelper->SetPathlossModelType(
      TypeId::LookupByName("ns3::LogDistancePropagationLossModel"));
  lteHelper->SetPathlossModelAttribute("Exponent", DoubleValue(3.5));
  lteHelper->SetPathlossModelAttribute("ReferenceLoss", DoubleValue(35));
  NodeContainer enbNodes;
  enbNodes.Create(2);
  auto ueNode = CreateObject<Node>();

  auto posAlloc = CreateObject<ListPositionAllocator>();
  posAlloc->Add(Vector(0, 0, 0));
  posAlloc->Add(Vector(m_targeteNodeBPosition, 0, 0));
  posAlloc->Add(Vector(200, 0, 0));

  MobilityHelper mobilityHelper;
  mobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityHelper.SetPositionAllocator(posAlloc);
  mobilityHelper.Install(enbNodes);
  mobilityHelper.Install(ueNode);

  auto enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  auto ueDev = lteHelper->InstallUeDevice(ueNode).Get(0);

  InternetStackHelper inetStackHelper;
  inetStackHelper.Install(ueNode);
  Ipv4InterfaceContainer ueIfs;
  ueIfs = epcHelper->AssignUeIpv4Address(ueDev);

  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",
      MakeCallback(&LteHandoverFailureTestCase::UeHandoverStartCallback, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureMaxRach",
      MakeCallback(&LteHandoverFailureTestCase::HandoverFailureMaxRach, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureNoPreamble",
      MakeCallback(&LteHandoverFailureTestCase::HandoverFailureNoPreamble,
                   this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureJoining",
      MakeCallback(&LteHandoverFailureTestCase::HandoverFailureJoining, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailureLeaving",
      MakeCallback(&LteHandoverFailureTestCase::HandoverFailureLeaving, this));

  lteHelper->AddX2Interface(enbNodes);
  lteHelper->Attach(ueDev, enbDevs.Get(0));
  lteHelper->HandoverRequest(m_handoverTime, ueDev, enbDevs.Get(0),
                             enbDevs.Get(1));

  Simulator::Stop(m_simulationDuration);
  Simulator::Run();
  Simulator::Destroy();

  RngSeedManager::SetSeed(previousSeed);
  RngSeedManager::SetRun(previousRun);
}

void LteHandoverFailureTestCase::UeHandoverStartCallback(
    std::string context, uint64_t imsi, uint16_t sourceCellId, uint16_t rnti,
    uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << " " << context << " IMSI-" << imsi << " sourceCellID-"
                       << sourceCellId << " RNTI-" << rnti << " targetCellID-"
                       << targetCellId);
  NS_LOG_INFO("HANDOVER COMMAND received through at UE "
              << imsi << " to handover from " << sourceCellId << " to "
              << targetCellId);
}

void LteHandoverFailureTestCase::HandoverFailureMaxRach(std::string context,
                                                        uint64_t imsi,
                                                        uint16_t rnti,
                                                        uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << context << imsi << rnti << targetCellId);
  m_hasHandoverFailureOccurred = true;
}

void LteHandoverFailureTestCase::HandoverFailureNoPreamble(
    std::string context, uint64_t imsi, uint16_t rnti, uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << context << imsi << rnti << targetCellId);
  m_hasHandoverFailureOccurred = true;
}

void LteHandoverFailureTestCase::HandoverFailureJoining(std::string context,
                                                        uint64_t imsi,
                                                        uint16_t rnti,
                                                        uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << context << imsi << rnti << targetCellId);
  m_hasHandoverFailureOccurred = true;
}

void LteHandoverFailureTestCase::HandoverFailureLeaving(std::string context,
                                                        uint64_t imsi,
                                                        uint16_t rnti,
                                                        uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << context << imsi << rnti << targetCellId);
  m_hasHandoverFailureOccurred = true;
}

void LteHandoverFailureTestCase::DoTeardown() {
  NS_LOG_FUNCTION(this);
  NS_TEST_ASSERT_MSG_EQ(m_hasHandoverFailureOccurred, true,
                        "Handover failure did not occur");
}

static class LteHandoverFailureTestSuite : public TestSuite {
public:
  LteHandoverFailureTestSuite()
      : TestSuite("lte-handover-failure", TestSuite::SYSTEM) {

    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to maximum RACH "
                    "transmissions reached from UE to target eNodeB",
                    false, Seconds(0.200), Seconds(0.300), 52, 3, 3,
                    MilliSeconds(200), MilliSeconds(500), 2500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to non-allocation of "
                    "non-contention preamble at "
                    "target eNodeB due to max number reached",
                    false, Seconds(0.100), Seconds(0.200), 64, 50, 3,
                    MilliSeconds(200), MilliSeconds(500), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to HANDOVER JOINING timeout "
                    "before reception of "
                    "RRC CONNECTION RECONFIGURATION at source eNodeB",
                    false, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(0), MilliSeconds(500), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to HANDOVER JOINING timeout "
                    "before completion "
                    "of non-contention RACH process to target eNodeB",
                    false, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(15), MilliSeconds(500), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to HANDOVER JOINING timeout "
                    "before reception of "
                    "RRC CONNECTION RECONFIGURATION COMPLETE at target eNodeB",
                    false, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(18), MilliSeconds(500), 500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to HANDOVER LEAVING timeout "
                    "before reception of "
                    "RRC CONNECTION RECONFIGURATION at source eNodeB",
                    false, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(200), MilliSeconds(0), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to HANDOVER LEAVING timeout "
                    "before completion "
                    "of non-contention RACH process to target eNodeB",
                    false, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(200), MilliSeconds(15), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "REAL Handover failure due to HANDOVER LEAVING timeout "
                    "before reception of "
                    "RRC CONNECTION RECONFIGURATION COMPLETE at target eNodeB",
                    false, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(200), MilliSeconds(18), 500),
                TestCase::QUICK);

    AddTestCase(new LteHandoverFailureTestCase(
                    "IDEAL Handover failure due to maximum RACH "
                    "transmissions reached from UE to target eNodeB",
                    true, Seconds(0.100), Seconds(0.200), 52, 3, 3,
                    MilliSeconds(200), MilliSeconds(500), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "IDEAL Handover failure due to non-allocation of "
                    "non-contention preamble "
                    "at target eNodeB due to max number reached",
                    true, Seconds(0.100), Seconds(0.200), 64, 50, 3,
                    MilliSeconds(200), MilliSeconds(500), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "IDEAL Handover failure due to HANDOVER JOINING timeout "
                    "before reception "
                    "of RRC CONNECTION RECONFIGURATION at source eNodeB",
                    true, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(0), MilliSeconds(500), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "IDEAL Handover failure due to HANDOVER JOINING timeout "
                    "before completion "
                    "of non-contention RACH process to target eNodeB",
                    true, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(10), MilliSeconds(500), 1500),
                TestCase::QUICK);
    AddTestCase(
        new LteHandoverFailureTestCase(
            "IDEAL Handover failure due to HANDOVER JOINING timeout before "
            "reception "
            "of RRC CONNECTION RECONFIGURATION COMPLETE at target eNodeB",
            true, Seconds(0.100), Seconds(0.200), 52, 50, 3, MilliSeconds(4),
            MilliSeconds(500), 500),
        TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "IDEAL Handover failure due to HANDOVER LEAVING timeout "
                    "before reception "
                    "of RRC CONNECTION RECONFIGURATION at source eNodeB",
                    true, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(500), MilliSeconds(0), 1500),
                TestCase::QUICK);
    AddTestCase(new LteHandoverFailureTestCase(
                    "IDEAL Handover failure due to HANDOVER LEAVING timeout "
                    "before completion "
                    "of non-contention RACH process to target eNodeB",
                    true, Seconds(0.100), Seconds(0.200), 52, 50, 3,
                    MilliSeconds(500), MilliSeconds(10), 1500),
                TestCase::QUICK);
    AddTestCase(
        new LteHandoverFailureTestCase(
            "IDEAL Handover failure due to HANDOVER LEAVING timeout before "
            "reception "
            "of RRC CONNECTION RECONFIGURATION COMPLETE at target eNodeB",
            true, Seconds(0.100), Seconds(0.200), 52, 50, 3, MilliSeconds(500),
            MilliSeconds(4), 500),
        TestCase::QUICK);
  }
} g_lteHandoverFailureTestSuite;
