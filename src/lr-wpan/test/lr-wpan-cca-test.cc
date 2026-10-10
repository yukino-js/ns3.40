
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

NS_LOG_COMPONENT_DEFINE("lr-wpan-clear-channel-assessment-test");

class LrWpanCcaTestCase : public TestCase {
public:
  LrWpanCcaTestCase();

private:
  static void PlmeCcaConfirm(LrWpanCcaTestCase *testcase,
                             Ptr<LrWpanNetDevice> device,
                             LrWpanPhyEnumeration status);
  static void PhyTxBegin(LrWpanCcaTestCase *testcase,
                         Ptr<LrWpanNetDevice> device, Ptr<const Packet> packet);
  static void PhyTxEnd(LrWpanCcaTestCase *testcase, Ptr<LrWpanNetDevice> device,
                       Ptr<const Packet> packet);
  static void PhyRxBegin(LrWpanCcaTestCase *testcase,
                         Ptr<LrWpanNetDevice> device, Ptr<const Packet> packet);
  static void PhyRxEnd(LrWpanCcaTestCase *testcase, Ptr<LrWpanNetDevice> device,
                       Ptr<const Packet> packet, double sinr);
  static void PhyRxDrop(LrWpanCcaTestCase *testcase,
                        Ptr<LrWpanNetDevice> device, Ptr<const Packet> packet);

  void DoRun() override;

  LrWpanPhyEnumeration m_status;
};

LrWpanCcaTestCase::LrWpanCcaTestCase()
    : TestCase("Test the 802.15.4 clear channel assessment") {
  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;
}

void LrWpanCcaTestCase::PlmeCcaConfirm(LrWpanCcaTestCase *testcase,
                                       Ptr<LrWpanNetDevice> device,
                                       LrWpanPhyEnumeration status) {
  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(9) << "["
            << Simulator::Now().As(Time::S) << "] "
            << device->GetMac()->GetShortAddress() << " PlmeCcaConfirm: "
            << LrWpanHelper::LrWpanPhyEnumerationPrinter(status) << std::endl;

  testcase->m_status = status;
}

void LrWpanCcaTestCase::PhyTxBegin(LrWpanCcaTestCase *testcase,
                                   Ptr<LrWpanNetDevice> device,
                                   Ptr<const Packet> packet) {
  std::ostringstream os;
  packet->Print(os);
  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(9) << "["
            << Simulator::Now().As(Time::S) << "] "
            << device->GetMac()->GetShortAddress()
            << " PhyTxBegin: " << os.str() << std::endl;
}

void LrWpanCcaTestCase::PhyTxEnd(LrWpanCcaTestCase *testcase,
                                 Ptr<LrWpanNetDevice> device,
                                 Ptr<const Packet> packet) {
  std::ostringstream os;
  packet->Print(os);
  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(9) << "["
            << Simulator::Now().As(Time::S) << "] "
            << device->GetMac()->GetShortAddress() << " PhyTxEnd: " << os.str()
            << std::endl;
}

void LrWpanCcaTestCase::PhyRxBegin(LrWpanCcaTestCase *testcase,
                                   Ptr<LrWpanNetDevice> device,
                                   Ptr<const Packet> packet) {
  std::ostringstream os;
  packet->Print(os);
  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(9) << "["
            << Simulator::Now().As(Time::S) << "] "
            << device->GetMac()->GetShortAddress()
            << " PhyRxBegin: " << os.str() << std::endl;
}

void LrWpanCcaTestCase::PhyRxEnd(LrWpanCcaTestCase *testcase,
                                 Ptr<LrWpanNetDevice> device,
                                 Ptr<const Packet> packet, double sinr) {
  std::ostringstream os;
  packet->Print(os);
  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(9) << "["
            << Simulator::Now().As(Time::S) << "] "
            << device->GetMac()->GetShortAddress() << " PhyRxEnd (" << sinr
            << "): " << os.str() << std::endl;

  device->GetPhy()->PlmeCcaRequest();
}

void LrWpanCcaTestCase::PhyRxDrop(LrWpanCcaTestCase *testcase,
                                  Ptr<LrWpanNetDevice> device,
                                  Ptr<const Packet> packet) {
  std::ostringstream os;
  packet->Print(os);
  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(9) << "["
            << Simulator::Now().As(Time::S) << "] "
            << device->GetMac()->GetShortAddress() << " PhyRxDrop: " << os.str()
            << std::endl;
}

