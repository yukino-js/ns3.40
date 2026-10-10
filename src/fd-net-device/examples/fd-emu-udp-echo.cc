

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EmulatedUdpEchoExample");

int main(int argc, char *argv[]) {
  std::string deviceName("eth1");
  std::string encapMode("Dix");
  bool clientMode = false;
  bool serverMode = false;
  double stopTime = 10;
  uint32_t nNodes = 2;

  CommandLine cmd(__FILE__);
  cmd.AddValue("client", "client mode", clientMode);
  cmd.AddValue("server", "server mode", serverMode);
  cmd.AddValue("deviceName", "device name", deviceName);
  cmd.AddValue("stopTime", "stop time (seconds)", stopTime);
  cmd.AddValue(
      "encapsulationMode",
      "encapsulation mode of emu device (\"Dix\" [default] or \"Llc\")",
      encapMode);
  cmd.AddValue("nNodes", "number of nodes to create (>= 2)", nNodes);

  cmd.Parse(argc, argv);

  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  if (clientMode && serverMode) {
    NS_FATAL_ERROR("Error, both client and server options cannot be enabled.");
  }
  nNodes = nNodes < 2 ? 2 : nNodes;

  NS_LOG_INFO("Create nodes.");
  NodeContainer n;
  n.Create(nNodes);

  InternetStackHelper internet;
  internet.Install(n);

  NS_LOG_INFO("Create channels.");
  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName(deviceName);
  emu.SetAttribute("EncapsulationMode", StringValue(encapMode));

  NetDeviceContainer d;
  Ipv4AddressHelper ipv4;
  Ipv4InterfaceContainer i;
  ApplicationContainer apps;

  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  if (clientMode) {
    d = emu.Install(n.Get(0));
    Ptr<FdNetDevice> dev = d.Get(0)->GetObject<FdNetDevice>();
    dev->SetAddress(Mac48Address("00:00:00:00:00:02"));
    NS_LOG_INFO("Assign IP Addresses.");
    ipv4.NewAddress();
    i = ipv4.Assign(d);
  } else if (serverMode) {
    d = emu.Install(n.Get(0));
    NS_LOG_INFO("Assign IP Addresses.");
    i = ipv4.Assign(d);
  } else {
    d = emu.Install(n);
    NS_LOG_INFO("Assign IP Addresses.");
    i = ipv4.Assign(d);
  }

  if (serverMode) {
    NS_LOG_INFO("Create Applications.");
    UdpEchoServerHelper server(9);
    apps = server.Install(n.Get(0));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(stopTime));
  } else if (clientMode) {
    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 20;
    Time interPacketInterval = Seconds(0.1);
    UdpEchoClientHelper client(Ipv4Address("10.1.1.1"), 9);
    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    apps = client.Install(n.Get(0));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(stopTime));
  } else {
    NS_LOG_INFO("Create Applications.");
    UdpEchoServerHelper server(9);
    apps = server.Install(n.Get(1));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(stopTime));

    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 20;
    Time interPacketInterval = Seconds(0.1);
    UdpEchoClientHelper client(i.GetAddress(1), 9);
    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    apps = client.Install(n.Get(0));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(stopTime));
  }

  emu.EnablePcapAll("fd-emu-udp-echo", true);
  emu.EnableAsciiAll("fd-emu-udp-echo.tr");

  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(stopTime + 2));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
