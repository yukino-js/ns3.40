

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FqCoDelL4SExample");

uint32_t checkTimes;
double avgQueueDiscSize;

uint32_t g_n0BytesReceived = 0;
uint32_t g_n1BytesReceived = 0;
uint32_t g_marksObserved = 0;
uint32_t g_dropsObserved = 0;

void TraceN0Cwnd(std::ofstream *ofStream, uint32_t oldCwnd, uint32_t newCwnd) {
  *ofStream << Simulator::Now().GetSeconds() << " "
            << static_cast<double>(newCwnd) / 1448 << std::endl;
}

void TraceN1Cwnd(std::ofstream *ofStream, uint32_t oldCwnd, uint32_t newCwnd) {
  *ofStream << Simulator::Now().GetSeconds() << " "
            << static_cast<double>(newCwnd) / 1448 << std::endl;
}

void TraceN0Rtt(std::ofstream *ofStream, Time oldRtt, Time newRtt) {
  *ofStream << Simulator::Now().GetSeconds() << " "
            << newRtt.GetSeconds() * 1000 << std::endl;
}

void TraceN1Rtt(std::ofstream *ofStream, Time oldRtt, Time newRtt) {
  *ofStream << Simulator::Now().GetSeconds() << " "
            << newRtt.GetSeconds() * 1000 << std::endl;
}

void TracePingRtt(std::ofstream *ofStream, uint16_t seqNo, Time rtt) {
  *ofStream << Simulator::Now().GetSeconds() << " " << seqNo << " "
            << rtt.GetSeconds() * 1000 << std::endl;
}

void TraceN0Rx(Ptr<const Packet> packet, const Address &address) {
  g_n0BytesReceived += packet->GetSize();
}

void TraceN1Rx(Ptr<const Packet> packet, const Address &address) {
  g_n1BytesReceived += packet->GetSize();
}

void TraceDrop(std::ofstream *ofStream, Ptr<const QueueDiscItem> item) {
  *ofStream << Simulator::Now().GetSeconds() << " " << std::hex << item->Hash()
            << std::endl;
  g_dropsObserved++;
}

void TraceMark(std::ofstream *ofStream, Ptr<const QueueDiscItem> item,
               const char *reason) {
  *ofStream << Simulator::Now().GetSeconds() << " " << std::hex << item->Hash()
            << std::endl;
  g_marksObserved++;
}

void TraceQueueLength(std::ofstream *ofStream, DataRate linkRate,
                      uint32_t oldVal, uint32_t newVal) {
  *ofStream << Simulator::Now().GetSeconds() << " " << std::fixed
            << static_cast<double>(newVal * 8) / (linkRate.GetBitRate() / 1000)
            << std::endl;
}

void TraceDropsFrequency(std::ofstream *ofStream, Time dropsSamplingInterval) {
  *ofStream << Simulator::Now().GetSeconds() << " " << g_dropsObserved
            << std::endl;
  g_dropsObserved = 0;
  Simulator::Schedule(dropsSamplingInterval, &TraceDropsFrequency, ofStream,
                      dropsSamplingInterval);
}

void TraceMarksFrequency(std::ofstream *ofStream, Time marksSamplingInterval) {
  *ofStream << Simulator::Now().GetSeconds() << " " << g_marksObserved
            << std::endl;
  g_marksObserved = 0;
  Simulator::Schedule(marksSamplingInterval, &TraceMarksFrequency, ofStream,
                      marksSamplingInterval);
}

void TraceN0Throughput(std::ofstream *ofStream, Time throughputInterval) {
  *ofStream << Simulator::Now().GetSeconds() << " "
            << g_n0BytesReceived * 8 / throughputInterval.GetSeconds() / 1e6
            << std::endl;
  g_n0BytesReceived = 0;
  Simulator::Schedule(throughputInterval, &TraceN0Throughput, ofStream,
                      throughputInterval);
}

