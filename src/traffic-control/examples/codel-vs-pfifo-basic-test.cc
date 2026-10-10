

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/enum.h"
#include "ns3/error-model.h"
#include "ns3/event-id.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/tcp-header.h"
#include "ns3/traffic-control-module.h"
#include "ns3/udp-header.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CoDelPfifoFastBasicTest");

static void CwndTracer(Ptr<OutputStreamWrapper> stream, uint32_t oldval,
                       uint32_t newval) {
  *stream->GetStream() << oldval << " " << newval << std::endl;
}

static void TraceCwnd(std::string cwndTrFileName) {
  AsciiTraceHelper ascii;
  if (cwndTrFileName.empty()) {
    NS_LOG_DEBUG("No trace file for cwnd provided");
    return;
  } else {
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream(cwndTrFileName);
    Config::ConnectWithoutContext(
        "/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
        MakeBoundCallback(&CwndTracer, stream));
  }
}

int main(int argc, char *argv[]) {
  std::string bottleneckBandwidth = "5Mbps";
  std::string bottleneckDelay = "5ms";
  std::string accessBandwidth = "100Mbps";
  std::string accessDelay = "0.1ms";

  std::string queueDiscType = "PfifoFast";
  uint32_t queueDiscSize = 1000;
  uint32_t queueSize = 10;
  uint32_t pktSize = 1458;
  float startTime = 0.1F;
  float simDuration = 60;

  bool isPcapEnabled = true;
  std::string pcapFileName = "pcapFilePfifoFast.pcap";
  std::string cwndTrFileName = "cwndPfifoFast.tr";
  bool logging = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("bottleneckBandwidth", "Bottleneck bandwidth",
               bottleneckBandwidth);
  cmd.AddValue("bottleneckDelay", "Bottleneck delay", bottleneckDelay);
  cmd.AddValue("accessBandwidth", "Access link bandwidth", accessBandwidth);
  cmd.AddValue("accessDelay", "Access link delay", accessDelay);
  cmd.AddValue("queueDiscType", "Bottleneck queue disc type: PfifoFast, CoDel",
               queueDiscType);
  cmd.AddValue("queueDiscSize", "Bottleneck queue disc size in packets",
               queueDiscSize);
  cmd.AddValue("queueSize", "Devices queue size in packets", queueSize);
  cmd.AddValue("pktSize", "Packet size in bytes", pktSize);
  cmd.AddValue("startTime", "Simulation start time", startTime);
  cmd.AddValue("simDuration", "Simulation duration in seconds", simDuration);
  cmd.AddValue("isPcapEnabled", "Flag to enable/disable pcap", isPcapEnabled);
  cmd.AddValue("pcapFileName", "Name of pcap file", pcapFileName);
  cmd.AddValue("cwndTrFileName", "Name of cwnd trace file", cwndTrFileName);
  cmd.AddValue("logging", "Flag to enable/disable logging", logging);
  cmd.Parse(argc, argv);

  float stopTime = startTime + simDuration;

  if (logging) {
    LogComponentEnable("CoDelPfifoFastBasicTest", LOG_LEVEL_ALL);
    LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
    LogComponentEnable("PfifoFastQueueDisc", LOG_LEVEL_ALL);
    LogComponentEnable("CoDelQueueDisc", LOG_LEVEL_ALL);
  }

  if (isPcapEnabled) {
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  }

  Config::SetDefault(
      "ns3::DropTailQueue<Packet>::MaxSize",
      QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queueSize)));

  NodeContainer gateway;
  gateway.Create(1);
  NodeContainer source;
  source.Create(1);
  NodeContainer sink;
  sink.Create(1);

  PointToPointHelper accessLink;
  accessLink.SetDeviceAttribute("DataRate", StringValue(accessBandwidth));
  accessLink.SetChannelAttribute("Delay", StringValue(accessDelay));

  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute("DataRate",
                                    StringValue(bottleneckBandwidth));
  bottleneckLink.SetChannelAttribute("Delay", StringValue(bottleneckDelay));

  InternetStackHelper stack;
  stack.InstallAll();

  TrafficControlHelper tchPfifoFastAccess;
  tchPfifoFastAccess.SetRootQueueDisc("ns3::PfifoFastQueueDisc", "MaxSize",
                                      StringValue("1000p"));

  TrafficControlHelper tchPfifo;
  tchPfifo.SetRootQueueDisc("ns3::PfifoFastQueueDisc", "MaxSize",
                            StringValue(std::to_string(queueDiscSize) + "p"));

  TrafficControlHelper tchCoDel;
  tchCoDel.SetRootQueueDisc("ns3::CoDelQueueDisc");
  Config::SetDefault("ns3::CoDelQueueDisc::MaxSize",
                     StringValue(std::to_string(queueDiscSize) + "p"));

  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer sinkInterface;

  NetDeviceContainer devicesAccessLink;
  NetDeviceContainer devicesBottleneckLink;

  devicesAccessLink = accessLink.Install(source.Get(0), gateway.Get(0));
  tchPfifoFastAccess.Install(devicesAccessLink);
  address.NewNetwork();
  Ipv4InterfaceContainer interfaces = address.Assign(devicesAccessLink);

  devicesBottleneckLink = bottleneckLink.Install(gateway.Get(0), sink.Get(0));
  address.NewNetwork();

  if (queueDiscType == "PfifoFast") {
    tchPfifo.Install(devicesBottleneckLink);
  } else if (queueDiscType == "CoDel") {
    tchCoDel.Install(devicesBottleneckLink);
  } else {
    NS_ABORT_MSG("Invalid queue disc type: Use --queueDiscType=PfifoFast or "
                 "--queueDiscType=CoDel");
  }
  interfaces = address.Assign(devicesBottleneckLink);

  sinkInterface.Add(interfaces.Get(1));

  NS_LOG_INFO("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  uint16_t port = 50000;
  Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

  AddressValue remoteAddress(
      InetSocketAddress(sinkInterface.GetAddress(0, 0), port));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(pktSize));
  BulkSendHelper ftp("ns3::TcpSocketFactory", Address());
  ftp.SetAttribute("Remote", remoteAddress);
  ftp.SetAttribute("SendSize", UintegerValue(pktSize));
  ftp.SetAttribute("MaxBytes", UintegerValue(0));

  ApplicationContainer sourceApp = ftp.Install(source.Get(0));
  sourceApp.Start(Seconds(0));
  sourceApp.Stop(Seconds(stopTime - 3));

  sinkHelper.SetAttribute("Protocol",
                          TypeIdValue(TcpSocketFactory::GetTypeId()));
  ApplicationContainer sinkApp = sinkHelper.Install(sink);
  sinkApp.Start(Seconds(0));
  sinkApp.Stop(Seconds(stopTime));

  Simulator::Schedule(Seconds(0.00001), &TraceCwnd, cwndTrFileName);

  if (isPcapEnabled) {
    accessLink.EnablePcap(pcapFileName, source, true);
  }

  Simulator::Stop(Seconds(stopTime));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
