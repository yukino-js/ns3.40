

#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/error-model.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-interface-container.h"
#include "ns3/log.h"
#include "ns3/neighbor-cache-helper.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/ping.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

#include <list>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PingTestSuite");

constexpr bool USEIPV6_FALSE = false;
constexpr bool USEIPV6_TRUE = true;

class PingTestCase : public TestCase {
public:
  PingTestCase(std::string name, bool useIpv6);

  void SetSimulatorStopTime(Time stopTime);

  void SetDestinationAddress(Address address);

  void SetStartTime(Time startTime) { m_startTime = startTime; }

  void SetStopTime(Time stopTime) { m_stopTime = stopTime; }

  void SetCount(uint32_t count) { m_countAttribute = count; }

  void SetSize(uint32_t size) { m_sizeAttribute = size; }

  void SetInterval(Time interval) { m_interpacketInterval = interval; }

  void SetDropList(const std::list<uint32_t> &dropList) {
    m_dropList = dropList;
  }

  void CheckTraceTx(uint32_t expectedTx);

  void CheckTraceRtt(uint32_t expectedRtt);

  void CheckReportTransmitted(uint32_t expectedReportTx);

  void CheckReportReceived(uint32_t expectedReportRx);

  void CheckReportLoss(uint16_t expectedReportLoss);

  void CheckReportTime(Time expectedTime);

private:
  void DoSetup() override;
  void DoTeardown() override;
  void DoRun() override;

  void TxTraceSink(uint16_t seq, Ptr<Packet> p);

  void RttTraceSink(uint16_t seq, Time rttSample);

  void DropTraceSink(uint16_t seq, Ping::DropReason reason);

  void ReportTraceSink(const Ping::PingReport &report);

  Address m_destination{Ipv4Address("10.0.0.2")};
  uint32_t m_mtu{1500};
  uint32_t m_countAttribute{0};
  uint32_t m_sizeAttribute{56};
  uint32_t m_expectedTraceTx{0};
  uint32_t m_expectedTraceRtt{0};
  uint32_t m_countTraceTx{0};
  uint32_t m_countTraceRtt{0};
  bool m_checkTraceTx{false};
  bool m_checkTraceRtt{false};
  uint32_t m_expectedReportTx{0};
  uint32_t m_expectedReportRx{0};
  uint16_t m_expectedReportLoss{0};
  Time m_expectedReportTime;
  bool m_checkReportTransmitted{false};
  bool m_checkReportReceived{false};
  bool m_checkReportLoss{false};
  bool m_checkReportTime{false};

  Time m_startTime{Seconds(1)};
  Time m_stopTime{Seconds(5)};
  Time m_simulatorStopTime{Seconds(6)};
  bool m_useIpv6{false};
  Time m_interpacketInterval{Seconds(1.0)};
  Time m_lastTx;
  std::list<uint32_t> m_dropList;

  NodeContainer m_nodes;
  Ipv4InterfaceContainer m_ipv4Interfaces;
  Ipv6InterfaceContainer m_ipv6Interfaces;
  NetDeviceContainer m_devices;
};

PingTestCase::PingTestCase(std::string name, bool useIpv6)
    : TestCase(name), m_useIpv6(useIpv6) {}

void PingTestCase::DoSetup() {
  NS_LOG_FUNCTION(this);
  m_nodes.Create(2);
  SimpleNetDeviceHelper deviceHelper;
  deviceHelper.SetChannel("ns3::SimpleChannel", "Delay",
                          TimeValue(MilliSeconds(10)));
  deviceHelper.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Gbps")));
  deviceHelper.SetNetDevicePointToPointMode(true);
  m_devices = deviceHelper.Install(m_nodes);
  Ptr<SimpleNetDevice> device0 = DynamicCast<SimpleNetDevice>(m_devices.Get(0));
  device0->SetMtu(m_mtu);
  Ptr<SimpleNetDevice> device1 = DynamicCast<SimpleNetDevice>(m_devices.Get(1));
  device1->SetMtu(m_mtu);

  InternetStackHelper internetHelper;
  if (!m_useIpv6) {
    internetHelper.SetIpv6StackInstall(false);
  } else {
    internetHelper.SetIpv4StackInstall(false);
  }
  internetHelper.Install(m_nodes);

  if (!m_useIpv6) {
    Ipv4AddressHelper ipv4AddrHelper;
    ipv4AddrHelper.SetBase("10.0.0.0", "255.255.255.0");
    m_ipv4Interfaces = ipv4AddrHelper.Assign(m_devices);
  } else {
    m_nodes.Get(0)->GetObject<Icmpv6L4Protocol>()->SetAttribute(
        "DAD", BooleanValue(false));
    m_nodes.Get(1)->GetObject<Icmpv6L4Protocol>()->SetAttribute(
        "DAD", BooleanValue(false));

    Ipv6AddressHelper ipv6AddrHelper;
    ipv6AddrHelper.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
    m_ipv6Interfaces = ipv6AddrHelper.Assign(m_devices);

    m_nodes.Get(0)->GetObject<Ipv6L3Protocol>()->SetForwarding(1, true);
    m_nodes.Get(1)->GetObject<Ipv6L3Protocol>()->SetForwarding(1, true);
  }
}

