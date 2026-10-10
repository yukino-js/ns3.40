

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaPacketSocketExample");

std::ofstream g_os;

static void SinkRx(std::string path, Ptr<const Packet> p,
                   const Address &address) {
  g_os << p->GetSize() << std::endl;
}

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("CsmaPacketSocketExample", LOG_LEVEL_INFO);
#endif

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  g_os.open("csma-packet-socket-sink.tr",
            std::ios_base::binary | std::ios_base::out);

  NS_LOG_INFO("Create nodes.");
  NodeContainer nodes;
  nodes.Create(4);

  PacketSocketHelper packetSocket;
  packetSocket.Install(nodes);

  NS_LOG_INFO("Create channels.");
  Ptr<CsmaChannel> channel = CreateObjectWithAttributes<CsmaChannel>(
      "DataRate", DataRateValue(DataRate(5000000)), "Delay",
      TimeValue(MilliSeconds(2)));

  NS_LOG_INFO("Build Topology.");
  CsmaHelper csma;
  csma.SetDeviceAttribute("EncapsulationMode", StringValue("Llc"));
  NetDeviceContainer devs = csma.Install(nodes, channel);

  NS_LOG_INFO("Create Applications.");
  PacketSocketAddress socket;
  socket.SetSingleDevice(devs.Get(0)->GetIfIndex());
  socket.SetPhysicalAddress(devs.Get(1)->GetAddress());
  socket.SetProtocol(2);
  OnOffHelper onoff("ns3::PacketSocketFactory", Address(socket));
  onoff.SetConstantRate(DataRate("500kb/s"));
  ApplicationContainer apps = onoff.Install(nodes.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  socket.SetSingleDevice(devs.Get(3)->GetIfIndex());
  socket.SetPhysicalAddress(devs.Get(0)->GetAddress());
  socket.SetProtocol(3);
  onoff.SetAttribute("Remote", AddressValue(socket));
  apps = onoff.Install(nodes.Get(3));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink = PacketSinkHelper("ns3::PacketSocketFactory", socket);
  apps = sink.Install(nodes.Get(0));
  apps.Start(Seconds(0.0));
  apps.Stop(Seconds(20.0));

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                  MakeCallback(&SinkRx));

  NS_LOG_INFO("Configure Tracing.");

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("csma-packet-socket.tr"));

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  g_os.close();

  return 0;
}
