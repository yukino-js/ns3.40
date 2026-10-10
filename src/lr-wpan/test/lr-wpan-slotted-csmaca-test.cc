
#include <ns3/constant-position-mobility-model.h>
#include <ns3/core-module.h>
#include <ns3/log.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/packet.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("lr-wpan-slotted-csma-test");

class LrWpanSlottedCsmacaTestCase : public TestCase {
public:
  LrWpanSlottedCsmacaTestCase();
  ~LrWpanSlottedCsmacaTestCase() override;

private:
  static void TransEndIndication(LrWpanSlottedCsmacaTestCase *testcase,
                                 Ptr<LrWpanNetDevice> dev,
                                 McpsDataConfirmParams params);
  static void DataIndicationCoordinator(LrWpanSlottedCsmacaTestCase *testcase,
                                        Ptr<LrWpanNetDevice> dev,
                                        McpsDataIndicationParams params,
                                        Ptr<Packet> p);
  static void StartConfirm(LrWpanSlottedCsmacaTestCase *testcase,
                           Ptr<LrWpanNetDevice> dev,
                           MlmeStartConfirmParams params);

  static void IncomingSuperframeStatus(LrWpanSlottedCsmacaTestCase *testcase,
                                       Ptr<LrWpanNetDevice> dev,
                                       SuperframeStatus oldValue,
                                       SuperframeStatus newValue);

  static void TransactionCost(LrWpanSlottedCsmacaTestCase *testcase,
                              Ptr<LrWpanNetDevice> dev, uint32_t trans);

  void DoRun() override;

  Time m_startCap;
  Time m_apBoundary;
  Time m_sentTime;
  uint32_t m_transCost;
};

LrWpanSlottedCsmacaTestCase::LrWpanSlottedCsmacaTestCase()
    : TestCase("Lrwpan: Slotted CSMA-CA test") {
  m_transCost = 0;
}

LrWpanSlottedCsmacaTestCase::~LrWpanSlottedCsmacaTestCase() {}

void LrWpanSlottedCsmacaTestCase::TransEndIndication(
    LrWpanSlottedCsmacaTestCase *testcase, Ptr<LrWpanNetDevice> dev,
    McpsDataConfirmParams params) {
  if (params.m_status == LrWpanMcpsDataConfirmStatus::IEEE_802_15_4_SUCCESS) {
    NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                  << "s Transmission successfully sent");
    testcase->m_sentTime = Simulator::Now();
  }
}

void LrWpanSlottedCsmacaTestCase::DataIndicationCoordinator(
    LrWpanSlottedCsmacaTestCase *testcase, Ptr<LrWpanNetDevice> dev,
    McpsDataIndicationParams params, Ptr<Packet> p) {
  NS_LOG_UNCOND(Simulator::Now().As(Time::S)
                << "s Coordinator Received DATA packet (size " << p->GetSize()
                << " bytes)");
}

void LrWpanSlottedCsmacaTestCase::StartConfirm(
    LrWpanSlottedCsmacaTestCase *testcase, Ptr<LrWpanNetDevice> dev,
    MlmeStartConfirmParams params) {
  NS_LOG_UNCOND(Simulator::Now().As(Time::S) << "s Beacon Sent");
}

void LrWpanSlottedCsmacaTestCase::IncomingSuperframeStatus(
    LrWpanSlottedCsmacaTestCase *testcase, Ptr<LrWpanNetDevice> dev,
    SuperframeStatus oldValue, SuperframeStatus newValue) {
  if (newValue == SuperframeStatus::CAP) {
    testcase->m_startCap = Simulator::Now();
    NS_LOG_UNCOND(Simulator::Now().As(Time::S)
                  << "s Incoming superframe CAP starts");
  }
}

void LrWpanSlottedCsmacaTestCase::TransactionCost(
    LrWpanSlottedCsmacaTestCase *testcase, Ptr<LrWpanNetDevice> dev,
    uint32_t trans) {
  testcase->m_apBoundary = Simulator::Now();
  testcase->m_transCost = trans;
  NS_LOG_UNCOND(Simulator::Now().As(Time::S)
                << "s Transaction Cost is:" << trans);
}

