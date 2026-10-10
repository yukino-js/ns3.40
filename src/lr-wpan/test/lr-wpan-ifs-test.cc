
#include "ns3/rng-seed-manager.h"
#include <ns3/constant-position-mobility-model.h>
#include <ns3/core-module.h>
#include <ns3/log.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/packet.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>

#include <iomanip>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("lr-wpan-ifs-test");

class LrWpanDataIfsTestCase : public TestCase {
public:
  LrWpanDataIfsTestCase();
  ~LrWpanDataIfsTestCase() override;

private:
  static void DataConfirm(LrWpanDataIfsTestCase *testcase,
                          Ptr<LrWpanNetDevice> dev,
                          McpsDataConfirmParams params);

  static void DataReceivedDev0(LrWpanDataIfsTestCase *testcase,
                               Ptr<LrWpanNetDevice> dev, Ptr<const Packet> p);

  static void PhyDataRxStart(LrWpanDataIfsTestCase *testcase,
                             Ptr<LrWpanNetDevice> dev, Ptr<const Packet> p);

  static void DataReceivedDev1(LrWpanDataIfsTestCase *testcase,
                               Ptr<LrWpanNetDevice> dev, Ptr<const Packet>);

  static void IfsEnd(LrWpanDataIfsTestCase *testcase, Ptr<LrWpanNetDevice> dev,
                     Time IfsTime);

  void DoRun() override;
  Time m_lastTxTime;
  Time m_ackRxTime;
  Time m_endIfs;
  Time m_phyStartRx;
};

LrWpanDataIfsTestCase::LrWpanDataIfsTestCase()
    : TestCase("Lrwpan: IFS tests") {}

LrWpanDataIfsTestCase::~LrWpanDataIfsTestCase() {}

void LrWpanDataIfsTestCase::DataConfirm(LrWpanDataIfsTestCase *testcase,
                                        Ptr<LrWpanNetDevice> dev,
                                        McpsDataConfirmParams params) {
  testcase->m_lastTxTime = Simulator::Now();
}

void LrWpanDataIfsTestCase::DataReceivedDev0(LrWpanDataIfsTestCase *testcase,
                                             Ptr<LrWpanNetDevice> dev,
                                             Ptr<const Packet> p) {
  Ptr<Packet> RxPacket = p->Copy();
  LrWpanMacHeader receivedMacHdr;
  RxPacket->RemoveHeader(receivedMacHdr);

  if (receivedMacHdr.IsAcknowledgment()) {
    testcase->m_ackRxTime = Simulator::Now();
    std::cout << Simulator::Now().GetSeconds()
              << " | Dev0 (Node 0) received Acknowledgment.\n";
  } else if (receivedMacHdr.GetShortDstAddr().IsBroadcast()) {
    std::cout << Simulator::Now().GetSeconds()
              << " | Dev0 (Node 0) received Broadcast. \n";
  }
}

void LrWpanDataIfsTestCase::PhyDataRxStart(LrWpanDataIfsTestCase *testcase,
                                           Ptr<LrWpanNetDevice> dev,
                                           Ptr<const Packet>) {
  testcase->m_phyStartRx = Simulator::Now();
}

void LrWpanDataIfsTestCase::DataReceivedDev1(LrWpanDataIfsTestCase *testcase,
                                             Ptr<LrWpanNetDevice> dev,
                                             Ptr<const Packet> p) {
  Ptr<Packet> RxPacket = p->Copy();
  LrWpanMacHeader receivedMacHdr;
  RxPacket->RemoveHeader(receivedMacHdr);

  if (receivedMacHdr.GetShortDstAddr().IsBroadcast()) {
    std::cout << Simulator::Now().GetSeconds()
              << " | Dev1 (Node 1) received Broadcast. \n";

    Ptr<Packet> p0 = Create<Packet>(50);
    McpsDataRequestParams params1;
    params1.m_dstPanId = 0;
    params1.m_srcAddrMode = SHORT_ADDR;
    params1.m_dstAddrMode = SHORT_ADDR;
    params1.m_dstAddr = Mac16Address("ff:ff");
    params1.m_msduHandle = 0;

    Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev->GetMac(), params1,
                           p0);
  }
}

void LrWpanDataIfsTestCase::IfsEnd(LrWpanDataIfsTestCase *testcase,
                                   Ptr<LrWpanNetDevice> dev, Time IfsTime) {
  testcase->m_endIfs = Simulator::Now();
}

