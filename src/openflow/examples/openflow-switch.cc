

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/openflow-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OpenFlowCsmaSwitchExample");

bool verbose = false;
bool use_drop = false;
ns3::Time timeout = ns3::Seconds(0);

bool SetVerbose(const std::string &value) {
  verbose = true;
  return true;
}

bool SetDrop(const std::string &value) {
  use_drop = true;
  return true;
}

bool SetTimeout(const std::string &value) {
  try {
    timeout = ns3::Seconds(std::stof(value));
    return true;
  } catch (...) {
    return false;
  }
  return false;
}

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.AddValue("v", "Verbose (turns on logging).", MakeCallback(&SetVerbose));
  cmd.AddValue("verbose", "Verbose (turns on logging).",
               MakeCallback(&SetVerbose));
  cmd.AddValue("d", "Use Drop Controller (Learning if not specified).",
               MakeCallback(&SetDrop));
  cmd.AddValue("drop", "Use Drop Controller (Learning if not specified).",
               MakeCallback(&SetDrop));
  cmd.AddValue("t",
               "Learning Controller Timeout (has no effect if drop controller "
               "is specified).",
               MakeCallback(&SetTimeout));
  cmd.AddValue("timeout",
               "Learning Controller Timeout (has no effect if drop controller "
               "is specified).",
               MakeCallback(&SetTimeout));

  cmd.Parse(argc, argv);

  if (verbose) {
    LogComponentEnable("OpenFlowCsmaSwitchExample", LOG_LEVEL_INFO);
    LogComponentEnable("OpenFlowInterface", LOG_LEVEL_INFO);
    LogComponentEnable("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
  }

  NS_LOG_INFO("Create nodes.");
  NodeContainer terminals;
  terminals.Create(4);

  NodeContainer csmaSwitch;
  csmaSwitch.Create(1);

  NS_LOG_INFO("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  NetDeviceContainer terminalDevices;
  NetDeviceContainer switchDevices;
  for (int i = 0; i < 4; i++) {
    NetDeviceContainer link =
        csma.Install(NodeContainer(terminals.Get(i), csmaSwitch));
    terminalDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1));
  }

  Ptr<Node> switchNode = csmaSwitch.Get(0);
  OpenFlowSwitchHelper swtch;

  if (use_drop) {
    Ptr<ns3::ofi::DropController> controller =
        CreateObject<ns3::ofi::DropController>();
    swtch.Install(switchNode, switchDevices, controller);
  } else {
    Ptr<ns3::ofi::LearningController> controller =
        CreateObject<ns3::ofi::LearningController>();
    if (!timeout.IsZero()) {
      controller->SetAttribute("ExpirationTime", TimeValue(timeout));
    }
    swtch.Install(switchNode, switchDevices, controller);
  }

  InternetStackHelper internet;
  internet.Install(terminals);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(terminalDevices);

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
  onoff.SetConstantRate(DataRate("500kb/s"));

  ApplicationContainer app = onoff.Install(terminals.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  app = sink.Install(terminals.Get(1));
  app.Start(Seconds(0.0));

  onoff.SetAttribute(
      "Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.1.1"), port)));
  app = onoff.Install(terminals.Get(3));
  app.Start(Seconds(1.1));
  app.Stop(Seconds(10.0));

  app = sink.Install(terminals.Get(0));
  app.Start(Seconds(0.0));

  NS_LOG_INFO("Configure Tracing.");

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("openflow-switch.tr"));

  csma.EnablePcapAll("openflow-switch", false);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