void PingTestCase::DoTeardown() { Simulator::Destroy(); }

void PingTestCase::CheckReportTransmitted(uint32_t expectedReportTx) {
  m_checkReportTransmitted = true;
  m_expectedReportTx = expectedReportTx;
}

void PingTestCase::CheckReportReceived(uint32_t expectedReportRx) {
  m_checkReportReceived = true;
  m_expectedReportRx = expectedReportRx;
}

void PingTestCase::CheckReportLoss(uint16_t expectedReportLoss) {
  m_checkReportLoss = true;
  m_expectedReportLoss = expectedReportLoss;
}

void PingTestCase::CheckReportTime(Time expectedTime) {
  m_checkReportTime = true;
  m_expectedReportTime = expectedTime;
}

void PingTestCase::RttTraceSink(uint16_t seq, Time rttSample) {
  NS_LOG_FUNCTION(this << seq << rttSample.As(Time::MS));
  m_countTraceRtt++;
  if (m_sizeAttribute == 56) {
    if (!m_useIpv6) {
      NS_TEST_ASSERT_MSG_EQ_TOL(
          rttSample.GetSeconds() * 1000, 20.0013, 0.0001,
          "Rtt sample not within 0.0001 ms of expected value of 20.0013 ms");
    } else {
      NS_TEST_ASSERT_MSG_EQ_TOL(
          rttSample.GetSeconds() * 1000, 20.0017, 0.0001,
          "Rtt sample not within 0.0001 ms of expected value of 20.0017 ms");
    }
  }
}

void PingTestCase::TxTraceSink(uint16_t seq, Ptr<Packet> p) {
  NS_LOG_FUNCTION(this << seq << p);
  m_countTraceTx++;
  if (m_lastTx == Seconds(0)) {
    m_lastTx = Simulator::Now();
  } else {
    NS_TEST_ASSERT_MSG_EQ(
        Simulator::Now() - m_lastTx, m_interpacketInterval,
        "configured interval didn't match the observed interval");
    m_lastTx = Simulator::Now();
  }
}

void PingTestCase::DropTraceSink(uint16_t seq, Ping::DropReason reason) {
  NS_LOG_FUNCTION(this << static_cast<uint16_t>(reason));
  if (reason == Ping::DropReason::DROP_TIMEOUT) {
    NS_LOG_DEBUG("Drop due to timeout " << seq);
  } else if (reason == Ping::DROP_HOST_UNREACHABLE) {
    NS_LOG_DEBUG("Destination host is unreachable " << seq);
  } else if (reason == Ping::DROP_NET_UNREACHABLE) {
    NS_LOG_DEBUG("Destination network not reachable " << seq);
  }
}

void PingTestCase::ReportTraceSink(const Ping::PingReport &report) {
  NS_LOG_FUNCTION(this << report.m_transmitted << report.m_received
                       << report.m_loss << report.m_rttMin << report.m_rttMax);
  if (m_checkReportTransmitted) {
    NS_TEST_ASSERT_MSG_EQ(report.m_transmitted, m_expectedReportTx,
                          "Report transmit count does not equal expected");
  }
  if (m_checkReportReceived) {
    NS_TEST_ASSERT_MSG_EQ(report.m_received, m_expectedReportRx,
                          "Report receive count does not equal expected");
  }
  if (m_checkReportLoss) {
    NS_TEST_ASSERT_MSG_EQ(report.m_loss, m_expectedReportLoss,
                          "Report lost count does not equal expected");
  }
  if (m_checkReportTime) {
    NS_TEST_ASSERT_MSG_EQ_TOL(
        Simulator::Now().GetMicroSeconds(),
        m_expectedReportTime.GetMicroSeconds(), 1,
        "Report application not stopped at expected time");
  }
  NS_TEST_ASSERT_MSG_GT_OR_EQ(report.m_rttMin, 0,
                              "minimum rtt should be greater than 0");
  NS_TEST_ASSERT_MSG_GT_OR_EQ(report.m_rttMax, 0,
                              "maximum rtt should be greater than 0");
}

