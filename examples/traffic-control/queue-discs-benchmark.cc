

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BenchmarkQueueDiscs");

void LimitsTrace(Ptr<OutputStreamWrapper> stream, uint32_t oldVal,
                 uint32_t newVal) {
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << newVal
                       << std::endl;
}

void BytesInQueueTrace(Ptr<OutputStreamWrapper> stream, uint32_t oldVal,
                       uint32_t newVal) {
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << newVal
                       << std::endl;
}

static void GoodputSampling(ApplicationContainer app,
                            Ptr<OutputStreamWrapper> stream, float period) {
  Simulator::Schedule(Seconds(period), &GoodputSampling, app, stream, period);
  double goodput;
  uint64_t totalPackets = DynamicCast<PacketSink>(app.Get(0))->GetTotalRx();
  goodput = totalPackets * 8 / (Simulator::Now().GetSeconds() * 1024);
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << goodput
                       << std::endl;
}

static void PingRtt(std::string context, uint16_t, Time rtt) {
  std::cout << context << "=" << rtt.GetMilliSeconds() << " ms" << std::endl;
}

int main(int argc, char *argv[]) {
  std::string bandwidth = "10Mbps";
  std::string delay = "5ms";
  std::string queueDiscType = "PfifoFast";
  uint32_t queueDiscSize = 1000;
  uint32_t netdevicesQueueSize = 50;
  bool bql = false;

  std::string flowsDatarate = "20Mbps";
  uint32_t flowsPacketsSize = 1000;

  float startTime = 0.1F;
  float simDuration = 60;
  float samplingPeriod = 1;

  CommandLine cmd(__FILE__);
  cmd.AddValue("bandwidth", "Bottleneck bandwidth", bandwidth);
  cmd.AddValue("delay", "Bottleneck delay", delay);
  cmd.AddValue("queueDiscType",
               "Bottleneck queue disc type in {PfifoFast, ARED, CoDel, "
               "FqCoDel, PIE, prio}",
               queueDiscType);
  cmd.AddValue("queueDiscSize", "Bottleneck queue disc size in packets",
               queueDiscSize);
  cmd.AddValue("netdevicesQueueSize",
               "Bottleneck netdevices queue size in packets",
               netdevicesQueueSize);
  cmd.AddValue("bql", "Enable byte queue limits on bottleneck netdevices", bql);
  cmd.AddValue("flowsDatarate", "Upload and download flows datarate",
               flowsDatarate);
  cmd.AddValue("flowsPacketsSize", "Upload and download flows packets sizes",
               flowsPacketsSize);
  cmd.AddValue("startTime", "Simulation start time", startTime);
  cmd.AddValue("simDuration", "Simulation duration in seconds", simDuration);
  cmd.AddValue("samplingPeriod", "Goodput sampling period in seconds",
               samplingPeriod);
  cmd.Parse(argc, argv);

  float stopTime = startTime + simDuration;

  NodeContainer n1;
  NodeContainer n2;
  NodeContainer n3;
  n1.Create(1);
  n2.Create(1);
  n3.Create(1);

  PointToPointHelper accessLink;
  accessLink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  accessLink.SetChannelAttribute("Delay", StringValue("0.1ms"));
  accessLink.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("100p"));

  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  bottleneckLink.SetChannelAttribute("Delay", StringValue(delay));
  bottleneckLink.SetQueue(
      "ns3::DropTailQueue", "MaxSize",
      StringValue(std::to_string(netdevicesQueueSize) + "p"));

  InternetStackHelper stack;
  stack.InstallAll();

  TrafficControlHelper tchPfifoFastAccess;
  tchPfifoFastAccess.SetRootQueueDisc("ns3::PfifoFastQueueDisc", "MaxSize",
                                      StringValue("1000p"));

  TrafficControlHelper tchBottleneck;

  if (queueDiscType == "PfifoFast") {
    tchBottleneck.SetRootQueueDisc(
        "ns3::PfifoFastQueueDisc", "MaxSize",
        QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queueDiscSize)));
  } else if (queueDiscType == "ARED") {
    tchBottleneck.SetRootQueueDisc("ns3::RedQueueDisc");
    Config::SetDefault("ns3::RedQueueDisc::ARED", BooleanValue(true));
    Config::SetDefault(
        "ns3::RedQueueDisc::MaxSize",
        QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queueDiscSize)));
  } else if (queueDiscType == "CoDel") {
    tchBottleneck.SetRootQueueDisc("ns3::CoDelQueueDisc");
    Config::SetDefault(
        "ns3::CoDelQueueDisc::MaxSize",
        QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queueDiscSize)));
  } else if (queueDiscType == "FqCoDel") {
    tchBottleneck.SetRootQueueDisc("ns3::FqCoDelQueueDisc");
    Config::SetDefault(
        "ns3::FqCoDelQueueDisc::MaxSize",
        QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queueDiscSize)));
  } else if (queueDiscType == "PIE") {
    tchBottleneck.SetRootQueueDisc("ns3::PieQueueDisc");
    Config::SetDefault(
        "ns3::PieQueueDisc::MaxSize",
        QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queueDiscSize)));
  } else if (queueDiscType == "prio") {
    uint16_t handle = tchBottleneck.SetRootQueueDisc(
        "ns3::PrioQueueDisc", "Priomap",
        StringValue("0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1"));
    TrafficControlHelper::ClassIdList cid =
        tchBottleneck.AddQueueDiscClasses(handle, 2, "ns3::QueueDiscClass");
    tchBottleneck.AddChildQueueDisc(handle, cid[0], "ns3::FifoQueueDisc");
    tchBottleneck.AddChildQueueDisc(handle, cid[1], "ns3::RedQueueDisc");
  } else {
    NS_ABORT_MSG("--queueDiscType not valid");
  }

  if (bql) {
    tchBottleneck.SetQueueLimits("ns3::DynamicQueueLimits");
  }

  NetDeviceContainer devicesAccessLink =
      accessLink.Install(n1.Get(0), n2.Get(0));
  tchPfifoFastAccess.Install(devicesAccessLink);
  Ipv4AddressHelper address;
  address.SetBase("192.168.0.0", "255.255.255.0");
  address.NewNetwork();
  Ipv4InterfaceContainer interfacesAccess = address.Assign(devicesAccessLink);

  NetDeviceContainer devicesBottleneckLink =
      bottleneckLink.Install(n2.Get(0), n3.Get(0));
  QueueDiscContainer qdiscs;
  qdiscs = tchBottleneck.Install(devicesBottleneckLink);

  address.NewNetwork();
  Ipv4InterfaceContainer interfacesBottleneck =
      address.Assign(devicesBottleneckLink);

  Ptr<NetDeviceQueueInterface> interface =
      devicesBottleneckLink.Get(0)->GetObject<NetDeviceQueueInterface>();
  Ptr<NetDeviceQueue> queueInterface = interface->GetTxQueue(0);
  Ptr<DynamicQueueLimits> queueLimits =
      StaticCast<DynamicQueueLimits>(queueInterface->GetQueueLimits());

  AsciiTraceHelper ascii;
  if (bql) {
    queueDiscType = queueDiscType + "-bql";
    Ptr<OutputStreamWrapper> streamLimits =
        ascii.CreateFileStream(queueDiscType + "-limits.txt");
    queueLimits->TraceConnectWithoutContext(
        "Limit", MakeBoundCallback(&LimitsTrace, streamLimits));
  }
  Ptr<Queue<Packet>> queue =
      StaticCast<PointToPointNetDevice>(devicesBottleneckLink.Get(0))
          ->GetQueue();
  Ptr<OutputStreamWrapper> streamBytesInQueue =
      ascii.CreateFileStream(queueDiscType + "-bytesInQueue.txt");
  queue->TraceConnectWithoutContext(
      "BytesInQueue",
      MakeBoundCallback(&BytesInQueueTrace, streamBytesInQueue));

  Ipv4InterfaceContainer n1Interface;
  n1Interface.Add(interfacesAccess.Get(0));

  Ipv4InterfaceContainer n3Interface;
  n3Interface.Add(interfacesBottleneck.Get(1));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Config::SetDefault("ns3::TcpSocket::SegmentSize",
                     UintegerValue(flowsPacketsSize));

  uint16_t port = 7;
  ApplicationContainer uploadApp;
  ApplicationContainer downloadApp;
  ApplicationContainer sourceApps;
  Address addUp(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelperUp("ns3::TcpSocketFactory", addUp);
  sinkHelperUp.SetAttribute("Protocol",
                            TypeIdValue(TcpSocketFactory::GetTypeId()));
  uploadApp.Add(sinkHelperUp.Install(n3));

  InetSocketAddress socketAddressUp =
      InetSocketAddress(n3Interface.GetAddress(0), port);
  OnOffHelper onOffHelperUp("ns3::TcpSocketFactory", Address());
  onOffHelperUp.SetAttribute("Remote", AddressValue(socketAddressUp));
  onOffHelperUp.SetAttribute(
      "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelperUp.SetAttribute(
      "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelperUp.SetAttribute("PacketSize", UintegerValue(flowsPacketsSize));
  onOffHelperUp.SetAttribute("DataRate", StringValue(flowsDatarate));
  sourceApps.Add(onOffHelperUp.Install(n1));

  port = 8;
  Address addDown(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelperDown("ns3::TcpSocketFactory", addDown);
  sinkHelperDown.SetAttribute("Protocol",
                              TypeIdValue(TcpSocketFactory::GetTypeId()));
  downloadApp.Add(sinkHelperDown.Install(n1));

  InetSocketAddress socketAddressDown =
      InetSocketAddress(n1Interface.GetAddress(0), port);
  OnOffHelper onOffHelperDown("ns3::TcpSocketFactory", Address());
  onOffHelperDown.SetAttribute("Remote", AddressValue(socketAddressDown));
  onOffHelperDown.SetAttribute(
      "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelperDown.SetAttribute(
      "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelperDown.SetAttribute("PacketSize", UintegerValue(flowsPacketsSize));
  onOffHelperDown.SetAttribute("DataRate", StringValue(flowsDatarate));
  sourceApps.Add(onOffHelperDown.Install(n3));

  PingHelper ping(n3Interface.GetAddress(0));
  ping.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::QUIET));
  ping.Install(n1);

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::Ping/Rtt",
                  MakeCallback(&PingRtt));

  uploadApp.Start(Seconds(0));
  uploadApp.Stop(Seconds(stopTime));
  downloadApp.Start(Seconds(0));
  downloadApp.Stop(Seconds(stopTime));

  sourceApps.Start(Seconds(0 + 0.1));
  sourceApps.Stop(Seconds(stopTime - 0.1));

  Ptr<OutputStreamWrapper> uploadGoodputStream =
      ascii.CreateFileStream(queueDiscType + "-upGoodput.txt");
  Simulator::Schedule(Seconds(samplingPeriod), &GoodputSampling, uploadApp,
                      uploadGoodputStream, samplingPeriod);
  Ptr<OutputStreamWrapper> downloadGoodputStream =
      ascii.CreateFileStream(queueDiscType + "-downGoodput.txt");
  Simulator::Schedule(Seconds(samplingPeriod), &GoodputSampling, downloadApp,
                      downloadGoodputStream, samplingPeriod);

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  accessLink.EnablePcapAll("queue");

  Simulator::Stop(Seconds(stopTime));
  Simulator::Run();

  flowMonitor->SerializeToXmlFile(queueDiscType + "-flowMonitor.xml", true,
                                  true);

  Simulator::Destroy();
  return 0;
}
