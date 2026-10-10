
#include "ns3/data-rate.h"
#include "ns3/dhcp-client.h"
#include "ns3/dhcp-helper.h"
#include "ns3/dhcp-server.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/test.h"

using namespace ns3;

class DhcpTestCase : public TestCase {
public:
  DhcpTestCase();
  ~DhcpTestCase() override;
  void LeaseObtained(std::string context, const Ipv4Address &newAddress);

private:
  void DoRun() override;
  Ipv4Address m_leasedAddress[3];
};

DhcpTestCase::DhcpTestCase() : TestCase("Dhcp test case ") {}

DhcpTestCase::~DhcpTestCase() {}

void DhcpTestCase::LeaseObtained(std::string context,
                                 const Ipv4Address &newAddress) {
  uint8_t numericalContext = std::stoi(context, nullptr, 10);

  if (numericalContext >= 0 && numericalContext <= 2) {
    m_leasedAddress[numericalContext] = newAddress;
  }
}

void DhcpTestCase::DoRun() {
  NodeContainer nodes;
  NodeContainer routers;
  nodes.Create(3);
  routers.Create(1);

  NodeContainer net(routers, nodes);

  SimpleNetDeviceHelper simpleNetDevice;
  simpleNetDevice.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  simpleNetDevice.SetDeviceAttribute("DataRate",
                                     DataRateValue(DataRate("5Mbps")));
  NetDeviceContainer devNet = simpleNetDevice.Install(net);

  InternetStackHelper tcpip;
  tcpip.Install(routers);
  tcpip.Install(nodes);

  DhcpHelper dhcpHelper;

  ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer(
      devNet.Get(0), Ipv4Address("172.30.0.12"), Ipv4Address("172.30.0.0"),
      Ipv4Mask("/24"), Ipv4Address("172.30.0.10"), Ipv4Address("172.30.0.15"),
      Ipv4Address("172.30.0.17"));
  dhcpServerApp.Start(Seconds(0.0));
  dhcpServerApp.Stop(Seconds(20.0));

  DynamicCast<DhcpServer>(dhcpServerApp.Get(0))
      ->AddStaticDhcpEntry(devNet.Get(3)->GetAddress(),
                           Ipv4Address("172.30.0.14"));

  NetDeviceContainer dhcpClientNetDevs;
  dhcpClientNetDevs.Add(devNet.Get(1));
  dhcpClientNetDevs.Add(devNet.Get(2));
  dhcpClientNetDevs.Add(devNet.Get(3));

  ApplicationContainer dhcpClientApps =
      dhcpHelper.InstallDhcpClient(dhcpClientNetDevs);
  dhcpClientApps.Start(Seconds(1.0));
  dhcpClientApps.Stop(Seconds(20.0));

  dhcpClientApps.Get(0)->TraceConnect(
      "NewLease", "0", MakeCallback(&DhcpTestCase::LeaseObtained, this));
  dhcpClientApps.Get(1)->TraceConnect(
      "NewLease", "1", MakeCallback(&DhcpTestCase::LeaseObtained, this));
  dhcpClientApps.Get(2)->TraceConnect(
      "NewLease", "2", MakeCallback(&DhcpTestCase::LeaseObtained, this));

  Simulator::Stop(Seconds(21.0));

  Simulator::Run();

  NS_TEST_ASSERT_MSG_EQ(m_leasedAddress[0], Ipv4Address("172.30.0.10"),
                        m_leasedAddress[0] << " instead of "
                                           << "172.30.0.10");

  NS_TEST_ASSERT_MSG_EQ(m_leasedAddress[1], Ipv4Address("172.30.0.11"),
                        m_leasedAddress[1] << " instead of "
                                           << "172.30.0.11");

  NS_TEST_ASSERT_MSG_EQ(m_leasedAddress[2], Ipv4Address("172.30.0.14"),
                        m_leasedAddress[2] << " instead of "
                                           << "172.30.0.14");

  Simulator::Destroy();
}

class DhcpTestSuite : public TestSuite {
public:
  DhcpTestSuite();
};

DhcpTestSuite::DhcpTestSuite() : TestSuite("dhcp", UNIT) {
  AddTestCase(new DhcpTestCase, TestCase::QUICK);
}

static DhcpTestSuite dhcpTestSuite;
