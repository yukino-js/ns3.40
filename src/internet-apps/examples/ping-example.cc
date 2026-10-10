

#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/nix-vector-routing-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PingExample");

int main(int argc, char *argv[]) {
  bool logging{false};
  Time interPacketInterval{Seconds(1.0)};
  uint32_t size{56};
  uint32_t count{5};
  std::string destinationStr;
  Address destination;
  std::string sourceStr;
  Address source;
  bool useIpv6{true};

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  CommandLine cmd(__FILE__);
  cmd.AddValue("logging", "Tell application to log if true", logging);
  cmd.AddValue("interval", "The time to wait between two packets",
               interPacketInterval);
  cmd.AddValue("size", "Data bytes to be sent, per-packet", size);
  cmd.AddValue("count", "Number of packets to be sent", count);
  cmd.AddValue("destination",
               "Destination IPv4 or IPv6 address, e.g., \"10.1.2.2\"",
               destinationStr);
  cmd.AddValue(
      "source",
      "Source address, needed only for multicast or broadcast destinations",
      sourceStr);
  cmd.Parse(argc, argv);

  if (!destinationStr.empty()) {
    Ipv4Address v4Dst(destinationStr.c_str());
    Ipv6Address v6Dst(destinationStr.c_str());
    if (v4Dst.IsInitialized()) {
      useIpv6 = false;
      destination = v4Dst;
    } else if (v6Dst.IsInitialized()) {
      useIpv6 = true;
      destination = v6Dst;
    }
  }

  if (!sourceStr.empty()) {
    Ipv4Address v4Src(sourceStr.c_str());
    Ipv6Address v6Src(sourceStr.c_str());
    if (v4Src.IsInitialized()) {
      source = v4Src;
    } else if (v6Src.IsInitialized()) {
      source = v6Src;
    }
  }
  if (sourceStr.empty()) {
    if (useIpv6) {
      Ipv6Address v6Dst = Ipv6Address(destinationStr.c_str());
      if (v6Dst.IsInitialized() && v6Dst.IsMulticast()) {
        std::cout << "Specify a source address to use when pinging multicast "
                     "addresses"
                  << std::endl;
        std::cout << "Program exiting..." << std::endl;
        return 0;
      }
    } else {
      Ipv4Address v4Dst(destinationStr.c_str());
      if (v4Dst.IsInitialized() &&
          (v4Dst.IsBroadcast() || v4Dst.IsMulticast())) {
        std::cout << "Specify a source address to use when pinging broadcast "
                     "or multicast "
                     "addresses"
                  << std::endl;
        std::cout << "Program exiting..." << std::endl;
        return 0;
      }
    }
  }

  if (logging) {
    LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_NODE));
    LogComponentEnable("Ping", LOG_LEVEL_ALL);
  }

  NodeContainer nodes;
  nodes.Create(3);
  NodeContainer link1Nodes;
  link1Nodes.Add(nodes.Get(0));
  link1Nodes.Add(nodes.Get(1));
  NodeContainer link2Nodes;
  link2Nodes.Add(nodes.Get(1));
  link2Nodes.Add(nodes.Get(2));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("5ms"));

  NetDeviceContainer link1Devices;
  link1Devices = pointToPoint.Install(link1Nodes);
  NetDeviceContainer link2Devices;
  link2Devices = pointToPoint.Install(link2Nodes);

  Ptr<PointToPointNetDevice> p2pSender =
      DynamicCast<PointToPointNetDevice>(link1Devices.Get(0));
  Ptr<ReceiveListErrorModel> errorModel = CreateObject<ReceiveListErrorModel>();
  std::list<uint32_t> dropList;
  errorModel->SetList(dropList);
  p2pSender->SetReceiveErrorModel(errorModel);

  if (!useIpv6) {
    Ipv4NixVectorHelper nixRouting;
    InternetStackHelper stack;
    stack.SetRoutingHelper(nixRouting);
    stack.SetIpv6StackInstall(false);
    stack.Install(nodes);

    Ipv4AddressHelper addressV4;
    addressV4.SetBase("10.1.1.0", "255.255.255.0");
    addressV4.Assign(link1Devices);
    addressV4.NewNetwork();
    Ipv4InterfaceContainer link2InterfacesV4 = addressV4.Assign(link2Devices);

    if (destination.IsInvalid()) {
      destination = link2InterfacesV4.GetAddress(1, 0);
    }
  } else {
    Ipv6NixVectorHelper nixRouting;
    InternetStackHelper stack;
    stack.SetRoutingHelper(nixRouting);
    stack.SetIpv4StackInstall(false);
    stack.Install(nodes);

    Ipv6AddressHelper addressV6;
    addressV6.SetBase("2001:1::", 64);
    addressV6.Assign(link1Devices);
    addressV6.NewNetwork();
    Ipv6InterfaceContainer link2InterfacesV6 = addressV6.Assign(link2Devices);

    if (destination.IsInvalid()) {
      destination = link2InterfacesV6.GetAddress(1, 1);
    }
  }

  PingHelper pingHelper(destination, source);
  pingHelper.SetAttribute("Interval", TimeValue(interPacketInterval));
  pingHelper.SetAttribute("Size", UintegerValue(size));
  pingHelper.SetAttribute("Count", UintegerValue(count));
  ApplicationContainer apps = pingHelper.Install(nodes.Get(0));
  apps.Start(Seconds(1));
  apps.Stop(Seconds(50));

  pointToPoint.EnablePcapAll("ping-example");

  Simulator::Stop(Seconds(60.0));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