void TraceN1Throughput(std::ofstream *ofStream, Time throughputInterval) {
  *ofStream << Simulator::Now().GetSeconds() << " "
            << g_n1BytesReceived * 8 / throughputInterval.GetSeconds() / 1e6
            << std::endl;
  g_n1BytesReceived = 0;
  Simulator::Schedule(throughputInterval, &TraceN1Throughput, ofStream,
                      throughputInterval);
}

void ScheduleN0TcpCwndTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContext(
      "/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
      MakeBoundCallback(&TraceN0Cwnd, ofStream));
}

void ScheduleN0TcpRttTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContext(
      "/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/RTT",
      MakeBoundCallback(&TraceN0Rtt, ofStream));
}

void ScheduleN0PacketSinkConnection() {
  Config::ConnectWithoutContext(
      "/NodeList/6/ApplicationList/*/$ns3::PacketSink/Rx",
      MakeCallback(&TraceN0Rx));
}

void ScheduleN1TcpCwndTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContext(
      "/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
      MakeBoundCallback(&TraceN1Cwnd, ofStream));
}

void ScheduleN1TcpRttTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContext(
      "/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTT",
      MakeBoundCallback(&TraceN1Rtt, ofStream));
}

void ScheduleN1PacketSinkConnection() {
  Config::ConnectWithoutContext(
      "/NodeList/7/ApplicationList/*/$ns3::PacketSink/Rx",
      MakeCallback(&TraceN1Rx));
}

static void PacketDequeue(std::ofstream *n0OfStream, std::ofstream *n1OfStream,
                          Ptr<const QueueDiscItem> item) {
  Ptr<Packet> p = item->GetPacket();
  Ptr<const Ipv4QueueDiscItem> iqdi = Ptr<const Ipv4QueueDiscItem>(
      dynamic_cast<const Ipv4QueueDiscItem *>(PeekPointer(item)));
  Ipv4Address address = iqdi->GetHeader().GetDestination();
  Time qDelay = Simulator::Now() - item->GetTimeStamp();
  if (address == "192.168.2.2") {
    *n0OfStream << Simulator::Now().GetSeconds() << " "
                << qDelay.GetMicroSeconds() / 1000.0 << std::endl;
  } else if (address == "192.168.3.2") {
    *n1OfStream << Simulator::Now().GetSeconds() << " "
                << qDelay.GetMicroSeconds() / 1000.0 << std::endl;
  }
}

