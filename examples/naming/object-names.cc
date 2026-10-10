

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ObjectNamesExample");

uint32_t bytesReceived = 0;

void RxEvent(std::string context, Ptr<const Packet> packet) {
  std::cout << Simulator::Now().GetSeconds() << "s " << context
            << " packet size " << packet->GetSize() << std::endl;
  bytesReceived += packet->GetSize();
}

int main(int argc, char *argv[]) {
  bool outputValidated = true;

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NodeContainer n;
  n.Create(4);

  Names::Add("clientZero", n.Get(0));
  Names::Add("/Names/server", n.Get(1));

  Names::Rename("clientZero", "client");

  InternetStackHelper internet;
  internet.Install(n);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", UintegerValue(1400));
  NetDeviceContainer d = csma.Install(n);

  Names::Add("/Names/client/eth0", d.Get(0));
  Names::Add("server/eth0", d.Get(1));

  Ptr<CsmaNetDevice> csmaNetDevice = d.Get(0)->GetObject<CsmaNetDevice>();
  UintegerValue val;
  csmaNetDevice->GetAttribute("Mtu", val);
  std::cout << "MTU on device 0 before configuration is " << val.Get()
            << std::endl;

  Config::Set("/Names/client/eth0/Mtu", UintegerValue(1234));

  csmaNetDevice->GetAttribute("Mtu", val);
  std::cout << "MTU on device 0 after configuration is " << val.Get()
            << std::endl;

  if (val.Get() != 1234) {
    outputValidated = false;
  }

  Config::Set("/NodeList/1/eth0/Mtu", UintegerValue(1234));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(d);

  uint16_t port = 9;
  UdpEchoServerHelper server(port);
  ApplicationContainer apps = server.Install("/Names/server");
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  Time interPacketInterval = Seconds(1.);
  UdpEchoClientHelper client(i.GetAddress(1), port);
  client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
  client.SetAttribute("Interval", TimeValue(interPacketInterval));
  client.SetAttribute("PacketSize", UintegerValue(packetSize));
  apps = client.Install("/Names/client");
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(10.0));

  Config::Connect("/Names/client/eth0/MacRx", MakeCallback(&RxEvent));

  csma.EnablePcapAll("object-names");

  csma.EnablePcap("client-device.pcap", d.Get(0), false, true);

  std::cout << "Running simulation..." << std::endl;
  Simulator::Run();
  Simulator::Destroy();

  if (bytesReceived != (64 + 64 + 1070)) {
    outputValidated = false;
  }

  if (!outputValidated) {
    std::cerr << "Program internal checking failed; returning with error"
              << std::endl;
    return (1);
  }

  return 0;
}
