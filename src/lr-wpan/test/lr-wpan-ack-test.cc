
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

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("lr-wpan-ack-test");

class LrWpanAckTestCase : public TestCase {
public:
  enum TestMode_e {
    EXTENDED_ADDRESS_UNICAST,
    SHORT_ADDRESS_UNICAST,
    SHORT_ADDRESS_MULTICAST,
    SHORT_ADDRESS_BROADCAST,
  };

  LrWpanAckTestCase(const char *const prefix, TestMode_e mode);

  void DataIndicationDev0(McpsDataIndicationParams params, Ptr<Packet> p);
  void DataIndicationDev1(McpsDataIndicationParams params, Ptr<Packet> p);
  void DataConfirmDev0(McpsDataConfirmParams params);
  void DataConfirmDev1(McpsDataConfirmParams params);

private:
  void DoRun() override;

  std::string m_prefix;
  Time m_requestTime;
  Time m_requestSentTime;
  Time m_replyTime;
  Time m_replySentTime;
  Time m_replyArrivalTime;
  TestMode_e m_mode;
  Ptr<LrWpanNetDevice> m_dev0;
  Ptr<LrWpanNetDevice> m_dev1;
};

LrWpanAckTestCase::LrWpanAckTestCase(const char *const prefix, TestMode_e mode)
    : TestCase("Test the 802.15.4 ACK handling") {
  m_prefix = prefix;
  m_requestTime = Seconds(0);
  m_requestSentTime = Seconds(0);
  m_replyTime = Seconds(0);
  m_replySentTime = Seconds(0);
  m_replyArrivalTime = Seconds(0);
  m_mode = mode;
}

void LrWpanAckTestCase::DataIndicationDev0(McpsDataIndicationParams params,
                                           Ptr<Packet> p) {
  m_replyArrivalTime = Simulator::Now();
}

void LrWpanAckTestCase::DataIndicationDev1(McpsDataIndicationParams params,
                                           Ptr<Packet> p) {
  Ptr<Packet> pkt = Create<Packet>(10);
  McpsDataRequestParams replyParams;
  replyParams.m_dstPanId = 0;
  replyParams.m_msduHandle = 0;
  replyParams.m_txOptions = TX_OPTION_NONE;

  if (m_mode == EXTENDED_ADDRESS_UNICAST) {
    replyParams.m_srcAddrMode = EXT_ADDR;
    replyParams.m_dstAddrMode = EXT_ADDR;
    replyParams.m_dstExtAddr = Mac64Address("00:00:00:00:00:00:00:01");
  } else {
    replyParams.m_srcAddrMode = SHORT_ADDR;
    replyParams.m_dstAddrMode = SHORT_ADDR;
    replyParams.m_dstAddr = Mac16Address("00:01");
  }
  m_replyTime = Simulator::Now();
  m_dev1->GetMac()->McpsDataRequest(replyParams, pkt);
}

void LrWpanAckTestCase::DataConfirmDev0(McpsDataConfirmParams params) {
  m_requestSentTime = Simulator::Now();
}

void LrWpanAckTestCase::DataConfirmDev1(McpsDataConfirmParams params) {
  m_replySentTime = Simulator::Now();
}