void PingTestCase::CheckTraceTx(uint32_t expectedTx) {
  m_checkTraceTx = true;
  m_expectedTraceTx = expectedTx;
}

void PingTestCase::CheckTraceRtt(uint32_t expectedRtt) {
  m_checkTraceRtt = true;
  m_expectedTraceRtt = expectedRtt;
}

void PingTestCase::SetSimulatorStopTime(Time stopTime) {
  m_simulatorStopTime = stopTime;
}

void PingTestCase::SetDestinationAddress(Address address) {
  m_destination = address;
}

void PingTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  Ptr<Ping> ping = CreateObject<Ping>();

  if (m_useIpv6) {
    Ipv6Address dst = Ipv6Address::ConvertFrom(m_destination);
    if (dst.IsLinkLocal() || dst.IsLinkLocalMulticast()) {
      NS_LOG_LOGIC("Setting local interface address to "
                   << m_ipv6Interfaces.GetAddress(0, 0));
      ping->SetAttribute("InterfaceAddress",
                         AddressValue(m_ipv6Interfaces.GetAddress(0, 0)));
    } else {
      NS_LOG_LOGIC("Setting local interface address to "
                   << m_ipv6Interfaces.GetAddress(0, 1));
      ping->SetAttribute("InterfaceAddress",
                         AddressValue(m_ipv6Interfaces.GetAddress(0, 1)));
    }
    NS_LOG_LOGIC("Setting destination address to "
                 << Ipv6Address::ConvertFrom(m_destination));
    ping->SetAttribute("Destination", AddressValue(m_destination));
  } else {
    NS_LOG_LOGIC("Setting destination address to "
                 << Ipv4Address::ConvertFrom(m_destination));
    ping->SetAttribute("InterfaceAddress",
                       AddressValue(m_ipv4Interfaces.GetAddress(0)));
    ping->SetAttribute("Destination", AddressValue(m_destination));
  }

  if (!m_dropList.empty()) {
    NS_LOG_LOGIC("Setting drop list to list of size " << m_dropList.size());
    Ptr<ReceiveListErrorModel> errorModel =
        CreateObject<ReceiveListErrorModel>();
    Ptr<SimpleNetDevice> netDev =
        DynamicCast<SimpleNetDevice>(m_devices.Get(0));
    netDev->SetReceiveErrorModel(errorModel);
    errorModel->SetList(m_dropList);
  }

  ping->SetAttribute("Count", UintegerValue(m_countAttribute));
  ping->SetAttribute("Size", UintegerValue(m_sizeAttribute));
  ping->SetAttribute("Interval", TimeValue(m_interpacketInterval));
  ping->SetStartTime(m_startTime);
  ping->SetStopTime(m_stopTime);
  m_nodes.Get(0)->AddApplication(ping);
  ping->TraceConnectWithoutContext(
      "Tx", MakeCallback(&PingTestCase::TxTraceSink, this));
  ping->TraceConnectWithoutContext(
      "Rtt", MakeCallback(&PingTestCase::RttTraceSink, this));
  ping->TraceConnectWithoutContext(
      "Drop", MakeCallback(&PingTestCase::DropTraceSink, this));
  ping->TraceConnectWithoutContext(
      "Report", MakeCallback(&PingTestCase::ReportTraceSink, this));

  NeighborCacheHelper neighborCacheHelper;
  neighborCacheHelper.PopulateNeighborCache();

  Simulator::Stop(m_simulatorStopTime);
  Simulator::Run();

  if (m_checkTraceTx) {
    NS_TEST_ASSERT_MSG_EQ(m_countTraceTx, m_expectedTraceTx,
                          "Traced Tx events do not equal expected");
  }
  if (m_checkTraceRtt) {
    NS_TEST_ASSERT_MSG_EQ(m_countTraceRtt, m_expectedTraceRtt,
                          "Traced Rtt events do not equal expected");
  }
}

class PingTestSuite : public TestSuite {
public:
  PingTestSuite();
};

