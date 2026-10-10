

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpServer");

int main(int argc, char *argv[]) {

  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(250));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("5kb/s"));
  uint32_t N = 9;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nNodes", "Number of nodes to place in the star", N);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer serverNode;
  NodeContainer clientNodes;
  serverNode.Create(1);
  clientNodes.Create(N - 1);
  NodeContainer allNodes = NodeContainer(serverNode, clientNodes);

  InternetStackHelper internet;
  internet.Install(allNodes);

  std::vector<NodeContainer> nodeAdjacencyList(N - 1);
  for (uint32_t i = 0; i < nodeAdjacencyList.size(); ++i) {
    nodeAdjacencyList[i] = NodeContainer(serverNode, clientNodes.Get(i));
  }

  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  std::vector<NetDeviceContainer> deviceAdjacencyList(N - 1);
  for (uint32_t i = 0; i < deviceAdjacencyList.size(); ++i) {
    deviceAdjacencyList[i] = p2p.Install(nodeAdjacencyList[i]);
  }

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList(N - 1);
  for (uint32_t i = 0; i < interfaceAdjacencyList.size(); ++i) {
    std::ostringstream subnet;
    subnet << "10.1." << i + 1 << ".0";
    ipv4.SetBase(subnet.str().c_str(), "255.255.255.0");
    interfaceAdjacencyList[i] = ipv4.Assign(deviceAdjacencyList[i]);
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  uint16_t port = 50000;
  Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install(serverNode);
  sinkApp.Start(Seconds(1.0));
  sinkApp.Stop(Seconds(10.0));

  OnOffHelper clientHelper("ns3::TcpSocketFactory", Address());
  clientHelper.SetAttribute(
      "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper.SetAttribute(
      "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer clientApps;
  for (uint32_t i = 0; i < clientNodes.GetN(); ++i) {
    AddressValue remoteAddress(
        InetSocketAddress(interfaceAdjacencyList[i].GetAddress(0), port));
    clientHelper.SetAttribute("Remote", remoteAddress);
    clientApps.Add(clientHelper.Install(clientNodes.Get(i)));
  }
  clientApps.Start(Seconds(1.0));
  clientApps.Stop(Seconds(10.0));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("tcp-star-server.tr"));
  p2p.EnablePcapAll("tcp-star-server");

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
