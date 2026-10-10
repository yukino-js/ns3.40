

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/radvd-interface.h"
#include "ns3/radvd-prefix.h"
#include "ns3/radvd.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RadvdTwoPrefixExample");

class IpAddressHelper {
public:
  inline void PrintIpAddresses(Ptr<Node> &n) {
    Ptr<Ipv6> ipv6 = n->GetObject<Ipv6>();
    uint32_t nInterfaces = ipv6->GetNInterfaces();

    std::cout << "Node: " << ipv6->GetObject<Node>()->GetId()
              << " Time: " << Simulator::Now().GetSeconds() << "s "
              << "IPv6 addresses" << std::endl;
    std::cout << "(Interface index, Address index)\t"
              << "IPv6 Address" << std::endl;

    for (uint32_t i = 0; i < nInterfaces; i++) {
      for (uint32_t j = 0; j < ipv6->GetNAddresses(i); j++) {
        std::cout << "(" << int(i) << "," << int(j) << ")\t"
                  << ipv6->GetAddress(i, j) << std::endl;
      }
    }
    std::cout << std::endl;
  }
};

int main(int argc, char **argv) {
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "turn on log components", verbose);
  cmd.Parse(argc, argv);

  if (verbose) {
    LogComponentEnable("Ipv6L3Protocol", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6RawSocketImpl", LOG_LEVEL_ALL);
    LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6StaticRouting", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6Interface", LOG_LEVEL_ALL);
    LogComponentEnable("RadvdApplication", LOG_LEVEL_ALL);
    LogComponentEnable("Ping", LOG_LEVEL_ALL);
  }

  NS_LOG_INFO("Create nodes.");
  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> r = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();

  NodeContainer net1(n0, r);
  NodeContainer net2(r, n1);
  NodeContainer all(n0, r, n1);

  NS_LOG_INFO("Create IPv6 Internet Stack");
  InternetStackHelper internetv6;
  internetv6.Install(all);

  NS_LOG_INFO("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  NetDeviceContainer d1 = csma.Install(net1);
  NetDeviceContainer d2 = csma.Install(net2);

  NS_LOG_INFO("Create networks and assign IPv6 Addresses.");
  Ipv6AddressHelper ipv6;

  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  NetDeviceContainer tmp;
  tmp.Add(d1.Get(0));
  Ipv6InterfaceContainer iic1 = ipv6.AssignWithoutAddress(tmp);

  NetDeviceContainer tmp2;
  tmp2.Add(d1.Get(1));
  Ipv6InterfaceContainer iicr1 = ipv6.Assign(tmp2);
  iicr1.SetForwarding(0, true);
  iic1.Add(iicr1);

  ipv6.SetBase(Ipv6Address("2001:ABCD::"), Ipv6Prefix(64));
  ipv6.Assign(tmp2);

  ipv6.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
  NetDeviceContainer tmp3;
  tmp3.Add(d2.Get(0));
  Ipv6InterfaceContainer iicr2 = ipv6.Assign(tmp3);
  iicr2.SetForwarding(0, true);

  NetDeviceContainer tmp4;
  tmp4.Add(d2.Get(1));
  Ipv6InterfaceContainer iic2 = ipv6.AssignWithoutAddress(tmp4);
  iic2.Add(iicr2);

  RadvdHelper radvdHelper;
  radvdHelper.AddAnnouncedPrefix(iic1.GetInterfaceIndex(1),
                                 Ipv6Address("2001:ABCD::0"), 64);
  radvdHelper.AddAnnouncedPrefix(iic1.GetInterfaceIndex(1),
                                 Ipv6Address("2001:1::0"), 64);

  Ptr<RadvdInterface> routerInterface =
      radvdHelper.GetRadvdInterface(iic1.GetInterfaceIndex(1));
  routerInterface->SetMaxRtrAdvInterval(2000);
  routerInterface->SetMinRtrAdvInterval(1000);
  RadvdInterface::RadvdPrefixList prefixList = routerInterface->GetPrefixes();
  for (auto iter = prefixList.begin(); iter != prefixList.end(); iter++) {
    (*iter)->SetPreferredLifeTime(3);
    (*iter)->SetValidLifeTime(5);
  }

  radvdHelper.AddAnnouncedPrefix(iic2.GetInterfaceIndex(1),
                                 Ipv6Address("2001:2::0"), 64);

  routerInterface = radvdHelper.GetRadvdInterface(iic2.GetInterfaceIndex(1));
  routerInterface->SetMaxRtrAdvInterval(2000);
  routerInterface->SetMinRtrAdvInterval(1000);
  prefixList = routerInterface->GetPrefixes();
  for (auto iter = prefixList.begin(); iter != prefixList.end(); iter++) {
    (*iter)->SetPreferredLifeTime(3);
    (*iter)->SetValidLifeTime(5);
  }

  ApplicationContainer radvdApps = radvdHelper.Install(r);
  radvdApps.Start(Seconds(1.0));
  radvdApps.Stop(Seconds(2.0));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 8;
  PingHelper ping(Ipv6Address("2001:2::200:ff:fe00:4"));

  ping.SetAttribute("Count", UintegerValue(maxPacketCount));
  ping.SetAttribute("Size", UintegerValue(packetSize));
  ApplicationContainer apps = ping.Install(net1.Get(0));
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(5.0));

  Ptr<OutputStreamWrapper> routingStream =
      Create<OutputStreamWrapper>(&std::cout);
  Ipv6RoutingHelper::PrintRoutingTableAt(Seconds(2.0), n0, routingStream);
  Ipv6RoutingHelper::PrintRoutingTableAt(Seconds(10.0), n0, routingStream);

  IpAddressHelper ipAddressHelper;
  Simulator::Schedule(Seconds(2.0), &IpAddressHelper::PrintIpAddresses,
                      &ipAddressHelper, n0);
  Simulator::Schedule(Seconds(10.0), &IpAddressHelper::PrintIpAddresses,
                      &ipAddressHelper, n0);

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("radvd-two-prefix.tr"));
  csma.EnablePcapAll(std::string("radvd-two-prefix"), true);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