void LrWpanCcaTestCase::DoRun() {

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(6);

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();
  Ptr<Node> n2 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev2 = CreateObject<LrWpanNetDevice>();

  dev0->AssignStreams(0);
  dev1->AssignStreams(10);
  dev2->AssignStreams(20);

  dev0->SetAddress(Mac16Address("00:01"));
  dev1->SetAddress(Mac16Address("00:02"));
  dev2->SetAddress(Mac16Address("00:03"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  propModel->SetReference(1.0, 40.0641);
  propModel->SetPathLossExponent(2);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  dev0->SetChannel(channel);
  dev1->SetChannel(channel);
  dev2->SetChannel(channel);

  n0->AddDevice(dev0);
  n1->AddDevice(dev1);
  n2->AddDevice(dev2);

  Ptr<ConstantPositionMobilityModel> sender0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender0Mobility->SetPosition(Vector(0, 0, 0));
  dev0->GetPhy()->SetMobility(sender0Mobility);
  Ptr<ConstantPositionMobilityModel> sender1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender1Mobility->SetPosition(Vector(0, 669, 0));
  dev1->GetPhy()->SetMobility(sender1Mobility);
  Ptr<ConstantPositionMobilityModel> sender2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender2Mobility->SetPosition(Vector(0, 1338, 0));
  dev2->GetPhy()->SetMobility(sender2Mobility);

  dev0->GetMac()->SetMcpsDataConfirmCallback(
      MakeNullCallback<void, McpsDataConfirmParams>());
  dev1->GetMac()->SetMcpsDataConfirmCallback(
      MakeNullCallback<void, McpsDataConfirmParams>());
  dev2->GetMac()->SetMcpsDataConfirmCallback(
      MakeNullCallback<void, McpsDataConfirmParams>());

  dev1->GetPhy()->SetPlmeCcaConfirmCallback(
      MakeBoundCallback(&LrWpanCcaTestCase::PlmeCcaConfirm, this, dev1));

  dev0->GetCsmaCa()->SetMacMinBE(0);
  dev2->GetCsmaCa()->SetMacMinBE(0);

  dev0->GetPhy()->TraceConnectWithoutContext(
      "PhyTxBegin",
      MakeBoundCallback(&LrWpanCcaTestCase::PhyTxBegin, this, dev0));
  dev0->GetPhy()->TraceConnectWithoutContext(
      "PhyTxEnd", MakeBoundCallback(&LrWpanCcaTestCase::PhyTxEnd, this, dev0));
  dev2->GetPhy()->TraceConnectWithoutContext(
      "PhyTxBegin",
      MakeBoundCallback(&LrWpanCcaTestCase::PhyTxBegin, this, dev2));
  dev2->GetPhy()->TraceConnectWithoutContext(
      "PhyTxEnd", MakeBoundCallback(&LrWpanCcaTestCase::PhyTxEnd, this, dev2));
  dev1->GetPhy()->TraceConnectWithoutContext(
      "PhyRxBegin",
      MakeBoundCallback(&LrWpanCcaTestCase::PhyRxBegin, this, dev1));
  dev1->GetPhy()->TraceConnectWithoutContext(
      "PhyRxEnd", MakeBoundCallback(&LrWpanCcaTestCase::PhyRxEnd, this, dev1));
  dev1->GetPhy()->TraceConnectWithoutContext(
      "PhyRxDrop",
      MakeBoundCallback(&LrWpanCcaTestCase::PhyRxDrop, this, dev1));

  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;

  Ptr<Packet> p0 = Create<Packet>(1);
  McpsDataRequestParams params0;
  params0.m_srcAddrMode = SHORT_ADDR;
  params0.m_dstAddrMode = SHORT_ADDR;
  params0.m_dstPanId = 0;
  params0.m_dstAddr = Mac16Address("00:02");
  params0.m_msduHandle = 0;
  params0.m_txOptions = TX_OPTION_NONE;
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev0->GetMac(), params0,
                         p0);

  Ptr<Packet> p1 = Create<Packet>(100);
  McpsDataRequestParams params1;
  params1.m_srcAddrMode = SHORT_ADDR;
  params1.m_dstAddrMode = SHORT_ADDR;
  params1.m_dstPanId = 0;
  params1.m_dstAddr = Mac16Address("00:02");
  params1.m_msduHandle = 0;
  params1.m_txOptions = TX_OPTION_NONE;
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev2->GetMac(), params1,
                         p1);

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_status, IEEE_802_15_4_PHY_BUSY,
                        "CCA status BUSY (as expected)");

  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;

  sender2Mobility->SetPosition(Vector(0, 1340, 0));

  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev0->GetMac(), params0,
                         p0);
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev2->GetMac(), params1,
                         p1);

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_status, IEEE_802_15_4_PHY_IDLE,
                        "CCA status IDLE (as expected)");

  Simulator::Destroy();
}

class LrWpanCcaTestSuite : public TestSuite {
public:
  LrWpanCcaTestSuite();
};

LrWpanCcaTestSuite::LrWpanCcaTestSuite()
    : TestSuite("lr-wpan-clear-channel-assessment", UNIT) {
  AddTestCase(new LrWpanCcaTestCase, TestCase::QUICK);
}

static LrWpanCcaTestSuite g_lrWpanCcaTestSuite;
