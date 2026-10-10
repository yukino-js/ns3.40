
#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/cc-helper.h>
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
#include <ns3/simulator.h>
#include <ns3/test.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteHandoverDelayTest");

class LteHandoverDelayTestCase : public TestCase {
public:
  LteHandoverDelayTestCase(uint8_t numberOfComponentCarriers, bool useIdealRrc,
                           Time handoverTime, Time delayThreshold,
                           Time simulationDuration)
      : TestCase("Verifying that the time needed for handover is under a "
                 "specified threshold"),
        m_numberOfComponentCarriers(numberOfComponentCarriers),
        m_useIdealRrc(useIdealRrc), m_handoverTime(handoverTime),
        m_delayThreshold(delayThreshold),
        m_simulationDuration(simulationDuration), m_ueHandoverStart(Seconds(0)),
        m_enbHandoverStart(Seconds(0)) {}

private:
  void DoRun() override;

  void UeHandoverStartCallback(std::string context, uint64_t imsi,
                               uint16_t cellid, uint16_t rnti,
                               uint16_t targetCellId);
  void UeHandoverEndOkCallback(std::string context, uint64_t imsi,
                               uint16_t cellid, uint16_t rnti);
  void EnbHandoverStartCallback(std::string context, uint64_t imsi,
                                uint16_t cellid, uint16_t rnti,
                                uint16_t targetCellId);
  void EnbHandoverEndOkCallback(std::string context, uint64_t imsi,
                                uint16_t cellid, uint16_t rnti);

  uint8_t m_numberOfComponentCarriers;
  bool m_useIdealRrc;
  Time m_handoverTime;
  Time m_delayThreshold;
  Time m_simulationDuration;

  Time m_ueHandoverStart;
  Time m_enbHandoverStart;
};

void LteHandoverDelayTestCase::DoRun() {
  NS_LOG_INFO("-----test case: ideal RRC = "
              << m_useIdealRrc
              << " handover time = " << m_handoverTime.As(Time::S) << "-----");

  auto epcHelper = CreateObject<PointToPointEpcHelper>();

  auto lteHelper = CreateObject<LteHelper>();
  lteHelper->SetEpcHelper(epcHelper);
  lteHelper->SetAttribute("UseIdealRrc", BooleanValue(m_useIdealRrc));
  lteHelper->SetAttribute("NumberOfComponentCarriers",
                          UintegerValue(m_numberOfComponentCarriers));

  auto ccHelper = CreateObject<CcHelper>();
  ccHelper->SetUlEarfcn(100 + 18000);
  ccHelper->SetDlEarfcn(100);
  ccHelper->SetUlBandwidth(25);
  ccHelper->SetDlBandwidth(25);
  ccHelper->SetNumberOfComponentCarriers(m_numberOfComponentCarriers);

  NodeContainer enbNodes;
  enbNodes.Create(2);
  auto ueNode = CreateObject<Node>();

  auto posAlloc = CreateObject<ListPositionAllocator>();
  posAlloc->Add(Vector(0, 0, 0));
  posAlloc->Add(Vector(1000, 0, 0));
  posAlloc->Add(Vector(500, 0, 0));

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
      MakeCallback(&LteHandoverDelayTestCase::UeHandoverStartCallback, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",
      MakeCallback(&LteHandoverDelayTestCase::UeHandoverEndOkCallback, this));

  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
      MakeCallback(&LteHandoverDelayTestCase::EnbHandoverStartCallback, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
      MakeCallback(&LteHandoverDelayTestCase::EnbHandoverEndOkCallback, this));

  lteHelper->AddX2Interface(enbNodes);
  lteHelper->Attach(ueDev, enbDevs.Get(0));
  lteHelper->HandoverRequest(m_handoverTime, ueDev, enbDevs.Get(0),
                             enbDevs.Get(1));

  Simulator::Stop(m_simulationDuration);
  Simulator::Run();
  Simulator::Destroy();
}

void LteHandoverDelayTestCase::UeHandoverStartCallback(std::string context,
                                                       uint64_t imsi,
                                                       uint16_t cellid,
                                                       uint16_t rnti,
                                                       uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << context);
  m_ueHandoverStart = Simulator::Now();
}

void LteHandoverDelayTestCase::UeHandoverEndOkCallback(std::string context,
                                                       uint64_t imsi,
                                                       uint16_t cellid,
                                                       uint16_t rnti) {
  NS_LOG_FUNCTION(this << context);
  NS_ASSERT(m_ueHandoverStart > Seconds(0));
  Time delay = Simulator::Now() - m_ueHandoverStart;

  NS_LOG_DEBUG(this << " UE delay = " << delay.As(Time::S));
  NS_TEST_ASSERT_MSG_LT(
      delay, m_delayThreshold,
      "UE handover delay is higher than the allowed threshold "
          << "(ideal RRC = " << m_useIdealRrc
          << " handover time = " << m_handoverTime.As(Time::S) << ")");
}

void LteHandoverDelayTestCase::EnbHandoverStartCallback(std::string context,
                                                        uint64_t imsi,
                                                        uint16_t cellid,
                                                        uint16_t rnti,
                                                        uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << context);
  m_enbHandoverStart = Simulator::Now();
}

void LteHandoverDelayTestCase::EnbHandoverEndOkCallback(std::string context,
                                                        uint64_t imsi,
                                                        uint16_t cellid,
                                                        uint16_t rnti) {
  NS_LOG_FUNCTION(this << context);
  NS_ASSERT(m_enbHandoverStart > Seconds(0));
  Time delay = Simulator::Now() - m_enbHandoverStart;

  NS_LOG_DEBUG(this << " eNodeB delay = " << delay.As(Time::S));
  NS_TEST_ASSERT_MSG_LT(
      delay, m_delayThreshold,
      "eNodeB handover delay is higher than the allowed threshold "
          << "(ideal RRC = " << m_useIdealRrc
          << " handover time = " << m_handoverTime.As(Time::S) << ")");
}

static class LteHandoverDelayTestSuite : public TestSuite {
public:
  LteHandoverDelayTestSuite()
      : TestSuite("lte-handover-delay", TestSuite::SYSTEM) {

    for (Time handoverTime = Seconds(0.100); handoverTime < Seconds(0.110);
         handoverTime += Seconds(0.001)) {
      AddTestCase(new LteHandoverDelayTestCase(1, true, handoverTime,
                                               Seconds(0.005), Seconds(0.200)),
                  TestCase::QUICK);
      AddTestCase(new LteHandoverDelayTestCase(2, true, handoverTime,
                                               Seconds(0.005), Seconds(0.200)),
                  TestCase::QUICK);
      AddTestCase(new LteHandoverDelayTestCase(4, true, handoverTime,
                                               Seconds(0.005), Seconds(0.200)),
                  TestCase::QUICK);
    }

    for (Time handoverTime = Seconds(0.100); handoverTime < Seconds(0.110);
         handoverTime += Seconds(0.001)) {
      AddTestCase(new LteHandoverDelayTestCase(1, false, handoverTime,
                                               Seconds(0.020), Seconds(0.200)),
                  TestCase::QUICK);
      AddTestCase(new LteHandoverDelayTestCase(2, false, handoverTime,
                                               Seconds(0.020), Seconds(0.200)),
                  TestCase::QUICK);
      AddTestCase(new LteHandoverDelayTestCase(4, false, handoverTime,
                                               Seconds(0.020), Seconds(0.200)),
                  TestCase::QUICK);
    }
  }
} g_lteHandoverDelayTestSuite;
