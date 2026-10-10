

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <errno.h>
#include <sys/socket.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FdNetDeviceSaturationExample");

int main(int argc, char *argv[]) {
  bool tcpMode = false;
  CommandLine cmd(__FILE__);
  cmd.AddValue("tcpMode", "1:true, 0:false, default mode UDP", tcpMode);
  cmd.Parse(argc, argv);

  std::string factory;
  if (tcpMode) {
    factory = "ns3::TcpSocketFactory";
  } else {
    factory = "ns3::UdpSocketFactory";
  }

  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  uint16_t sinkPort = 8000;
  uint32_t packetSize = 10000;
  std::string dataRate("10Mb/s");

  NS_LOG_INFO("Create Node");
  NodeContainer nodes;
  nodes.Create(2);

  NS_LOG_INFO("Create Device");
  FdNetDeviceHelper fd;
  NetDeviceContainer devices = fd.Install(nodes);

  int sv[2];
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) < 0) {
    NS_FATAL_ERROR("Error creating pipe=" << strerror(errno));
  }

  Ptr<NetDevice> d1 = devices.Get(0);
  Ptr<FdNetDevice> clientDevice = d1->GetObject<FdNetDevice>();
  clientDevice->SetFileDescriptor(sv[0]);

  Ptr<NetDevice> d2 = devices.Get(1);
  Ptr<FdNetDevice> serverDevice = d2->GetObject<FdNetDevice>();
  serverDevice->SetFileDescriptor(sv[1]);

  NS_LOG_INFO("Add Internet Stack");
  InternetStackHelper internetStackHelper;
  internetStackHelper.SetIpv4StackInstall(true);
  internetStackHelper.Install(nodes);

  NS_LOG_INFO("Create IPv4 Interface");
  Ipv4AddressHelper addresses;
  addresses.SetBase("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = addresses.Assign(devices);

  Ptr<Node> clientNode = nodes.Get(0);
  Ipv4Address serverIp = interfaces.GetAddress(1);
  Ptr<Node> serverNode = nodes.Get(1);

  Address sinkLocalAddress(InetSocketAddress(serverIp, sinkPort));

  PacketSinkHelper sinkHelper(factory, sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install(serverNode);
  sinkApp.Start(Seconds(0.0));
  sinkApp.Stop(Seconds(30.0));
  fd.EnablePcap("fd2fd-onoff-server", serverDevice);

  AddressValue serverAddress(InetSocketAddress(serverIp, sinkPort));
  OnOffHelper onoff(factory, Address());
  onoff.SetAttribute("Remote", serverAddress);
  onoff.SetAttribute("OnTime",
                     StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime",
                     StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute("DataRate", DataRateValue(dataRate));
  onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
  ApplicationContainer clientApps = onoff.Install(clientNode);
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(29.0));
  fd.EnablePcap("fd2fd-onoff-client", clientDevice);

  Simulator::Stop(Seconds(30.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
