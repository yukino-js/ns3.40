

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("StaticRoutingSlash32Test");

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  Ptr<Node> nA = CreateObject<Node>();
  Ptr<Node> nB = CreateObject<Node>();
  Ptr<Node> nC = CreateObject<Node>();

  NodeContainer c = NodeContainer(nA, nB, nC);

  InternetStackHelper internet;
  internet.Install(c);

  NodeContainer nAnB = NodeContainer(nA, nB);
  NodeContainer nBnC = NodeContainer(nB, nC);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer dAdB = p2p.Install(nAnB);

  NetDeviceContainer dBdC = p2p.Install(nBnC);

  Ptr<CsmaNetDevice> deviceA = CreateObject<CsmaNetDevice>();
  deviceA->SetAddress(Mac48Address::Allocate());
  nA->AddDevice(deviceA);
  deviceA->SetQueue(CreateObject<DropTailQueue<Packet>>());

  Ptr<CsmaNetDevice> deviceC = CreateObject<CsmaNetDevice>();
  deviceC->SetAddress(Mac48Address::Allocate());
  nC->AddDevice(deviceC);
  deviceC->SetQueue(CreateObject<DropTailQueue<Packet>>());

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer iAiB = ipv4.Assign(dAdB);

  ipv4.SetBase("10.1.1.4", "255.255.255.252");
  Ipv4InterfaceContainer iBiC = ipv4.Assign(dBdC);

  Ptr<Ipv4> ipv4A = nA->GetObject<Ipv4>();
  Ptr<Ipv4> ipv4B = nB->GetObject<Ipv4>();
  Ptr<Ipv4> ipv4C = nC->GetObject<Ipv4>();

  int32_t ifIndexA = ipv4A->AddInterface(deviceA);
  int32_t ifIndexC = ipv4C->AddInterface(deviceC);

  Ipv4InterfaceAddress ifInAddrA =
      Ipv4InterfaceAddress(Ipv4Address("172.16.1.1"), Ipv4Mask("/32"));
  ipv4A->AddAddress(ifIndexA, ifInAddrA);
  ipv4A->SetMetric(ifIndexA, 1);
  ipv4A->SetUp(ifIndexA);

  Ipv4InterfaceAddress ifInAddrC =
      Ipv4InterfaceAddress(Ipv4Address("192.168.1.1"), Ipv4Mask("/32"));
  ipv4C->AddAddress(ifIndexC, ifInAddrC);
  ipv4C->SetMetric(ifIndexC, 1);
  ipv4C->SetUp(ifIndexC);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRoutingA =
      ipv4RoutingHelper.GetStaticRouting(ipv4A);
  staticRoutingA->AddHostRouteTo(Ipv4Address("192.168.1.1"),
                                 Ipv4Address("10.1.1.2"), 1);
  Ptr<Ipv4StaticRouting> staticRoutingB =
      ipv4RoutingHelper.GetStaticRouting(ipv4B);
  staticRoutingB->AddHostRouteTo(Ipv4Address("192.168.1.1"),
                                 Ipv4Address("10.1.1.6"), 2);
  uint16_t port = 9;
  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(ifInAddrC.GetLocal(), port)));
  onoff.SetConstantRate(DataRate(6000));
  ApplicationContainer apps = onoff.Install(nA);
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sink.Install(nC);
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("static-routing-slash32.tr"));
  p2p.EnablePcapAll("static-routing-slash32");

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
