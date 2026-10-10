

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EmuFdNetDeviceSaturationExample");

int main(int argc, char *argv[]) {
  uint16_t sinkPort = 8000;
  uint32_t packetSize = 1400;
  std::string dataRate("1000Mb/s");
  bool serverMode = false;

  std::string deviceName("eth0");
  std::string client("10.1.1.1");
  std::string server("10.1.1.2");
  std::string netmask("255.255.255.0");
  std::string macClient("00:00:00:00:00:01");
  std::string macServer("00:00:00:00:00:02");
  std::string transportProt = "Tcp";
  std::string socketType;
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
  cmd.AddValue("client", "Local IP address (dotted decimal only please)",
               client);
  cmd.AddValue("server", "Remote IP address (dotted decimal only please)",
               server);
  cmd.AddValue("localmask", "Local mask address (dotted decimal only please)",
               netmask);
  cmd.AddValue("serverMode", "1:true, 0:false, default client", serverMode);
  cmd.AddValue("mac-client",
               "Mac Address for Server Client : 00:00:00:00:00:01", macClient);
  cmd.AddValue("mac-server",
               "Mac Address for Server Default : 00:00:00:00:00:02", macServer);
  cmd.AddValue("data-rate", "Data rate defaults to 1000Mb/s", dataRate);
  cmd.AddValue("transportProt", "Transport protocol to use: Tcp, Udp",
               transportProt);
  cmd.AddValue("emuMode", "Emulation mode in {raw, netmap}", emuMode);
  cmd.Parse(argc, argv);

  if (transportProt == "Tcp") {
    socketType = "ns3::TcpSocketFactory";
  } else {
    socketType = "ns3::UdpSocketFactory";
  }

  Ipv4Address remoteIp;
  Ipv4Address localIp;
  Mac48AddressValue localMac;

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(packetSize));

  if (serverMode) {
    remoteIp = Ipv4Address(client.c_str());
    localIp = Ipv4Address(server.c_str());
    localMac = Mac48AddressValue(macServer.c_str());
  } else {
    remoteIp = Ipv4Address(server.c_str());
    localIp = Ipv4Address(client.c_str());
    localMac = Mac48AddressValue(macClient.c_str());
  }

  Ipv4Mask localMask(netmask.c_str());

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
  device->SetAttribute("Address", localMac);

  NS_LOG_INFO("Add Internet Stack");
  InternetStackHelper internetStackHelper;
  internetStackHelper.SetIpv4StackInstall(true);
  internetStackHelper.Install(node);

  NS_LOG_INFO("Create IPv4 Interface");
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t interface = ipv4->AddInterface(device);
  Ipv4InterfaceAddress address = Ipv4InterfaceAddress(localIp, localMask);
  ipv4->AddAddress(interface, address);
  ipv4->SetMetric(interface, 1);
  ipv4->SetUp(interface);

  if (serverMode) {
    Address sinkLocalAddress(InetSocketAddress(localIp, sinkPort));
    PacketSinkHelper sinkHelper(socketType, sinkLocalAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(node);
    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(60.0));

    helper->EnablePcap("fd-server", device);
  } else {
    AddressValue remoteAddress(InetSocketAddress(remoteIp, sinkPort));
    OnOffHelper onoff(socketType, Address());
    onoff.SetAttribute("Remote", remoteAddress);
    onoff.SetAttribute("OnTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime",
                       StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", DataRateValue(dataRate));
    onoff.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer clientApps = onoff.Install(node);
    clientApps.Start(Seconds(4.0));
    clientApps.Stop(Seconds(58.0));

    helper->EnablePcap("fd-client", device);
  }

  Simulator::Stop(Seconds(61.0));
  Simulator::Run();
  Simulator::Destroy();
  delete helper;

  return 0;
}
