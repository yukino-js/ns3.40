

#include "tutorial-app.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/stats-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SeventhScriptExample");

static void CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd,
                       uint32_t newCwnd) {
  NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
  *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd
                       << "\t" << newCwnd << std::endl;
}

static void RxDrop(Ptr<PcapFileWrapper> file, Ptr<const Packet> p) {
  NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
  file->Write(Simulator::Now(), p);
}

int main(int argc, char *argv[]) {
  bool useV6 = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("useIpv6", "Use Ipv6", useV6);
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install(nodes);

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
  em->SetAttribute("ErrorRate", DoubleValue(0.00001));
  devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  InternetStackHelper stack;
  stack.Install(nodes);

  uint16_t sinkPort = 8080;
  Address sinkAddress;
  Address anyAddress;
  std::string probeType;
  std::string tracePath;
  if (!useV6) {
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    sinkAddress = InetSocketAddress(interfaces.GetAddress(1), sinkPort);
    anyAddress = InetSocketAddress(Ipv4Address::GetAny(), sinkPort);
    probeType = "ns3::Ipv4PacketProbe";
    tracePath = "/NodeList/*/$ns3::Ipv4L3Protocol/Tx";
  } else {
    Ipv6AddressHelper address;
    address.SetBase("2001:0000:f00d:cafe::", Ipv6Prefix(64));
    Ipv6InterfaceContainer interfaces = address.Assign(devices);
    sinkAddress = Inet6SocketAddress(interfaces.GetAddress(1, 1), sinkPort);
    anyAddress = Inet6SocketAddress(Ipv6Address::GetAny(), sinkPort);
    probeType = "ns3::Ipv6PacketProbe";
    tracePath = "/NodeList/*/$ns3::Ipv6L3Protocol/Tx";
  }

  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", anyAddress);
  ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
  sinkApps.Start(Seconds(0.));
  sinkApps.Stop(Seconds(20.));

  Ptr<Socket> ns3TcpSocket =
      Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());

  Ptr<TutorialApp> app = CreateObject<TutorialApp>();
  app->Setup(ns3TcpSocket, sinkAddress, 1040, 1000, DataRate("1Mbps"));
  nodes.Get(0)->AddApplication(app);
  app->SetStartTime(Seconds(1.));
  app->SetStopTime(Seconds(20.));

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream =
      asciiTraceHelper.CreateFileStream("seventh.cwnd");
  ns3TcpSocket->TraceConnectWithoutContext(
      "CongestionWindow", MakeBoundCallback(&CwndChange, stream));

  PcapHelper pcapHelper;
  Ptr<PcapFileWrapper> file =
      pcapHelper.CreateFile("seventh.pcap", std::ios::out, PcapHelper::DLT_PPP);
  devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop",
                                             MakeBoundCallback(&RxDrop, file));

  GnuplotHelper plotHelper;

  plotHelper.ConfigurePlot("seventh-packet-byte-count",
                           "Packet Byte Count vs. Time", "Time (Seconds)",
                           "Packet Byte Count");

  plotHelper.PlotProbe(probeType, tracePath, "OutputBytes", "Packet Byte Count",
                       GnuplotAggregator::KEY_BELOW);

  FileHelper fileHelper;

  fileHelper.ConfigureFile("seventh-packet-byte-count",
                           FileAggregator::FORMATTED);

  fileHelper.Set2dFormat("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");

  fileHelper.WriteProbe(probeType, tracePath, "OutputBytes");

  Simulator::Stop(Seconds(20));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
