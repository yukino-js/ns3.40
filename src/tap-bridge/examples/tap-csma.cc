

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/wifi-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TapCsmaExample");

int main(int argc, char *argv[]) {
  std::string mode = "ConfigureLocal";
  std::string tapName = "thetap";

  CommandLine cmd(__FILE__);
  cmd.AddValue("mode", "Mode setting of TapBridge", mode);
  cmd.AddValue("tapName", "Name of the OS tap device", tapName);
  cmd.Parse(argc, argv);

  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  NodeContainer nodes;
  nodes.Create(4);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer devices = csma.Install(nodes);

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper addresses;
  addresses.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = addresses.Assign(devices);

  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute("Mode", StringValue(mode));
  tapBridge.SetAttribute("DeviceName", StringValue(tapName));
  tapBridge.Install(nodes.Get(0), devices.Get(0));

  csma.EnablePcapAll("tap-csma", false);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(60.));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