void LrWpanSlottedCsmacaTestCase::DoRun() {
  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();

  dev0->SetAddress(Mac16Address("00:01"));
  dev1->SetAddress(Mac16Address("00:02"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  dev0->SetChannel(channel);
  dev1->SetChannel(channel);

  n0->AddDevice(dev0);
  n1->AddDevice(dev1);

  Ptr<ConstantPositionMobilityModel> sender0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender0Mobility->SetPosition(Vector(0, 0, 0));
  dev0->GetPhy()->SetMobility(sender0Mobility);
  Ptr<ConstantPositionMobilityModel> sender1Mobility =
      CreateObject<ConstantPositionMobilityModel>();

  sender1Mobility->SetPosition(Vector(0, 10, 0));
  dev1->GetPhy()->SetMobility(sender1Mobility);

  MlmeStartConfirmCallback cb0;
  cb0 =
      MakeBoundCallback(&LrWpanSlottedCsmacaTestCase::StartConfirm, this, dev0);
  dev0->GetMac()->SetMlmeStartConfirmCallback(cb0);

  McpsDataConfirmCallback cb1;
  cb1 = MakeBoundCallback(&LrWpanSlottedCsmacaTestCase::TransEndIndication,
                          this, dev1);
  dev1->GetMac()->SetMcpsDataConfirmCallback(cb1);

  LrWpanMacTransCostCallback cb2;
  cb2 = MakeBoundCallback(&LrWpanSlottedCsmacaTestCase::TransactionCost, this,
                          dev1);
  dev1->GetCsmaCa()->SetLrWpanMacTransCostCallback(cb2);

  McpsDataIndicationCallback cb5;
  cb5 = MakeBoundCallback(
      &LrWpanSlottedCsmacaTestCase::DataIndicationCoordinator, this, dev0);
  dev0->GetMac()->SetMcpsDataIndicationCallback(cb5);

  dev1->GetMac()->TraceConnectWithoutContext(
      "MacIncSuperframeStatus",
      MakeBoundCallback(&LrWpanSlottedCsmacaTestCase::IncomingSuperframeStatus,
                        this, dev1));

  dev1->GetMac()->SetPanId(5);
  dev1->GetMac()->SetAssociatedCoor(Mac16Address("00:01"));

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 14;
  params.m_sfrmOrd = 6;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 dev0->GetMac(), params);

  Ptr<Packet> p1 = Create<Packet>(5);
  McpsDataRequestParams params2;
  params2.m_dstPanId = 5;
  params2.m_srcAddrMode = SHORT_ADDR;
  params2.m_dstAddrMode = SHORT_ADDR;
  params2.m_dstAddr = Mac16Address("00:01");
  params2.m_msduHandle = 0;

  Simulator::ScheduleWithContext(1, Seconds(2.93), &LrWpanMac::McpsDataRequest,
                                 dev1->GetMac(), params2, p1);

  Simulator::Stop(Seconds(4));
  Simulator::Run();

  Time activePeriodsSum;
  Time transactionTime;
  uint64_t symbolRate;
  uint32_t activePeriodSize = 20;
  double boundary;

  symbolRate = (uint64_t)dev1->GetMac()->GetPhy()->GetDataOrSymbolRate(false);
  activePeriodsSum = m_apBoundary - m_startCap;
  boundary = (activePeriodsSum.GetMicroSeconds() * 1000 * 1000 * symbolRate) %
             activePeriodSize;

  NS_TEST_EXPECT_MSG_EQ(boundary, 0,
                        "Error, the transaction is not calculated on a "
                        "boundary of an Active Period in the CAP");

  uint32_t ifsSize;
  if (p1->GetSize() > 18) {
    ifsSize = 40;
  } else {
    ifsSize = 12;
  }

  transactionTime =
      Seconds((double)(m_transCost - (ifsSize + 12)) / symbolRate);
  NS_LOG_UNCOND(
      "Transmission start time(On a boundary): " << m_apBoundary.As(Time::S));
  NS_LOG_UNCOND(
      "Transmission End time (McpsData.confirm): " << m_sentTime.As(Time::S));

  NS_TEST_EXPECT_MSG_EQ(
      m_sentTime, (m_apBoundary + transactionTime),
      "Error, the transaction time is not the expected value");

  Simulator::Destroy();
}

class LrWpanSlottedCsmacaTestSuite : public TestSuite {
public:
  LrWpanSlottedCsmacaTestSuite();
};

LrWpanSlottedCsmacaTestSuite::LrWpanSlottedCsmacaTestSuite()
    : TestSuite("lr-wpan-slotted-csmaca", UNIT) {
  AddTestCase(new LrWpanSlottedCsmacaTestCase, TestCase::QUICK);
}

static LrWpanSlottedCsmacaTestSuite lrWpanSlottedCsmacaTestSuite;
