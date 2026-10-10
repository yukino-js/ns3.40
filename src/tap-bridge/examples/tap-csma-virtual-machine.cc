

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/tap-bridge-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TapCsmaVirtualMachineExample");

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  NodeContainer nodes;
  nodes.Create(2);

  CsmaHelper csma;
  NetDeviceContainer devices = csma.Install(nodes);

  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute("Mode", StringValue("UseBridge"));
  tapBridge.SetAttribute("DeviceName", StringValue("tap-left"));
  tapBridge.Install(nodes.Get(0), devices.Get(0));

  tapBridge.SetAttribute("DeviceName", StringValue("tap-right"));
  tapBridge.Install(nodes.Get(1), devices.Get(1));

  Simulator::Stop(Seconds(600.));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
