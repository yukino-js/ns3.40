

#include "ns3/abort.h"
#include "ns3/core-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PingEmulationExample");

static void PingRtt(std::string context, uint16_t seqNo, Time rtt) {
  NS_LOG_UNCOND("Received " << seqNo << " Response with RTT = " << rtt);
}

int main(int argc, char *argv[]) {
  NS_LOG_INFO("Ping Emulation Example");

  std::string deviceName("eth0");
  std::string remote("8.8.8.8");
  std::string localAddress("1.2.3.4");
  std::string localGateway("1.2.3.4");
#ifdef HAVE_PACKET_H
  std::string emuMode("raw");
#elif HAVE_NETMAP_USER_H
  std::string emuMode("netmap");
#else
  std::string emuMode("dpdk");
#endif

  CommandLine cmd(__FILE__);
  cmd.AddValue(
      "deviceName",
      "Device name (in raw, netmap mode) or Device address (in dpdk mode, eg: "
      "0000:00:1f.6). Use `lspci` to find device address.",
      deviceName);
  cmd.AddValue("remote", "Remote IP address (dotted decimal only please)",
               remote);
  cmd.AddValue("localIp", "Local IP address (dotted decimal only please)",
               localAddress);
  cmd.AddValue("gateway", "Gateway address (dotted decimal only please)",
               localGateway);
  cmd.AddValue("emuMode", "Emulation mode in {raw, netmap, dpdk}", emuMode);
  cmd.Parse(argc, argv);

  Ipv4Address remoteIp(remote.c_str());
  Ipv4Address localIp(localAddress.c_str());
  NS_ABORT_MSG_IF(
      localIp == "1.2.3.4",
      "You must change the local IP address before running this example");

  Ipv4Mask localMask("255.255.255.0");

  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  NS_LOG_INFO("Create Node");
  Ptr<Node> node = CreateObject<Node>();

  NS_LOG_INFO("Create Device");

  FdNetDeviceHelper *helper = nullptr;

#ifdef HAVE_PACKET_H
  if (emuMode == "raw") {
    auto raw = new EmuFdNetDeviceHelper;
    raw->SetDeviceName(deviceName);
    helper = raw;
  }
#endif
#ifdef HAVE_NETMAP_USER_H
  if (emuMode == "netmap") {
    NetmapNetDeviceHelper *netmap = new NetmapNetDeviceHelper;
    netmap->SetDeviceName(deviceName);
    helper = netmap;
  }
#endif
#ifdef HAVE_DPDK_USER_H
  if (emuMode == "dpdk") {
    DpdkNetDeviceHelper *dpdk = new DpdkNetDeviceHelper();
    dpdk->SetPmdLibrary("librte_pmd_e1000.so");
    dpdk->SetDpdkDriver("uio_pci_generic");
    dpdk->SetDeviceName(deviceName);
    helper = dpdk;
  }
#endif

  if (helper == nullptr) {
    NS_ABORT_MSG(emuMode << " not supported.");
  }

  NetDeviceContainer devices = helper->Install(node);
  Ptr<NetDevice> device = devices.Get(0);
  device->SetAttribute("Address", Mac48AddressValue(Mac48Address::Allocate()));

  NS_LOG_INFO("Add Internet Stack");
  InternetStackHelper internetStackHelper;
  internetStackHelper.Install(node);

  NS_LOG_INFO("Create IPv4 Interface");
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t interface = ipv4->AddInterface(device);
  Ipv4InterfaceAddress address = Ipv4InterfaceAddress(localIp, localMask);
  ipv4->AddAddress(interface, address);
  ipv4->SetMetric(interface, 1);
  ipv4->SetUp(interface);

  Ipv4Address gateway(localGateway.c_str());
  NS_ABORT_MSG_IF(
      gateway == "1.2.3.4",
      "You must change the gateway IP address before running this example");

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRouting =
      ipv4RoutingHelper.GetStaticRouting(ipv4);
  staticRouting->SetDefaultRoute(gateway, interface);

  NS_LOG_INFO("Create Ping Application");
  Ptr<Ping> app = CreateObject<Ping>();
  app->SetAttribute("Destination", AddressValue(remoteIp));
  app->SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));
  node->AddApplication(app);
  app->SetStartTime(Seconds(1.0));
  app->SetStopTime(Seconds(22.0));

  Names::Add("app", app);

  Config::Connect("/Names/app/Rtt", MakeCallback(&PingRtt));

  helper->EnablePcap(emuMode + "-emu-ping", device, true);

  NS_LOG_INFO("Run Emulation in " << emuMode << " mode.");
  Simulator::Stop(Seconds(23.0));
  Simulator::Run();
  Simulator::Destroy();
  delete helper;
  NS_LOG_INFO("Done.");

  return 0;
}