void LrWpanAckTestCase::DoRun() {

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(6);

  Packet::EnablePrinting();

  LrWpanHelper helper;
  std::string asciiPrefix;

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();

  m_dev0 = CreateObject<LrWpanNetDevice>();
  m_dev1 = CreateObject<LrWpanNetDevice>();

  m_dev0->AssignStreams(0);
  m_dev1->AssignStreams(10);

  m_dev0->SetAddress(Mac16Address("00:01"));
  m_dev1->SetAddress(Mac16Address("00:02"));
  m_dev0->GetMac()->SetExtendedAddress(Mac64Address("00:00:00:00:00:00:00:01"));
  m_dev1->GetMac()->SetExtendedAddress(Mac64Address("00:00:00:00:00:00:00:02"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  m_dev0->SetChannel(channel);
  m_dev1->SetChannel(channel);

  n0->AddDevice(m_dev0);
  n1->AddDevice(m_dev1);

  Ptr<ConstantPositionMobilityModel> sender0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender0Mobility->SetPosition(Vector(0, 0, 0));
  m_dev0->GetPhy()->SetMobility(sender0Mobility);
  Ptr<ConstantPositionMobilityModel> sender1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender1Mobility->SetPosition(Vector(0, 10, 0));
  m_dev1->GetPhy()->SetMobility(sender1Mobility);

  McpsDataConfirmCallback cb0;
  cb0 = MakeCallback(&LrWpanAckTestCase::DataConfirmDev0, this);
  m_dev0->GetMac()->SetMcpsDataConfirmCallback(cb0);

  McpsDataIndicationCallback cb1;
  cb1 = MakeCallback(&LrWpanAckTestCase::DataIndicationDev0, this);
  m_dev0->GetMac()->SetMcpsDataIndicationCallback(cb1);

  McpsDataConfirmCallback cb2;
  cb2 = MakeCallback(&LrWpanAckTestCase::DataConfirmDev1, this);
  m_dev1->GetMac()->SetMcpsDataConfirmCallback(cb2);

  McpsDataIndicationCallback cb3;
  cb3 = MakeCallback(&LrWpanAckTestCase::DataIndicationDev1, this);
  m_dev1->GetMac()->SetMcpsDataIndicationCallback(cb3);

  Ptr<Packet> p0 = Create<Packet>(50);
  McpsDataRequestParams params;
  uint8_t expectedAckCount = 0;
  switch (m_mode) {
  case SHORT_ADDRESS_UNICAST:
    params.m_srcAddrMode = SHORT_ADDR;
    params.m_dstAddrMode = SHORT_ADDR;
    params.m_dstAddr = Mac16Address("00:02");
    expectedAckCount = 1;
    break;
  case SHORT_ADDRESS_MULTICAST:
    params.m_srcAddrMode = SHORT_ADDR;
    params.m_dstAddrMode = SHORT_ADDR;
    params.m_dstAddr =
        Mac16Address::GetMulticast(Ipv6Address::GetAllNodesMulticast());
    expectedAckCount = 0;
    break;
  case SHORT_ADDRESS_BROADCAST:
    params.m_srcAddrMode = SHORT_ADDR;
    params.m_dstAddrMode = SHORT_ADDR;
    params.m_dstAddr = Mac16Address::GetBroadcast();
    expectedAckCount = 0;
    break;
  case EXTENDED_ADDRESS_UNICAST:
    params.m_srcAddrMode = EXT_ADDR;
    params.m_dstAddrMode = EXT_ADDR;
    params.m_dstExtAddr = Mac64Address("00:00:00:00:00:00:00:02");
    expectedAckCount = 1;
    break;
  }
  params.m_dstPanId = 0;
  params.m_msduHandle = 0;
  params.m_txOptions = TX_OPTION_ACK;
  m_requestTime = Simulator::Now();
  Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, m_dev0->GetMac(), params,
                         p0);

  helper.EnableAscii(CreateTempDirFilename(m_prefix), m_dev0);
  Simulator::Run();

  std::ifstream traceFile(CreateTempDirFilename(m_prefix) + "-0-0.tr");
  uint8_t ackCounter = 0;
  std::string sub("Frame Type = 2");
  for (std::string line; getline(traceFile, line);) {
    if (line.find(sub, 0) != std::string::npos) {
      ackCounter++;
    }
  }
  traceFile.close();

  NS_TEST_EXPECT_MSG_LT(m_requestTime, m_replyTime,
                        "Sent the request before the reply (as expected)");
  NS_TEST_EXPECT_MSG_GT(m_requestSentTime, Time(0),
                        "The request was sent (as expected)");
  NS_TEST_EXPECT_MSG_LT(
      m_requestSentTime, m_replyArrivalTime,
      "The request was sent before the reply arrived (as expected)");
  NS_TEST_EXPECT_MSG_LT(
      m_replySentTime, m_replyArrivalTime,
      "The reply was sent before the reply arrived (as expected)");
  NS_TEST_EXPECT_MSG_EQ(
      ackCounter, expectedAckCount,
      "The right amount of ACKs have been seen on the channel (as expected)");

  m_dev0 = nullptr;
  m_dev1 = nullptr;

  Simulator::Destroy();
}

class LrWpanAckTestSuite : public TestSuite {
public:
  LrWpanAckTestSuite();
};

LrWpanAckTestSuite::LrWpanAckTestSuite() : TestSuite("lr-wpan-ack", UNIT) {
  AddTestCase(new LrWpanAckTestCase("short-unicast",
                                    LrWpanAckTestCase::SHORT_ADDRESS_UNICAST),
              TestCase::QUICK);
  AddTestCase(new LrWpanAckTestCase("short-multicast",
                                    LrWpanAckTestCase::SHORT_ADDRESS_MULTICAST),
              TestCase::QUICK);
  AddTestCase(new LrWpanAckTestCase("short-broadcast",
                                    LrWpanAckTestCase::SHORT_ADDRESS_BROADCAST),
              TestCase::QUICK);
  AddTestCase(
      new LrWpanAckTestCase("extended-unicast",
                            LrWpanAckTestCase::EXTENDED_ADDRESS_UNICAST),
      TestCase::QUICK);
}

static LrWpanAckTestSuite g_lrWpanAckTestSuite;
