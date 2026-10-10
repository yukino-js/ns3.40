

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/csma-star-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaStar");

int main(int argc, char *argv[]) {
  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(137));

  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("14kb/s"));

  uint32_t nSpokes = 7;
  uint32_t useIpv6 = 0;
  Ipv6Address ipv6AddressBase = Ipv6Address("2001::");
  Ipv6Prefix ipv6AddressPrefix = Ipv6Prefix(64);

  CommandLine cmd(__FILE__);
  cmd.AddValue("nSpokes", "Number of spoke nodes to place in the star",
               nSpokes);
  cmd.AddValue("useIpv6", "Use Ipv6", useIpv6);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Build star topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay", StringValue("1ms"));
  CsmaStarHelper star(nSpokes, csma);

  NodeContainer fillNodes;

  NetDeviceContainer fillDevices;

  uint32_t nFill = 14;
  for (uint32_t i = 0; i < star.GetSpokeDevices().GetN(); ++i) {
    Ptr<Channel> channel = star.GetSpokeDevices().Get(i)->GetChannel();
    Ptr<CsmaChannel> csmaChannel = channel->GetObject<CsmaChannel>();
    NodeContainer newNodes;
    newNodes.Create(nFill);
    fillNodes.Add(newNodes);
    fillDevices.Add(csma.Install(newNodes, csmaChannel));
  }

  NS_LOG_INFO("Install internet stack on all nodes.");
  InternetStackHelper internet;
  star.InstallStack(internet);
  internet.Install(fillNodes);

  NS_LOG_INFO("Assign IP Addresses.");
  if (useIpv6 == 0) {
    star.AssignIpv4Addresses(Ipv4AddressHelper("10.1.0.0", "255.255.255.0"));
  } else {
    star.AssignIpv6Addresses(ipv6AddressBase, ipv6AddressPrefix);
  }

  Ipv4AddressHelper address;
  Ipv6AddressHelper address6;
  for (uint32_t i = 0; i < star.SpokeCount(); ++i) {
    if (useIpv6 == 0) {
      std::ostringstream subnet;
      subnet << "10.1." << i << ".0";
      NS_LOG_INFO("Assign IP Addresses for CSMA subnet " << subnet.str());
      address.SetBase(subnet.str().c_str(), "255.255.255.0", "0.0.0.3");

      for (uint32_t j = 0; j < nFill; ++j) {
        address.Assign(fillDevices.Get(i * nFill + j));
      }
    } else {
      Ipv6AddressGenerator::Init(ipv6AddressBase, ipv6AddressPrefix);
      Ipv6Address v6network =
          Ipv6AddressGenerator::GetNetwork(ipv6AddressPrefix);
      address6.SetBase(v6network, ipv6AddressPrefix);

      for (uint32_t j = 0; j < nFill; ++j) {
        address6.Assign(fillDevices.Get(i * nFill + j));
      }
    }
  }

  NS_LOG_INFO("Create applications.");
  uint16_t port = 50000;

  if (useIpv6 == 0) {
    Address hubLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", hubLocalAddress);
    ApplicationContainer hubApp = packetSinkHelper.Install(star.GetHub());
    hubApp.Start(Seconds(1.0));
    hubApp.Stop(Seconds(10.0));
  } else {
    Address hubLocalAddress6(Inet6SocketAddress(Ipv6Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper6("ns3::TcpSocketFactory",
                                       hubLocalAddress6);
    ApplicationContainer hubApp6 = packetSinkHelper6.Install(star.GetHub());
    hubApp6.Start(Seconds(1.0));
    hubApp6.Stop(Seconds(10.0));
  }

  OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
  onOffHelper.SetAttribute(
      "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute(
      "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer spokeApps;

  for (uint32_t i = 0; i < star.SpokeCount(); ++i) {
    if (useIpv6 == 0) {
      AddressValue remoteAddress(
          InetSocketAddress(star.GetHubIpv4Address(i), port));
      onOffHelper.SetAttribute("Remote", remoteAddress);
    } else {
      AddressValue remoteAddress(
          Inet6SocketAddress(star.GetHubIpv6Address(i), port));
      onOffHelper.SetAttribute("Remote", remoteAddress);
    }
    spokeApps.Add(onOffHelper.Install(star.GetSpokeNode(i)));
  }

  spokeApps.Start(Seconds(1.0));
  spokeApps.Stop(Seconds(10.0));

  ApplicationContainer fillApps;

  for (uint32_t i = 0; i < fillNodes.GetN(); ++i) {
    AddressValue remoteAddress;
    if (useIpv6 == 0) {
      remoteAddress = AddressValue(
          InetSocketAddress(star.GetHubIpv4Address(i / nFill), port));
    } else {
      remoteAddress = AddressValue(
          Inet6SocketAddress(star.GetHubIpv6Address(i / nFill), port));
    }
    onOffHelper.SetAttribute("Remote", remoteAddress);
    fillApps.Add(onOffHelper.Install(fillNodes.Get(i)));
  }

  fillApps.Start(Seconds(1.0));
  fillApps.Stop(Seconds(10.0));

  NS_LOG_INFO("Enable static global routing.");
  if (useIpv6 == 0) {
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  }

  NS_LOG_INFO("Enable pcap tracing.");
  csma.EnablePcapAll("csma-star", false);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