void LrWpanDataIfsTestCase::DoRun() {

  LogComponentEnableAll(LOG_PREFIX_TIME);
  LogComponentEnableAll(LOG_PREFIX_FUNC);
  LogComponentEnable("LrWpanPhy", LOG_LEVEL_DEBUG);
  LogComponentEnable("LrWpanMac", LOG_LEVEL_DEBUG);
  LogComponentEnable("LrWpanCsmaCa", LOG_LEVEL_DEBUG);

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

  dev0->GetMac()->TraceConnectWithoutContext(
      "IfsEnd", MakeBoundCallback(&LrWpanDataIfsTestCase::IfsEnd, this, dev0));
  dev0->GetMac()->TraceConnectWithoutContext(
      "MacRx",
      MakeBoundCallback(&LrWpanDataIfsTestCase::DataReceivedDev0, this, dev0));
  dev0->GetPhy()->TraceConnectWithoutContext(
      "PhyRxBegin",
      MakeBoundCallback(&LrWpanDataIfsTestCase::PhyDataRxStart, this, dev0));
  dev1->GetMac()->TraceConnectWithoutContext(
      "MacRx",
      MakeBoundCallback(&LrWpanDataIfsTestCase::DataReceivedDev1, this, dev1));

  Ptr<ConstantPositionMobilityModel> sender0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender0Mobility->SetPosition(Vector(0, 0, 0));
  dev0->GetPhy()->SetMobility(sender0Mobility);
  Ptr<ConstantPositionMobilityModel> sender1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender1Mobility->SetPosition(Vector(0, 10, 0));
  dev1->GetPhy()->SetMobility(sender1Mobility);

  McpsDataConfirmCallback cb0;
  cb0 = MakeBoundCallback(&LrWpanDataIfsTestCase::DataConfirm, this, dev0);
  dev0->GetMac()->SetMcpsDataConfirmCallback(cb0);

  McpsDataConfirmCallback cb1;
  cb1 = MakeBoundCallback(&LrWpanDataIfsTestCase::DataConfirm, this, dev1);
  dev1->GetMac()->SetMcpsDataConfirmCallback(cb1);

  Ptr<Packet> p0 = Create<Packet>(2);
  McpsDataRequestParams params;
  params.m_dstPanId = 0;

  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstAddr = Mac16Address("00:02");
  params.m_msduHandle = 0;

  Time ifsSize;

  Simulator::ScheduleWithContext(1, Seconds(0.0), &LrWpanMac::McpsDataRequest,
                                 dev0->GetMac(), params, p0);

  Simulator::Run();

  ifsSize = m_endIfs - m_lastTxTime;
  NS_TEST_EXPECT_MSG_EQ(
      ifsSize, Time(MicroSeconds(192)),
      "Wrong Short InterFrame Space (SIFS) Size after dataframe Tx");
  std::cout << "----------------------------------\n";

  p0 = Create<Packet>(8);

  Simulator::ScheduleWithContext(1, Seconds(0.0), &LrWpanMac::McpsDataRequest,
                                 dev0->GetMac(), params, p0);

  Simulator::Run();

  ifsSize = m_endIfs - m_lastTxTime;
  NS_TEST_EXPECT_MSG_EQ(
      ifsSize, Time(MicroSeconds(640)),
      "Wrong Long InterFrame Space (LIFS) Size after dataframe Tx");
  std::cout << "----------------------------------\n";

  params.m_txOptions = TX_OPTION_ACK;
  p0 = Create<Packet>(2);

  Simulator::ScheduleWithContext(1, Seconds(0.0), &LrWpanMac::McpsDataRequest,
                                 dev0->GetMac(), params, p0);

  Simulator::Run();

  ifsSize = m_endIfs - m_ackRxTime;
  NS_TEST_EXPECT_MSG_EQ(
      ifsSize, Time(MicroSeconds(192)),
      "Wrong Short InterFrame Space (SIFS) Size after ACK Rx");
  std::cout << "----------------------------------\n";

  params.m_txOptions = TX_OPTION_ACK;
  p0 = Create<Packet>(8);

  Simulator::ScheduleWithContext(1, Seconds(0.0), &LrWpanMac::McpsDataRequest,
                                 dev0->GetMac(), params, p0);

  Simulator::Run();

  ifsSize = m_endIfs - m_ackRxTime;
  NS_TEST_EXPECT_MSG_EQ(ifsSize, Time(MicroSeconds(640)),
                        "Wrong Long InterFrame Space (LIFS) Size after ACK Rx");
  std::cout << "----------------------------------\n";

  dev0->GetCsmaCa()->SetMacMinBE(0);
  dev1->GetCsmaCa()->SetMacMinBE(0);

  p0 = Create<Packet>(50);
  params.m_dstPanId = 0;
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstAddr = Mac16Address("ff:ff");
  params.m_msduHandle = 0;

  Simulator::ScheduleWithContext(1, Seconds(0.0), &LrWpanMac::McpsDataRequest,
                                 dev0->GetMac(), params, p0);

  Simulator::Run();

  NS_TEST_ASSERT_MSG_GT(
      m_endIfs, m_phyStartRx,
      "Error, IFS end time should be greater than PHY start Rx time");

  Simulator::Destroy();
}

class LrWpanIfsTestSuite : public TestSuite {
public:
  LrWpanIfsTestSuite();
};

LrWpanIfsTestSuite::LrWpanIfsTestSuite() : TestSuite("lr-wpan-ifs-test", UNIT) {
  AddTestCase(new LrWpanDataIfsTestCase, TestCase::QUICK);
}

static LrWpanIfsTestSuite lrWpanIfsTestSuite;
