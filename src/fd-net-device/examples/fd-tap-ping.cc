

#include "ns3/abort.h"
#include "ns3/core-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TAPPingExample");

static void PingRtt(std::string context, uint16_t seqNo, Time rtt) {
  NS_LOG_UNCOND("Received " << seqNo << " Response with RTT = " << rtt);
}

int main(int argc, char *argv[]) {
  NS_LOG_INFO("Ping Emulation Example with TAP");

  std::string deviceName("tap0");
  std::string remote("192.0.43.10");
  std::string network("1.2.3.4");
  std::string mask("255.255.255.0");
  std::string pi("no");

  CommandLine cmd(__FILE__);
  cmd.AddValue("deviceName", "Device name", deviceName);
  cmd.AddValue("remote", "Remote IP address (dotted decimal only please)",
               remote);
  cmd.AddValue("tapNetwork",
               "Network address to assign the TAP device IP address (dotted "
               "decimal only please)",
               network);
  cmd.AddValue(
      "tapMask",
      "Network mask for configure the TAP device (dotted decimal only please)",
      mask);
  cmd.AddValue("modePi",
               "If 'yes' a PI header will be added to the traffic traversing "
               "the device(flag "
               "IFF_NOPI will be unset).",
               pi);
  cmd.Parse(argc, argv);

  NS_ABORT_MSG_IF(
      network == "1.2.3.4",
      "You must change the local IP address before running this example");

  Ipv4Address remoteIp(remote.c_str());
  Ipv4Address tapNetwork(network.c_str());
  Ipv4Mask tapMask(mask.c_str());

  bool modePi = (pi == "yes");

  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  NS_LOG_INFO("Create Node");
  Ptr<Node> node = CreateObject<Node>();

  Ipv4AddressHelper addresses;
  addresses.SetBase(tapNetwork, tapMask);
  Ipv4Address tapIp = addresses.NewAddress();

  NS_LOG_INFO("Create Device");
  TapFdNetDeviceHelper helper;
  helper.SetDeviceName(deviceName);
  helper.SetModePi(modePi);
  helper.SetTapIpv4Address(tapIp);
  helper.SetTapIpv4Mask(tapMask);

  NetDeviceContainer devices = helper.Install(node);
  Ptr<NetDevice> device = devices.Get(0);

  NS_LOG_INFO("Add Internet Stack");
  InternetStackHelper internetStackHelper;
  internetStackHelper.Install(node);

  NS_LOG_INFO("Create IPv4 Interface");
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t interface = ipv4->AddInterface(device);
  Ipv4Address devIp = addresses.NewAddress();
  Ipv4InterfaceAddress address = Ipv4InterfaceAddress(devIp, tapMask);
  ipv4->AddAddress(interface, address);
  ipv4->SetMetric(interface, 1);
  ipv4->SetUp(interface);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRouting =
      ipv4RoutingHelper.GetStaticRouting(ipv4);
  staticRouting->SetDefaultRoute(tapIp, interface);

  NS_LOG_INFO("Create Ping Application");
  Ptr<Ping> app = CreateObject<Ping>();
  app->SetAttribute("Destination", AddressValue(remoteIp));
  app->SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));
  node->AddApplication(app);
  app->SetStartTime(Seconds(1.0));
  app->SetStopTime(Seconds(21.0));

  Names::Add("app", app);

  Config::Connect("/Names/app/Rtt", MakeCallback(&PingRtt));

  helper.EnablePcap("fd-tap-ping", device, true);

  NS_LOG_INFO("Run Emulation.");
  Simulator::Stop(Seconds(25.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
