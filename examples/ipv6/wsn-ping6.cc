

#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/sixlowpan-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Ping6WsnExample");

int main(int argc, char **argv) {
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "turn on log components", verbose);
  cmd.Parse(argc, argv);

  if (verbose) {
    LogComponentEnable("Ping6WsnExample", LOG_LEVEL_INFO);
    LogComponentEnable("Ipv6EndPointDemux", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6L3Protocol", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6StaticRouting", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6ListRouting", LOG_LEVEL_ALL);
    LogComponentEnable("Ipv6Interface", LOG_LEVEL_ALL);
    LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_ALL);
    LogComponentEnable("Ping", LOG_LEVEL_ALL);
    LogComponentEnable("NdiscCache", LOG_LEVEL_ALL);
    LogComponentEnable("SixLowPanNetDevice", LOG_LEVEL_ALL);
  }

  NS_LOG_INFO("Create nodes.");
  NodeContainer nodes;
  nodes.Create(2);

  SeedManager::SetSeed(167);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  Ptr<ListPositionAllocator> nodesPositionAlloc =
      CreateObject<ListPositionAllocator>();
  nodesPositionAlloc->Add(Vector(0.0, 0.0, 0.0));
  nodesPositionAlloc->Add(Vector(50.0, 0.0, 0.0));
  mobility.SetPositionAllocator(nodesPositionAlloc);
  mobility.Install(nodes);

  NS_LOG_INFO("Create channels.");
  LrWpanHelper lrWpanHelper;
  NetDeviceContainer devContainer = lrWpanHelper.Install(nodes);
  lrWpanHelper.CreateAssociatedPan(devContainer, 10);

  std::cout << "Created " << devContainer.GetN() << " devices" << std::endl;
  std::cout << "There are " << nodes.GetN() << " nodes" << std::endl;

  NS_LOG_INFO("Install Internet stack.");
  InternetStackHelper internetv6;
  internetv6.SetIpv4StackInstall(false);
  internetv6.Install(nodes);

  NS_LOG_INFO("Install 6LoWPAN.");
  SixLowPanHelper sixlowpan;
  NetDeviceContainer six1 = sixlowpan.Install(devContainer);

  NS_LOG_INFO("Assign addresses.");
  Ipv6AddressHelper ipv6;
  ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
  Ipv6InterfaceContainer i = ipv6.Assign(six1);

  NS_LOG_INFO("Create Applications.");

  uint32_t packetSize = 16;
  uint32_t maxPacketCount = 5;
  Time interPacketInterval = Seconds(1.);
  PingHelper ping(i.GetAddress(1, 1));

  ping.SetAttribute("Count", UintegerValue(maxPacketCount));
  ping.SetAttribute("Interval", TimeValue(interPacketInterval));
  ping.SetAttribute("Size", UintegerValue(packetSize));
  ApplicationContainer apps = ping.Install(nodes.Get(0));
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(10.0));

  AsciiTraceHelper ascii;
  lrWpanHelper.EnableAsciiAll(ascii.CreateFileStream("ping6wsn.tr"));
  lrWpanHelper.EnablePcapAll(std::string("ping6wsn"), true);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