PingTestSuite::PingTestSuite() : TestSuite("ping", UNIT) {
  auto testcase1v4 =
      new PingTestCase("1. Unlimited pings, no losses, StopApplication () with "
                       "no packets in flight IPv4",
                       USEIPV6_FALSE);
  testcase1v4->SetStartTime(Seconds(1));
  testcase1v4->SetStopTime(Seconds(5.5));
  testcase1v4->SetCount(0);
  testcase1v4->CheckReportTransmitted(5);
  testcase1v4->CheckReportReceived(5);
  testcase1v4->CheckTraceTx(5);
  testcase1v4->SetDestinationAddress(Ipv4Address("10.0.0.2"));
  AddTestCase(testcase1v4, TestCase::QUICK);

  auto testcase1v6 =
      new PingTestCase("1. Unlimited pings, no losses, StopApplication () with "
                       "no packets in flight IPv6",
                       USEIPV6_TRUE);
  testcase1v6->SetStartTime(Seconds(1));
  testcase1v6->SetStopTime(Seconds(5.5));
  testcase1v6->SetCount(0);
  testcase1v6->CheckReportTransmitted(5);
  testcase1v6->CheckReportReceived(5);
  testcase1v6->CheckTraceTx(5);
  testcase1v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:2"));
  AddTestCase(testcase1v6, TestCase::QUICK);

  auto testcase2v4 =
      new PingTestCase("2. Unlimited pings, no losses, StopApplication () with "
                       "1 packet in flight IPv4",
                       USEIPV6_FALSE);
  testcase2v4->SetStartTime(Seconds(1));
  testcase2v4->SetStopTime(Seconds(1.0001));
  testcase2v4->SetSimulatorStopTime(Seconds(5));
  testcase2v4->CheckReportTransmitted(1);
  testcase2v4->CheckReportReceived(0);
  testcase1v4->SetDestinationAddress(Ipv4Address("10.0.0.2"));
  AddTestCase(testcase2v4, TestCase::QUICK);

  auto testcase2v6 =
      new PingTestCase("2. Unlimited pings, no losses, StopApplication () with "
                       "1 packet in flight IPv6",
                       USEIPV6_TRUE);
  testcase2v6->SetStartTime(Seconds(1));
  testcase2v6->SetStopTime(Seconds(1.0001));
  testcase2v6->SetSimulatorStopTime(Seconds(5));
  testcase2v6->CheckReportTransmitted(1);
  testcase2v6->CheckReportReceived(0);
  testcase2v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:2"));
  AddTestCase(testcase2v6, TestCase::QUICK);

  uint32_t count = 2;
  uint32_t expectedTx = 2;
  auto testcase3v4 =
      new PingTestCase("3. Test for operation of count attribute and exit "
                       "time after all pings were received, IPv4",
                       USEIPV6_FALSE);
  testcase3v4->SetStartTime(Seconds(1));
  testcase3v4->SetStopTime(Seconds(5));
  testcase3v4->SetCount(count);
  testcase3v4->SetSimulatorStopTime(Seconds(6));
  testcase3v4->CheckReportTransmitted(2);
  testcase3v4->CheckReportReceived(2);
  testcase3v4->CheckReportTime(MicroSeconds(2020001));
  testcase3v4->CheckTraceTx(expectedTx);
  testcase3v4->SetDestinationAddress(Ipv4Address("10.0.0.2"));
  AddTestCase(testcase3v4, TestCase::QUICK);

  auto testcase3v6 =
      new PingTestCase("3. Test for operation of count attribute and exit "
                       "time after all pings were received, IPv6",
                       USEIPV6_TRUE);
  testcase3v6->SetStartTime(Seconds(1));
  testcase3v6->SetStopTime(Seconds(5));
  testcase3v6->SetCount(count);
  testcase3v6->SetSimulatorStopTime(Seconds(6));
  testcase3v6->CheckReportTransmitted(2);
  testcase3v6->CheckReportReceived(2);
  testcase3v6->CheckReportTime(MicroSeconds(2020001));
  testcase3v6->CheckTraceTx(expectedTx);
  testcase3v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:2"));
  AddTestCase(testcase3v6, TestCase::QUICK);

  Time interval = Seconds(3.0);
  auto testcase4v4 = new PingTestCase(
      "4. Test for the operation of interval attribute for IPv4",
      USEIPV6_FALSE);
  testcase4v4->SetStartTime(Seconds(1));
  testcase4v4->SetStopTime(Seconds(5));
  testcase4v4->SetInterval(interval);
  testcase4v4->SetSimulatorStopTime(Seconds(6));
  testcase4v4->CheckReportTransmitted(2);
  testcase4v4->CheckReportReceived(2);
  testcase4v4->SetDestinationAddress(Ipv4Address("10.0.0.2"));
  AddTestCase(testcase4v4, TestCase::QUICK);

  auto testcase4v6 = new PingTestCase(
      "4. Test for the operation of interval attribute for IPv6", USEIPV6_TRUE);
  testcase4v6->SetStartTime(Seconds(1));
  testcase4v6->SetStopTime(Seconds(5));
  testcase4v6->SetInterval(interval);
  testcase4v6->SetSimulatorStopTime(Seconds(6));
  testcase4v6->CheckReportTransmitted(2);
  testcase4v6->CheckReportReceived(2);
  testcase4v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:2"));
  AddTestCase(testcase4v6, TestCase::QUICK);

  auto testcase5v4 = new PingTestCase(
      "5. Test for behavior of ping to unreachable IPv4 address",
      USEIPV6_FALSE);
  testcase5v4->SetStartTime(Seconds(1));
  testcase5v4->SetStopTime(Seconds(5.5));
  testcase5v4->SetDestinationAddress(Ipv4Address("1.2.3.4"));
  testcase5v4->CheckReportTransmitted(5);
  testcase5v4->CheckReportReceived(0);
  testcase5v4->CheckReportLoss(100);
  AddTestCase(testcase5v4, TestCase::QUICK);

  auto testcase5v6 = new PingTestCase(
      "5. Test for behavior of ping to unreachable IPv6 address", USEIPV6_TRUE);
  testcase5v6->SetStartTime(Seconds(1));
  testcase5v6->SetStopTime(Seconds(5.5));
  testcase5v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:3"));
  testcase5v6->CheckReportTransmitted(5);
  testcase5v6->CheckReportReceived(0);
  testcase5v6->CheckReportLoss(100);
  AddTestCase(testcase5v6, TestCase::QUICK);

  auto testcase6v4 = new PingTestCase(
      "6. Test for behavior of ping to broadcast IPv4 address", USEIPV6_FALSE);
  testcase6v4->SetStartTime(Seconds(1));
  testcase6v4->SetStopTime(Seconds(5.5));
  testcase6v4->SetDestinationAddress(Ipv4Address::GetBroadcast());
  testcase6v4->CheckReportTransmitted(5);
  testcase6v4->CheckReportReceived(5);
  testcase6v4->CheckReportLoss(0);
  AddTestCase(testcase6v4, TestCase::QUICK);

  auto testcase6v6 = new PingTestCase(
      "6. Test for behavior of ping to all-nodes multicast IPv6 address",
      USEIPV6_TRUE);
  testcase6v6->SetStartTime(Seconds(1));
  testcase6v6->SetStopTime(Seconds(5.5));
  testcase6v6->SetDestinationAddress(Ipv6Address::GetAllNodesMulticast());
  testcase6v6->CheckReportTransmitted(5);
  testcase6v6->CheckReportReceived(5);
  testcase6v6->CheckReportLoss(0);
  AddTestCase(testcase6v6, TestCase::QUICK);

  auto testcase7v4 = new PingTestCase("7. Test behavior of first reply lost in "
                                      "a count-limited configuration, IPv4",
                                      USEIPV6_FALSE);
  std::list<uint32_t> dropList{0};
  testcase7v4->SetDropList(dropList);
  testcase7v4->SetStartTime(Seconds(1));
  testcase7v4->SetCount(3);
  testcase7v4->SetStopTime(Seconds(5));
  testcase7v4->CheckTraceTx(3);
  testcase7v4->CheckTraceRtt(2);
  testcase7v4->CheckReportTransmitted(3);
  testcase7v4->CheckReportReceived(2);
  testcase7v4->CheckReportLoss(33);
  testcase7v4->CheckReportTime(MicroSeconds(3040000));
  AddTestCase(testcase7v4, TestCase::QUICK);

  auto testcase7v6 = new PingTestCase("7. Test behavior of first reply lost in "
                                      "a count-limited configuration, IPv6",
                                      USEIPV6_TRUE);
  testcase7v6->SetDropList(dropList);
  testcase7v6->SetStartTime(Seconds(1));
  testcase7v6->SetCount(3);
  testcase7v6->SetStopTime(Seconds(5));
  testcase7v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:2"));
  testcase7v6->CheckTraceTx(3);
  testcase7v6->CheckTraceRtt(2);
  testcase7v6->CheckReportTransmitted(3);
  testcase7v6->CheckReportReceived(2);
  testcase7v6->CheckReportLoss(33);
  testcase7v6->CheckReportTime(MicroSeconds(3040000));
  AddTestCase(testcase7v6, TestCase::QUICK);

  auto testcase8v4 = new PingTestCase("8. Test behavior of second reply lost "
                                      "in a count-limited configuration, IPv4",
                                      USEIPV6_FALSE);
  std::list<uint32_t> dropList2{1};
  testcase8v4->SetDropList(dropList2);
  testcase8v4->SetStartTime(Seconds(1));
  testcase8v4->SetCount(3);
  testcase8v4->SetStopTime(Seconds(5));
  testcase8v4->CheckTraceTx(3);
  testcase8v4->CheckTraceRtt(2);
  testcase8v4->CheckReportTransmitted(3);
  testcase8v4->CheckReportReceived(2);
  testcase8v4->CheckReportLoss(33);
  testcase8v4->CheckReportTime(MicroSeconds(3040000));
  AddTestCase(testcase8v4, TestCase::QUICK);

  auto testcase8v6 = new PingTestCase("8. Test behavior of second reply lost "
                                      "in a count-limited configuration, IPv6",
                                      USEIPV6_TRUE);
  testcase8v6->SetDropList(dropList2);
  testcase8v6->SetStartTime(Seconds(1));
  testcase8v6->SetCount(3);
  testcase8v6->SetStopTime(Seconds(5));
  testcase8v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:2"));
  testcase8v6->CheckTraceTx(3);
  testcase8v6->CheckTraceRtt(2);
  testcase8v6->CheckReportTransmitted(3);
  testcase8v6->CheckReportReceived(2);
  testcase8v6->CheckReportLoss(33);
  testcase8v6->CheckReportTime(MicroSeconds(3040000));
  AddTestCase(testcase8v6, TestCase::QUICK);

  auto testcase9v4 = new PingTestCase("9. Test behavior of last reply lost in "
                                      "a count-limited configuration, IPv4",
                                      USEIPV6_FALSE);
  std::list<uint32_t> dropList3{2};
  testcase9v4->SetDropList(dropList3);
  testcase9v4->SetStartTime(Seconds(1));
  testcase9v4->SetCount(3);
  testcase9v4->SetStopTime(Seconds(5));
  testcase9v4->CheckTraceTx(3);
  testcase9v4->CheckTraceRtt(2);
  testcase9v4->CheckReportTransmitted(3);
  testcase9v4->CheckReportReceived(2);
  testcase9v4->CheckReportLoss(33);
  testcase9v4->CheckReportTime(MicroSeconds(3040000));
  AddTestCase(testcase9v4, TestCase::QUICK);

  auto testcase9v6 = new PingTestCase("9. Test behavior of last reply lost in "
                                      "a count-limited configuration, IPv6",
                                      USEIPV6_TRUE);
  testcase9v6->SetDropList(dropList3);
  testcase9v6->SetStartTime(Seconds(1));
  testcase9v6->SetCount(3);
  testcase9v6->SetStopTime(Seconds(5));
  testcase9v6->SetDestinationAddress(Ipv6Address("2001:1::200:ff:fe00:2"));
  testcase9v6->CheckTraceTx(3);
  testcase9v6->CheckTraceRtt(2);
  testcase9v6->CheckReportTransmitted(3);
  testcase9v6->CheckReportReceived(2);
  testcase9v6->CheckReportLoss(33);
  testcase9v6->CheckReportTime(MicroSeconds(3040000));
  AddTestCase(testcase9v6, TestCase::QUICK);

#ifdef NOTYET
  PingTestCase *testcase10v4 =
      new PingTestCase("10. Test for IPv4 fragmentation", USEIPV6_FALSE);
  testcase10v4->SetStartTime(Seconds(1));
  testcase10v4->SetStopTime(Seconds(2.5));
  testcase10v4->SetCount(1);
  testcase10v4->SetSize(2000);
  testcase10v4->CheckReportTransmitted(1);
  testcase10v4->CheckReportReceived(1);
  testcase10v4->CheckReportTime(MicroSeconds(1020028));
  AddTestCase(testcase10v4, TestCase::QUICK);
#endif
}

static PingTestSuite pingTestSuite;
