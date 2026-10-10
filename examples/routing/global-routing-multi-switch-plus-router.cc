

#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GlobalRoutingMultiSwitchPlusRouter");

#define vssearch(loc, vec)                                                     \
  std::find((vec).begin(), (vec).end(), (loc)) != (vec).end()

int main(int argc, char *argv[]) {
  bool verbose = true;

  int simDurationSeconds = 60;

  bool enableUdpMultiSW = true;
  bool enableUdpSingleSW = true;

  std::string pcapLocations = "";
  uint32_t snapLen = PcapFile::SNAPLEN_DEFAULT;

  std::string csmaXLinkDataRate = "100Mbps";
  std::string csmaXLinkDelay = "500ns";

  std::string csmaYLinkDataRate = "10Mbps";
  std::string csmaYLinkDelay = "500ns";

  std::string p2pLinkDataRate = "5Mbps";
  std::string p2pLinkDelay = "50ms";

  uint16_t udpEchoPort = 9;

  CommandLine cmd(__FILE__);

  cmd.Usage("NOTE: valid --pcap arguments are: "
            "'t2,t3,b2,b3,trlan,trwan,brlan,brwan'");

  cmd.AddValue("verbose", "Enable printing informational messages", verbose);

  cmd.AddValue("duration", "Duration of simulation.", simDurationSeconds);

  cmd.AddValue("udpMultiSW", "Enable udp over multi-switch links",
               enableUdpMultiSW);
  cmd.AddValue("udpSingleSW", "Enable udp over single-switch links",
               enableUdpSingleSW);

  cmd.AddValue("pcap", "Comma separated list of PCAP Locations to tap",
               pcapLocations);
  cmd.AddValue("snapLen", "PCAP packet capture length", snapLen);

  cmd.AddValue("csmaXRate", "CSMA X Link data rate", csmaXLinkDataRate);
  cmd.AddValue("csmaXDelay", "CSMA X Link delay", csmaXLinkDelay);

  cmd.AddValue("csmaYRate", "CSMA Y Link data rate", csmaYLinkDataRate);
  cmd.AddValue("csmaYDelay", "CSMA Y Link delay", csmaYLinkDelay);

  cmd.AddValue("p2pRate", "P2P Link data rate", p2pLinkDataRate);
  cmd.AddValue("p2pDelay", "P2P Link delay", p2pLinkDelay);

  cmd.Parse(argc, argv);

  if (verbose) {
    LogComponentEnable("GlobalRoutingMultiSwitchPlusRouter", LOG_LEVEL_INFO);
  }

  const std::vector<std::string> pcapTaps{
      "t2", "t3", "b2", "b3", "trlan", "trwan", "brlan", "brwan",
  };

  std::vector<std::string> pcapLocationVec;
  if (!pcapLocations.empty()) {
    std::stringstream sStream(pcapLocations);

    while (sStream.good()) {
      std::string substr;
      getline(sStream, substr, ',');
      if (vssearch(substr, pcapTaps)) {
        pcapLocationVec.push_back(substr);
      } else {
        NS_LOG_ERROR("WARNING: Unrecognized PCAP location: <" + substr + ">");
      }
    }

    for (auto ploc = pcapLocationVec.begin(); ploc != pcapLocationVec.end();
         ++ploc) {
      NS_LOG_INFO("PCAP capture at: <" + *ploc + ">");
    }
  }

  if (snapLen != PcapFile::SNAPLEN_DEFAULT) {
    Config::SetDefault("ns3::PcapFileWrapper::CaptureSize",
                       UintegerValue(snapLen));
  }

  NS_LOG_INFO("INFO: Create nodes.");
  Ptr<Node> t2 = CreateObject<Node>();
  Ptr<Node> t3 = CreateObject<Node>();
  Ptr<Node> ts1 = CreateObject<Node>();
  Ptr<Node> ts2 = CreateObject<Node>();
  Ptr<Node> ts3 = CreateObject<Node>();
  Ptr<Node> ts4 = CreateObject<Node>();
  Ptr<Node> tr = CreateObject<Node>();
  Ptr<Node> br = CreateObject<Node>();
  Ptr<Node> bs1 = CreateObject<Node>();
  Ptr<Node> bs2 = CreateObject<Node>();
  Ptr<Node> bs3 = CreateObject<Node>();
  Ptr<Node> bs4 = CreateObject<Node>();
  Ptr<Node> bs5 = CreateObject<Node>();
  Ptr<Node> b2 = CreateObject<Node>();

  Ptr<Node> b3 = CreateObject<Node>();

  Names::Add("t2", t2);
  Names::Add("t3", t3);
  Names::Add("ts1", ts1);
  Names::Add("ts2", ts2);
  Names::Add("ts3", ts3);
  Names::Add("ts4", ts4);
  Names::Add("tr", tr);
  Names::Add("br", br);
  Names::Add("bs1", bs1);
  Names::Add("bs2", bs2);
  Names::Add("bs3", bs3);
  Names::Add("bs4", bs4);
  Names::Add("bs5", bs5);
  Names::Add("b2", b2);
  Names::Add("b3", b3);

  NS_LOG_INFO("L2: Create a " << csmaXLinkDataRate << " " << csmaXLinkDelay
                              << " CSMA link for csmaX for LANs.");
  CsmaHelper csmaX;
  csmaX.SetChannelAttribute("DataRate", StringValue(csmaXLinkDataRate));
  csmaX.SetChannelAttribute("Delay", StringValue(csmaXLinkDelay));

  NS_LOG_INFO("L2: Create a " << csmaYLinkDataRate << " " << csmaYLinkDelay
                              << " CSMA link for csmaY for LANs.");
  CsmaHelper csmaY;
  csmaY.SetChannelAttribute("DataRate", StringValue(csmaYLinkDataRate));
  csmaY.SetChannelAttribute("Delay", StringValue(csmaYLinkDelay));

  NS_LOG_INFO(
      "L2: Connect nodes on top LAN together with half-duplex CSMA links.");

  NetDeviceContainer link_t2_ts4 = csmaX.Install(NodeContainer(t2, ts4));
  NetDeviceContainer link_ts4_ts3 = csmaY.Install(NodeContainer(ts4, ts3));
  NetDeviceContainer link_ts3_ts2 = csmaX.Install(NodeContainer(ts3, ts2));
  NetDeviceContainer link_ts2_ts1 = csmaY.Install(NodeContainer(ts2, ts1));

  NetDeviceContainer link_t3_ts1 = csmaX.Install(NodeContainer(t3, ts1));

  NetDeviceContainer link_tr_ts1 = csmaY.Install(NodeContainer(tr, ts1));

  NS_LOG_INFO(
      "L2: Connect nodes on bottom LAN together with half-duplex CSMA links.");

  NetDeviceContainer link_b2_bs5 = csmaY.Install(NodeContainer(b2, bs5));
  NetDeviceContainer link_bs5_bs4 = csmaX.Install(NodeContainer(bs5, bs4));
  NetDeviceContainer link_bs4_bs3 = csmaY.Install(NodeContainer(bs4, bs3));
  NetDeviceContainer link_bs3_bs2 = csmaX.Install(NodeContainer(bs3, bs2));
  NetDeviceContainer link_bs2_bs1 = csmaY.Install(NodeContainer(bs2, bs1));

  NetDeviceContainer link_b3_bs1 = csmaY.Install(NodeContainer(b3, bs1));

  NetDeviceContainer link_br_bs1 = csmaX.Install(NodeContainer(br, bs1));

  NS_LOG_INFO("L2: Create a " << p2pLinkDataRate << " " << p2pLinkDelay
                              << " Point-to-Point link for the WAN.");

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(p2pLinkDataRate));
  p2p.SetChannelAttribute("Delay", StringValue(p2pLinkDelay));

  NS_LOG_INFO(
      "L2: Connect the routers together with the Point-to-Point WAN link.");

  NetDeviceContainer link_tr_br;
  link_tr_br = p2p.Install(NodeContainer(tr, br));

  NetDeviceContainer ts4nd;
  ts4nd.Add(link_t2_ts4.Get(1));
  ts4nd.Add(link_ts4_ts3.Get(0));

  NetDeviceContainer ts3nd;
  ts3nd.Add(link_ts4_ts3.Get(1));
  ts3nd.Add(link_ts3_ts2.Get(0));

  NetDeviceContainer ts2nd;
  ts2nd.Add(link_ts3_ts2.Get(1));
  ts2nd.Add(link_ts2_ts1.Get(0));

  NetDeviceContainer ts1nd;
  ts1nd.Add(link_ts2_ts1.Get(1));
  ts1nd.Add(link_t3_ts1.Get(1));
  ts1nd.Add(link_tr_ts1.Get(1));

  NetDeviceContainer bs1nd;
  bs1nd.Add(link_br_bs1.Get(1));
  bs1nd.Add(link_bs2_bs1.Get(1));
  bs1nd.Add(link_b3_bs1.Get(1));

  NetDeviceContainer bs2nd;
  bs2nd.Add(link_bs2_bs1.Get(0));
  bs2nd.Add(link_bs3_bs2.Get(1));

  NetDeviceContainer bs3nd;
  bs3nd.Add(link_bs3_bs2.Get(0));
  bs3nd.Add(link_bs4_bs3.Get(1));

  NetDeviceContainer bs4nd;
  bs4nd.Add(link_bs4_bs3.Get(0));
  bs4nd.Add(link_bs5_bs4.Get(1));

  NetDeviceContainer bs5nd;
  bs5nd.Add(link_bs5_bs4.Get(0));
  bs5nd.Add(link_b2_bs5.Get(1));

  BridgeHelper bridge;

  bridge.Install(ts1, ts1nd);
  bridge.Install(ts2, ts2nd);
  bridge.Install(ts3, ts3nd);
  bridge.Install(ts4, ts4nd);

  bridge.Install(bs1, bs1nd);
  bridge.Install(bs2, bs2nd);
  bridge.Install(bs3, bs3nd);
  bridge.Install(bs4, bs4nd);
  bridge.Install(bs5, bs5nd);

  InternetStackHelper ns3IpStack;

  NS_LOG_INFO("L3: Install the ns3 IP stack on udp client and server nodes.");
  NodeContainer endpointNodes(t2, t3, b2, b3);
  ns3IpStack.Install(endpointNodes);

  NS_LOG_INFO("L3: Install the ns3 IP stack on routers.");
  NodeContainer routerNodes(tr, br);
  ns3IpStack.Install(routerNodes);

  NS_LOG_INFO("L3: Assign top LAN IP Addresses.");

  NetDeviceContainer topLanIpDevices;
  topLanIpDevices.Add(link_tr_ts1.Get(0));
  topLanIpDevices.Add(link_t2_ts4.Get(0));
  topLanIpDevices.Add(link_t3_ts1.Get(0));
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("192.168.1.0", "255.255.255.0");
  ipv4.Assign(topLanIpDevices);

  NS_LOG_INFO("L3: Assign bottom LAN IP Addresses.");

  NetDeviceContainer botLanIpDevices;
  botLanIpDevices.Add(link_br_bs1.Get(0));
  botLanIpDevices.Add(link_b2_bs5.Get(0));
  botLanIpDevices.Add(link_b3_bs1.Get(0));

  ipv4.SetBase("192.168.2.0", "255.255.255.0");
  ipv4.Assign(botLanIpDevices);

  NS_LOG_INFO("L3: Assign WAN IP Addresses.");

  ipv4.SetBase("76.1.1.0", "255.255.255.0");
  ipv4.Assign(link_tr_br);

  NS_LOG_INFO("L3: Populate routing tables.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  ApplicationContainer apps;

  if (enableUdpMultiSW) {
    NS_LOG_INFO("APP: Multi-Switch UDP server (on node b2 of bottom LAN)");

    UdpEchoServerHelper server(udpEchoPort);

    ApplicationContainer serverApp = server.Install(b2);
    serverApp.Start(Seconds(0.5));
    serverApp.Stop(Seconds(simDurationSeconds));

    NS_LOG_INFO("APP: Multi-Switch UDP client (on node t2 of top LAN)");

    Time interPacketInterval = Seconds(0.005);
    uint32_t packetSize = 1000;
    uint32_t maxPacketCount = (simDurationSeconds - 2.0) / 0.005;

    UdpEchoClientHelper client(Ipv4Address("192.168.2.2"), udpEchoPort);

    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer clientApp = client.Install(t2);
    clientApp.Start(Seconds(0.5));
    clientApp.Stop(Seconds(simDurationSeconds));
  }

  if (enableUdpSingleSW) {
    NS_LOG_INFO("APP: Single-Switch UDP server (on node t3 of top LAN)");

    UdpEchoServerHelper server(udpEchoPort);

    ApplicationContainer serverApp = server.Install(t3);
    serverApp.Start(Seconds(0.5));
    serverApp.Stop(Seconds(simDurationSeconds));

    NS_LOG_INFO("APP: Single-Switch UDP client (on node b3 bottom LAN)");

    Time interPacketInterval = Seconds(0.005);
    uint32_t packetSize = 1000;
    uint32_t maxPacketCount = (simDurationSeconds - 2.0) / 0.005;

    UdpEchoClientHelper client(Ipv4Address("192.168.1.3"), udpEchoPort);

    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer clientApp = client.Install(b3);
    clientApp.Start(Seconds(0.5));
    clientApp.Stop(Seconds(simDurationSeconds));
  }

  NS_LOG_INFO("Set up to print routing tables at T=0.1s");

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(
      "global-routing-multi-switch-plus-router.routes", std::ios::out);

  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(0.1), routingStream);

  NS_LOG_INFO("Configure PCAP Tracing (if any configured).");

  if (vssearch("t2", pcapLocationVec)) {
    csmaX.EnablePcap("t2.pcap", topLanIpDevices.Get(1), true, true);
  }

  if (vssearch("b2", pcapLocationVec)) {
    csmaY.EnablePcap("b2.pcap", botLanIpDevices.Get(1), true, true);
  }

  if (vssearch("b3", pcapLocationVec)) {
    csmaY.EnablePcap("b3.pcap", botLanIpDevices.Get(2), true, true);
  }

  if (vssearch("t3", pcapLocationVec)) {
    csmaX.EnablePcap("t3.pcap", topLanIpDevices.Get(2), true, true);
  }

  if (vssearch("trlan", pcapLocationVec)) {
    csmaY.EnablePcap("trlan.pcap", topLanIpDevices.Get(0), true, true);
  }

  if (vssearch("brlan", pcapLocationVec)) {
    csmaX.EnablePcap("brlan.pcap", botLanIpDevices.Get(0), true, true);
  }

  if (vssearch("trwan", pcapLocationVec)) {
    p2p.EnablePcap("trwan.pcap", link_tr_br.Get(0), true, true);
  }

  if (vssearch("brwan", pcapLocationVec)) {
    p2p.EnablePcap("brwan.pcap", link_tr_br.Get(1), true, true);
  }

  NS_LOG_INFO("Run Simulation for " << simDurationSeconds << " seconds.");

  Simulator::Stop(Seconds(simDurationSeconds));
  Simulator::Run();

  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
