
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

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("lr-wpan-energy-detection-test");

class LrWpanEdTestCase : public TestCase {
public:
  LrWpanEdTestCase();

private:
  void DoRun() override;

  void PlmeEdConfirm(LrWpanPhyEnumeration status, uint8_t level);

  LrWpanPhyEnumeration m_status;
  uint8_t m_level;
};

LrWpanEdTestCase::LrWpanEdTestCase()
    : TestCase("Test the 802.15.4 energie detection") {
  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;
  m_level = 0;
}

void LrWpanEdTestCase::PlmeEdConfirm(LrWpanPhyEnumeration status,
                                     uint8_t level) {
  NS_LOG_UNCOND("Energy Detection completed with status "
                << LrWpanHelper::LrWpanPhyEnumerationPrinter(status)
                << " and energy level " << static_cast<uint32_t>(level));
  m_status = status;
  m_level = level;
}

void LrWpanEdTestCase::DoRun() {

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(6);

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();

  dev0->AssignStreams(0);
  dev1->AssignStreams(10);

  dev0->SetAddress(Mac16Address("00:01"));
  dev1->SetAddress(Mac16Address("00:02"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<FixedRssLossModel> propModel = CreateObject<FixedRssLossModel>();
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

  dev0->GetPhy()->SetPlmeEdConfirmCallback(
      MakeCallback(&LrWpanEdTestCase::PlmeEdConfirm, this));
  dev1->GetPhy()->SetPlmeEdConfirmCallback(
      MakeCallback(&LrWpanEdTestCase::PlmeEdConfirm, this));

  propModel->SetRss(-107.58);

  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;
  m_level = 0;
  Ptr<Packet> p0 = Create<Packet>(100);
  McpsDataRequestParams params;
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address("00:02");
  params.m_msduHandle = 0;
  params.m_txOptions = TX_OPTION_NONE;
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev0->GetMac(), params,
                         p0);

  Simulator::Schedule(Seconds(0.0025), &LrWpanPhy::PlmeEdRequest,
                      dev1->GetPhy());

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_status, IEEE_802_15_4_PHY_SUCCESS,
                        "ED status SUCCESS (as expected)");
  NS_TEST_EXPECT_MSG_EQ(m_level, 0, "ED reported signal level 0 (as expected)");

  propModel->SetRss(-106.58);

  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;
  m_level = 0;
  Ptr<Packet> p1 = Create<Packet>(100);
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address("00:02");
  params.m_msduHandle = 0;
  params.m_txOptions = TX_OPTION_NONE;
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev0->GetMac(), params,
                         p1);

  Simulator::Schedule(Seconds(0.0025), &LrWpanPhy::PlmeEdRequest,
                      dev1->GetPhy());

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_status, IEEE_802_15_4_PHY_SUCCESS,
                        "ED status SUCCESS (as expected)");
  NS_TEST_EXPECT_MSG_EQ(m_level, 0, "ED reported signal level 0 (as expected)");

  propModel->SetRss(-81.58);

  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;
  m_level = 0;
  Ptr<Packet> p2 = Create<Packet>(100);
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address("00:02");
  params.m_msduHandle = 0;
  params.m_txOptions = TX_OPTION_NONE;
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev0->GetMac(), params,
                         p2);

  Simulator::Schedule(Seconds(0.0025), &LrWpanPhy::PlmeEdRequest,
                      dev1->GetPhy());

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_status, IEEE_802_15_4_PHY_SUCCESS,
                        "ED status SUCCESS (as expected)");
  NS_TEST_EXPECT_MSG_EQ(m_level, 127,
                        "ED reported signal level 127 (as expected)");

  propModel->SetRss(-66.58);

  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;
  m_level = 0;
  Ptr<Packet> p3 = Create<Packet>(100);
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address("00:02");
  params.m_msduHandle = 0;
  params.m_txOptions = TX_OPTION_NONE;
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev0->GetMac(), params,
                         p3);

  Simulator::Schedule(Seconds(0.0025), &LrWpanPhy::PlmeEdRequest,
                      dev1->GetPhy());

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_status, IEEE_802_15_4_PHY_SUCCESS,
                        "ED status SUCCESS (as expected)");
  NS_TEST_EXPECT_MSG_EQ(m_level, 255,
                        "ED reported signal level 255 (as expected)");

  m_status = IEEE_802_15_4_PHY_UNSPECIFIED;
  m_level = 0;
  Ptr<Packet> p4 = Create<Packet>(100);
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address("00:02");
  params.m_msduHandle = 0;
  params.m_txOptions = TX_OPTION_NONE;
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev0->GetMac(), params,
                         p4);

  Simulator::Schedule(Seconds(0.0025), &LrWpanPhy::PlmeEdRequest,
                      dev0->GetPhy());

  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_status, IEEE_802_15_4_PHY_TX_ON,
                        "ED status TX_ON (as expected)");
  NS_TEST_EXPECT_MSG_EQ(m_level, 0, "ED reported signal level 0 (as expected)");

  Simulator::Destroy();
}

class LrWpanEdTestSuite : public TestSuite {
public:
  LrWpanEdTestSuite();
};

LrWpanEdTestSuite::LrWpanEdTestSuite()
    : TestSuite("lr-wpan-energy-detection", UNIT) {
  AddTestCase(new LrWpanEdTestCase, TestCase::QUICK);
}

static LrWpanEdTestSuite g_lrWpanEdTestSuite;
