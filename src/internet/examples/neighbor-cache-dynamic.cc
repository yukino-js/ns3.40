

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NeighborCacheDynamic");

void AddIpv4Address(Ptr<Ipv4Interface> ipv4Interface,
                    Ipv4InterfaceAddress ifaceAddr) {
  ipv4Interface->AddAddress(ifaceAddr);
  std::cout << "\nArp caches after add address 10.1.1.4 to n1" << std::endl;
}

void AddIpv6Address(Ptr<Ipv6Interface> ipv6Interface,
                    Ipv6InterfaceAddress ifaceAddr) {
  ipv6Interface->AddAddress(ifaceAddr);
  std::cout << "\nNdisc caches after add address 2001:1::200:ff:fe00:4 n1"
            << std::endl;
}

void RemoveIpv4Address(Ptr<Ipv4Interface> ipv4Interface, uint32_t index) {
  ipv4Interface->RemoveAddress(index);
  std::cout << "\nArp caches after remove the first address (10.1.1.1) from n1"
            << std::endl;
}

void RemoveIpv6Address(Ptr<Ipv6Interface> ipv6Interface, uint32_t index) {
  ipv6Interface->RemoveAddress(index);
  std::cout << "\nNdisc caches after remove the second address "
               "(2001:1::200:ff:fe00:1) from n1"
            << std::endl;
}

int main(int argc, char *argv[]) {
  bool useIpv6 = false;
  bool enableLog = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("useIPv6", "Use IPv6 instead of IPv4", useIpv6);
  cmd.AddValue("enableLog", "Enable ArpL3Protocol and Icmpv6L4Protocol logging",
               enableLog);
  cmd.Parse(argc, argv);

  if (enableLog) {
    LogComponentEnable("ArpL3Protocol", LOG_LEVEL_LOGIC);
    LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_LOGIC);
  }

  uint32_t nCsma = 3;
  NodeContainer csmaNodes;
  csmaNodes.Create(nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  InternetStackHelper stack;
  if (!useIpv6) {
    stack.SetIpv6StackInstall(false);
  } else {
    stack.SetIpv4StackInstall(false);
  }
  stack.Install(csmaNodes);

  if (!useIpv6) {
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);
  } else {
    Ipv6AddressHelper address;
    address.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);
  }

  NeighborCacheHelper neighborCache;
  neighborCache.SetDynamicNeighborCache(true);
  neighborCache.PopulateNeighborCache();

  if (!useIpv6) {
    Ptr<Node> n1 = csmaNodes.Get(0);
    uint32_t ipv4ifIndex = 1;
    Ptr<Ipv4Interface> ipv4Interface =
        n1->GetObject<Ipv4L3Protocol>()->GetInterface(ipv4ifIndex);
    Ipv4InterfaceAddress ifaceAddr =
        Ipv4InterfaceAddress("10.1.1.4", "255.255.255.0");
    Simulator::Schedule(Seconds(0.5), &AddIpv4Address, ipv4Interface,
                        ifaceAddr);

    uint32_t addressIndex = 0;
    Simulator::Schedule(Seconds(1.5), &RemoveIpv4Address, ipv4Interface,
                        addressIndex);

    Ptr<OutputStreamWrapper> outputStream =
        Create<OutputStreamWrapper>(&std::cout);
    Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), outputStream);
    Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(1), outputStream);
    Ipv4RoutingHelper::PrintNeighborCacheAllAt(Seconds(2), outputStream);
  } else {
    Ptr<Node> n1 = csmaNodes.Get(0);
    uint32_t ipv6ifIndex = 1;
    Ptr<Ipv6Interface> ipv6Interface =
        n1->GetObject<Ipv6L3Protocol>()->GetInterface(ipv6ifIndex);
    Ipv6InterfaceAddress ifaceAddr =
        Ipv6InterfaceAddress("2001:1::200:ff:fe00:4", Ipv6Prefix(64));
    Simulator::Schedule(Seconds(0.5), &AddIpv6Address, ipv6Interface,
                        ifaceAddr);

    uint32_t addressIndex = 1;
    Simulator::Schedule(Seconds(1.5), &RemoveIpv6Address, ipv6Interface,
                        addressIndex);

    Ptr<OutputStreamWrapper> outputStream =
        Create<OutputStreamWrapper>(&std::cout);
    Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(0), outputStream);
    Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(1), outputStream);
    Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(2), outputStream);
  }

  Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