int main(int argc, char *argv[]) {
  Time stopTime = Seconds(70);
  Time baseRtt = MilliSeconds(80);
  uint32_t pingSize = 100;
  Time pingInterval = MilliSeconds(100);
  Time marksSamplingInterval = MilliSeconds(100);
  Time throughputSamplingInterval = MilliSeconds(200);
  DataRate bottleneckRate("100Mbps");

  std::string dir = "results/FqCoDel-L4S/";
  std::string dirToSave = "mkdir -p " + dir;
  if (system(dirToSave.c_str()) == -1) {
    exit(1);
  }

  std::string pingTraceFile = dir + "ping.dat";
  std::string n0TcpRttTraceFile = dir + "n0-tcp-rtt.dat";
  std::string n0TcpCwndTraceFile = dir + "n0-tcp-cwnd.dat";
  std::string n0TcpThroughputTraceFile = dir + "n0-tcp-throughput.dat";
  std::string n1TcpRttTraceFile = dir + "n1-tcp-rtt.dat";
  std::string n1TcpCwndTraceFile = dir + "n1-tcp-cwnd.dat";
  std::string n1TcpThroughputTraceFile = dir + "n1-tcp-throughput.dat";
  std::string dropTraceFile = dir + "drops.dat";
  std::string dropsFrequencyTraceFile = dir + "drops-frequency.dat";
  std::string lengthTraceFile = dir + "length.dat";
  std::string markTraceFile = dir + "mark.dat";
  std::string marksFrequencyTraceFile = dir + "marks-frequency.dat";
  std::string queueDelayN0TraceFile = dir + "queue-delay-n0.dat";
  std::string queueDelayN1TraceFile = dir + "queue-delay-n1.dat";

  bool enablePcap = false;
  bool useCeThreshold = false;
  Time ceThreshold = MilliSeconds(1);
  std::string n0TcpType = "bic";
  std::string n1TcpType = "";
  bool enableN1Tcp = false;
  bool useEcn = true;
  std::string queueType = "fq";
  std::string linkDataRate = "1Gbps";
  uint32_t scenarioNum = 0;

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(8192000));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(8192000));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(10));
  Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                     TypeIdValue(TcpPrrRecovery::GetTypeId()));

  CommandLine cmd;
  cmd.AddValue("n0TcpType", "n0 TCP type (bic, dctcp, or reno)", n0TcpType);
  cmd.AddValue("n1TcpType", "n1 TCP type (bic, dctcp, or reno)", n1TcpType);
  cmd.AddValue("scenarioNum",
               "Scenario number from the scenarios available in the file (1-9)",
               scenarioNum);
  cmd.AddValue("bottleneckQueueType", "n2 queue type (fq or codel)", queueType);
  cmd.AddValue("baseRtt", "base RTT", baseRtt);
  cmd.AddValue("useCeThreshold", "use CE Threshold", useCeThreshold);
  cmd.AddValue("useEcn", "use ECN", useEcn);
  cmd.AddValue("ceThreshold", "CoDel CE threshold", ceThreshold);
  cmd.AddValue("bottleneckRate", "data rate of bottleneck", bottleneckRate);
  cmd.AddValue("linkRate", "data rate of edge link", linkDataRate);
  cmd.AddValue("stopTime", "simulation stop time", stopTime);
  cmd.AddValue("enablePcap", "enable Pcap", enablePcap);
  cmd.AddValue("pingTraceFile", "filename for ping tracing", pingTraceFile);
  cmd.AddValue("n0TcpRttTraceFile", "filename for n0 rtt tracing",
               n0TcpRttTraceFile);
  cmd.AddValue("n0TcpCwndTraceFile", "filename for n0 cwnd tracing",
               n0TcpCwndTraceFile);
  cmd.AddValue("n0TcpThroughputTraceFile", "filename for n0 throughput tracing",
               n0TcpThroughputTraceFile);
  cmd.AddValue("n1TcpRttTraceFile", "filename for n1 rtt tracing",
               n1TcpRttTraceFile);
  cmd.AddValue("n1TcpCwndTraceFile", "filename for n1 cwnd tracing",
               n1TcpCwndTraceFile);
  cmd.AddValue("n1TcpThroughputTraceFile", "filename for n1 throughput tracing",
               n1TcpThroughputTraceFile);
  cmd.AddValue("dropTraceFile", "filename for n2 drops tracing", dropTraceFile);
  cmd.AddValue("dropsFrequencyTraceFile",
               "filename for n2 drop frequency tracing",
               dropsFrequencyTraceFile);
  cmd.AddValue("lengthTraceFile", "filename for n2 queue length tracing",
               lengthTraceFile);
  cmd.AddValue("markTraceFile", "filename for n2 mark tracing", markTraceFile);
  cmd.AddValue("marksFrequencyTraceFile",
               "filename for n2 mark frequency tracing",
               marksFrequencyTraceFile);
  cmd.AddValue("queueDelayN0TraceFile", "filename for n0 queue delay tracing",
               queueDelayN0TraceFile);
  cmd.AddValue("queueDelayN1TraceFile", "filename for n1 queue delay tracing",
               queueDelayN1TraceFile);
  cmd.Parse(argc, argv);
  Time oneWayDelay = baseRtt / 2;
  TypeId n0TcpTypeId;
  TypeId n1TcpTypeId;
  TypeId queueTypeId;
  if (!scenarioNum) {
    if (useEcn) {
      Config::SetDefault("ns3::TcpSocketBase::UseEcn", StringValue("On"));
    }

    if (n0TcpType == "reno") {
      n0TcpTypeId = TcpNewReno::GetTypeId();
    } else if (n0TcpType == "bic") {
      n0TcpTypeId = TcpBic::GetTypeId();
    } else if (n0TcpType == "dctcp") {
      n0TcpTypeId = TcpDctcp::GetTypeId();
    } else {
      NS_FATAL_ERROR("Fatal error:  tcp unsupported");
    }

    if (n1TcpType == "reno") {
      enableN1Tcp = true;
      n1TcpTypeId = TcpNewReno::GetTypeId();
    } else if (n1TcpType == "bic") {
      enableN1Tcp = true;
      n1TcpTypeId = TcpBic::GetTypeId();
    } else if (n1TcpType == "dctcp") {
      enableN1Tcp = true;
      n1TcpTypeId = TypeId::LookupByName("ns3::TcpDctcp");
    } else if (n1TcpType.empty()) {
      NS_LOG_DEBUG("No N1 TCP selected");
    } else {
      NS_FATAL_ERROR("Fatal error:  tcp unsupported");
    }

    if (queueType == "fq") {
      queueTypeId = FqCoDelQueueDisc::GetTypeId();
    } else if (queueType == "codel") {
      queueTypeId = CoDelQueueDisc::GetTypeId();
    } else {
      NS_FATAL_ERROR("Fatal error:  queueType unsupported");
    }
    if (useCeThreshold) {
      Config::SetDefault("ns3::FqCoDelQueueDisc::CeThreshold",
                         TimeValue(ceThreshold));
    }
  } else if (scenarioNum == 1 || scenarioNum == 2 || scenarioNum == 5 ||
             scenarioNum == 6) {
    if (scenarioNum == 2 || scenarioNum == 6) {
      Config::SetDefault("ns3::TcpSocketBase::UseEcn", StringValue("On"));
    }
    n0TcpTypeId = TcpBic::GetTypeId();
    if (scenarioNum == 5 || scenarioNum == 6) {
      enableN1Tcp = true;
      n1TcpTypeId = TcpBic::GetTypeId();
    }
    queueTypeId = FqCoDelQueueDisc::GetTypeId();
  } else if (scenarioNum == 3 || scenarioNum == 4 || scenarioNum == 7 ||
             scenarioNum == 8 || scenarioNum == 9) {
    Config::SetDefault("ns3::TcpSocketBase::UseEcn", StringValue("On"));
    n0TcpTypeId = TcpDctcp::GetTypeId();
    queueTypeId = FqCoDelQueueDisc::GetTypeId();
    oneWayDelay = MicroSeconds(500);
    Config::SetDefault("ns3::FqCoDelQueueDisc::CeThreshold",
                       TimeValue(MilliSeconds(1)));
    if (scenarioNum == 9) {
      n0TcpTypeId = TcpBic::GetTypeId();
      oneWayDelay = MilliSeconds(40);
    }
    if (scenarioNum == 4 || scenarioNum == 8 || scenarioNum == 9) {
      Config::SetDefault("ns3::FqCoDelQueueDisc::UseL4s", BooleanValue(true));
      Config::SetDefault("ns3::TcpDctcp::UseEct0", BooleanValue(false));
    }
    if (scenarioNum == 7 || scenarioNum == 8 || scenarioNum == 9) {
      enableN1Tcp = true;
      n1TcpTypeId = TcpDctcp::GetTypeId();
    }
  } else {
    NS_FATAL_ERROR("Fatal error:  scenario unavailble");
  }

  std::ofstream pingOfStream;
  pingOfStream.open(pingTraceFile, std::ofstream::out);
  std::ofstream n0TcpRttOfStream;
  n0TcpRttOfStream.open(n0TcpRttTraceFile, std::ofstream::out);
  std::ofstream n0TcpCwndOfStream;
  n0TcpCwndOfStream.open(n0TcpCwndTraceFile, std::ofstream::out);
  std::ofstream n0TcpThroughputOfStream;
  n0TcpThroughputOfStream.open(n0TcpThroughputTraceFile, std::ofstream::out);
  std::ofstream n1TcpRttOfStream;
  n1TcpRttOfStream.open(n1TcpRttTraceFile, std::ofstream::out);
  std::ofstream n1TcpCwndOfStream;
  n1TcpCwndOfStream.open(n1TcpCwndTraceFile, std::ofstream::out);
  std::ofstream n1TcpThroughputOfStream;
  n1TcpThroughputOfStream.open(n1TcpThroughputTraceFile, std::ofstream::out);

  std::ofstream dropOfStream;
  dropOfStream.open(dropTraceFile, std::ofstream::out);
  std::ofstream markOfStream;
  markOfStream.open(markTraceFile, std::ofstream::out);
  std::ofstream dropsFrequencyOfStream;
  dropsFrequencyOfStream.open(dropsFrequencyTraceFile, std::ofstream::out);
  std::ofstream marksFrequencyOfStream;
  marksFrequencyOfStream.open(marksFrequencyTraceFile, std::ofstream::out);
  std::ofstream lengthOfStream;
  lengthOfStream.open(lengthTraceFile, std::ofstream::out);
  std::ofstream queueDelayN0OfStream;
  queueDelayN0OfStream.open(queueDelayN0TraceFile, std::ofstream::out);
  std::ofstream queueDelayN1OfStream;
  queueDelayN1OfStream.open(queueDelayN1TraceFile, std::ofstream::out);

  Ptr<Node> pingServer = CreateObject<Node>();
  Ptr<Node> n0Server = CreateObject<Node>();
  Ptr<Node> n1Server = CreateObject<Node>();
  Ptr<Node> n2 = CreateObject<Node>();
  Ptr<Node> n3 = CreateObject<Node>();
  Ptr<Node> pingClient = CreateObject<Node>();
  Ptr<Node> n4Client = CreateObject<Node>();
  Ptr<Node> n5Client = CreateObject<Node>();

  NetDeviceContainer pingServerDevices;
  NetDeviceContainer n0ServerDevices;
  NetDeviceContainer n1ServerDevices;
  NetDeviceContainer n2n3Devices;
  NetDeviceContainer pingClientDevices;
  NetDeviceContainer n4ClientDevices;
  NetDeviceContainer n5ClientDevices;

  PointToPointHelper p2p;
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize",
               QueueSizeValue(QueueSize("3p")));
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(linkDataRate)));
  p2p.SetChannelAttribute("Delay", TimeValue(oneWayDelay));
  pingServerDevices = p2p.Install(n2, pingServer);
  n0ServerDevices = p2p.Install(n2, n0Server);

  if (scenarioNum == 9) {
    p2p.SetChannelAttribute("Delay", TimeValue(MicroSeconds(500)));
  }
  n1ServerDevices = p2p.Install(n2, n1Server);
  p2p.SetChannelAttribute("Delay", TimeValue(MicroSeconds(1)));
  n2n3Devices = p2p.Install(n2, n3);
  pingClientDevices = p2p.Install(n3, pingClient);
  n4ClientDevices = p2p.Install(n3, n4Client);
  n5ClientDevices = p2p.Install(n3, n5Client);
  Ptr<PointToPointNetDevice> p =
      n2n3Devices.Get(0)->GetObject<PointToPointNetDevice>();
  p->SetAttribute("DataRate", DataRateValue(bottleneckRate));

  InternetStackHelper stackHelper;
  stackHelper.InstallAll();

  Ptr<TcpL4Protocol> proto;
  proto = n4Client->GetObject<TcpL4Protocol>();
  proto->SetAttribute("SocketType", TypeIdValue(n0TcpTypeId));
  proto = n0Server->GetObject<TcpL4Protocol>();
  proto->SetAttribute("SocketType", TypeIdValue(n0TcpTypeId));
  if (enableN1Tcp) {
    proto = n5Client->GetObject<TcpL4Protocol>();
    proto->SetAttribute("SocketType", TypeIdValue(n1TcpTypeId));
    proto = n1Server->GetObject<TcpL4Protocol>();
    proto->SetAttribute("SocketType", TypeIdValue(n1TcpTypeId));
  }

  TrafficControlHelper tchFq;
  tchFq.SetRootQueueDisc("ns3::FqCoDelQueueDisc");
  tchFq.SetQueueLimits("ns3::DynamicQueueLimits", "HoldTime",
                       StringValue("1ms"));
  tchFq.Install(pingServerDevices);
  tchFq.Install(n0ServerDevices);
  tchFq.Install(n1ServerDevices);
  tchFq.Install(n2n3Devices.Get(1));
  tchFq.Install(pingClientDevices);
  tchFq.Install(n4ClientDevices);
  tchFq.Install(n5ClientDevices);
  TrafficControlHelper tchN2;
  tchN2.SetRootQueueDisc(queueTypeId.GetName());
  tchN2.SetQueueLimits("ns3::DynamicQueueLimits", "HoldTime",
                       StringValue("1000ms"));
  tchN2.Install(n2n3Devices.Get(0));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer pingServerIfaces = ipv4.Assign(pingServerDevices);
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer n0ServerIfaces = ipv4.Assign(n0ServerDevices);
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer secondServerIfaces = ipv4.Assign(n1ServerDevices);
  ipv4.SetBase("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer n2n3Ifaces = ipv4.Assign(n2n3Devices);
  ipv4.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer pingClientIfaces = ipv4.Assign(pingClientDevices);
  ipv4.SetBase("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer n4ClientIfaces = ipv4.Assign(n4ClientDevices);
  ipv4.SetBase("192.168.3.0", "255.255.255.0");
  Ipv4InterfaceContainer n5ClientIfaces = ipv4.Assign(n5ClientDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  PingHelper pingHelper(Ipv4Address("192.168.1.2"));
  pingHelper.SetAttribute("Interval", TimeValue(pingInterval));
  pingHelper.SetAttribute("Size", UintegerValue(pingSize));
  ApplicationContainer pingContainer = pingHelper.Install(pingServer);
  Ptr<Ping> ping = pingContainer.Get(0)->GetObject<Ping>();
  ping->TraceConnectWithoutContext(
      "Rtt", MakeBoundCallback(&TracePingRtt, &pingOfStream));
  pingContainer.Start(Seconds(1));
  pingContainer.Stop(stopTime - Seconds(1));

  BulkSendHelper tcp("ns3::TcpSocketFactory", Address());
  tcp.SetAttribute("MaxBytes", UintegerValue(7500000000));
  uint16_t n4Port = 5000;
  ApplicationContainer n0App;
  InetSocketAddress n0DestAddress(n4ClientIfaces.GetAddress(1), n4Port);
  tcp.SetAttribute("Remote", AddressValue(n0DestAddress));
  n0App = tcp.Install(n0Server);
  n0App.Start(Seconds(5));
  n0App.Stop(stopTime - Seconds(1));

  Address n4SinkAddress(InetSocketAddress(Ipv4Address::GetAny(), n4Port));
  PacketSinkHelper n4SinkHelper("ns3::TcpSocketFactory", n4SinkAddress);
  ApplicationContainer n4SinkApp;
  n4SinkApp = n4SinkHelper.Install(n4Client);
  n4SinkApp.Start(Seconds(5));
  n4SinkApp.Stop(stopTime - MilliSeconds(500));

  if (enableN1Tcp) {
    uint16_t n5Port = 5000;
    ApplicationContainer secondApp;
    InetSocketAddress n1DestAddress(n5ClientIfaces.GetAddress(1), n5Port);
    tcp.SetAttribute("Remote", AddressValue(n1DestAddress));
    secondApp = tcp.Install(n1Server);
    secondApp.Start(Seconds(15));
    secondApp.Stop(stopTime - Seconds(1));

    Address n5SinkAddress(InetSocketAddress(Ipv4Address::GetAny(), n5Port));
    PacketSinkHelper n5SinkHelper("ns3::TcpSocketFactory", n5SinkAddress);
    ApplicationContainer n5SinkApp;
    n5SinkApp = n5SinkHelper.Install(n5Client);
    n5SinkApp.Start(Seconds(15));
    n5SinkApp.Stop(stopTime - MilliSeconds(500));
  }

  Ptr<TrafficControlLayer> tc;
  Ptr<QueueDisc> qd;
  tc = n2n3Devices.Get(0)->GetNode()->GetObject<TrafficControlLayer>();
  qd = tc->GetRootQueueDiscOnDevice(n2n3Devices.Get(0));
  qd->TraceConnectWithoutContext("Drop",
                                 MakeBoundCallback(&TraceDrop, &dropOfStream));
  qd->TraceConnectWithoutContext("Mark",
                                 MakeBoundCallback(&TraceMark, &markOfStream));
  qd->TraceConnectWithoutContext(
      "BytesInQueue",
      MakeBoundCallback(&TraceQueueLength, &lengthOfStream, bottleneckRate));
  qd->TraceConnectWithoutContext(
      "Dequeue", MakeBoundCallback(&PacketDequeue, &queueDelayN0OfStream,
                                   &queueDelayN1OfStream));

  Simulator::Schedule(Seconds(5) + MilliSeconds(100),
                      &ScheduleN0TcpRttTraceConnection, &n0TcpRttOfStream);
  Simulator::Schedule(Seconds(5) + MilliSeconds(100),
                      &ScheduleN0TcpCwndTraceConnection, &n0TcpCwndOfStream);
  Simulator::Schedule(Seconds(5) + MilliSeconds(100),
                      &ScheduleN0PacketSinkConnection);
  Simulator::Schedule(throughputSamplingInterval, &TraceN0Throughput,
                      &n0TcpThroughputOfStream, throughputSamplingInterval);
  if (enableN1Tcp) {
    Simulator::Schedule(Seconds(15) + MilliSeconds(100),
                        &ScheduleN1TcpRttTraceConnection, &n1TcpRttOfStream);
    Simulator::Schedule(Seconds(15) + MilliSeconds(100),
                        &ScheduleN1TcpCwndTraceConnection, &n1TcpCwndOfStream);
    Simulator::Schedule(Seconds(15) + MilliSeconds(100),
                        &ScheduleN1PacketSinkConnection);
  }
  Simulator::Schedule(throughputSamplingInterval, &TraceN1Throughput,
                      &n1TcpThroughputOfStream, throughputSamplingInterval);
  Simulator::Schedule(marksSamplingInterval, &TraceMarksFrequency,
                      &marksFrequencyOfStream, marksSamplingInterval);
  Simulator::Schedule(marksSamplingInterval, &TraceDropsFrequency,
                      &dropsFrequencyOfStream, marksSamplingInterval);

  if (enablePcap) {
    p2p.EnablePcapAll("FqCoDel-L4S-example", false);
  }

  Simulator::Stop(stopTime);
  Simulator::Run();

  pingOfStream.close();
  n0TcpCwndOfStream.close();
  n0TcpRttOfStream.close();
  n0TcpThroughputOfStream.close();
  n1TcpCwndOfStream.close();
  n1TcpRttOfStream.close();
  n1TcpThroughputOfStream.close();
  dropOfStream.close();
  markOfStream.close();
  dropsFrequencyOfStream.close();
  marksFrequencyOfStream.close();
  lengthOfStream.close();
  queueDelayN0OfStream.close();
  queueDelayN1OfStream.close();

  return 0;
}
