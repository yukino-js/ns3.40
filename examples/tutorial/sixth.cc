

#include "tutorial-app.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SixthScriptExample");

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
  CommandLine cmd(__FILE__);
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

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  uint16_t sinkPort = 8080;
  Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));
  PacketSinkHelper packetSinkHelper(
      "ns3::TcpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
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
      asciiTraceHelper.CreateFileStream("sixth.cwnd");
  ns3TcpSocket->TraceConnectWithoutContext(
      "CongestionWindow", MakeBoundCallback(&CwndChange, stream));

  PcapHelper pcapHelper;
  Ptr<PcapFileWrapper> file =
      pcapHelper.CreateFile("sixth.pcap", std::ios::out, PcapHelper::DLT_PPP);
  devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop",
                                             MakeBoundCallback(&RxDrop, file));

  Simulator::Stop(Seconds(20));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
