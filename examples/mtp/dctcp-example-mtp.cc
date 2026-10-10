

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mtp-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include <iomanip>
#include <iostream>

using namespace ns3;

std::stringstream filePlotQueue1;
std::stringstream filePlotQueue2;
std::ofstream rxS1R1Throughput;
std::ofstream rxS2R2Throughput;
std::ofstream rxS3R1Throughput;
std::ofstream fairnessIndex;
std::ofstream t1QueueLength;
std::ofstream t2QueueLength;
std::vector<uint64_t> rxS1R1Bytes;
std::vector<uint64_t> rxS2R2Bytes;
std::vector<uint64_t> rxS3R1Bytes;

void PrintProgress(Time interval) {
  std::cout << "Progress to " << std::fixed << std::setprecision(1)
            << Simulator::Now().GetSeconds() << " seconds simulation time"
            << std::endl;
  Simulator::Schedule(interval, &PrintProgress, interval);
}

void TraceS1R1Sink(std::size_t index, Ptr<const Packet> p, const Address &a) {
  rxS1R1Bytes[index] += p->GetSize();
}

void TraceS2R2Sink(std::size_t index, Ptr<const Packet> p, const Address &a) {
  rxS2R2Bytes[index] += p->GetSize();
}

void TraceS3R1Sink(std::size_t index, Ptr<const Packet> p, const Address &a) {
  rxS3R1Bytes[index] += p->GetSize();
}

void InitializeCounters() {
  for (std::size_t i = 0; i < 10; i++) {
    rxS1R1Bytes[i] = 0;
  }
  for (std::size_t i = 0; i < 20; i++) {
    rxS2R2Bytes[i] = 0;
  }
  for (std::size_t i = 0; i < 10; i++) {
    rxS3R1Bytes[i] = 0;
  }
}

void PrintThroughput(Time measurementWindow) {
  for (std::size_t i = 0; i < 10; i++) {
    rxS1R1Throughput << measurementWindow.GetSeconds() << "s " << i << " "
                     << (rxS1R1Bytes[i] * 8) /
                            (measurementWindow.GetSeconds()) / 1e6
                     << std::endl;
  }
  for (std::size_t i = 0; i < 20; i++) {
    rxS2R2Throughput << Simulator::Now().GetSeconds() << "s " << i << " "
                     << (rxS2R2Bytes[i] * 8) /
                            (measurementWindow.GetSeconds()) / 1e6
                     << std::endl;
  }
  for (std::size_t i = 0; i < 10; i++) {
    rxS3R1Throughput << Simulator::Now().GetSeconds() << "s " << i << " "
                     << (rxS3R1Bytes[i] * 8) /
                            (measurementWindow.GetSeconds()) / 1e6
                     << std::endl;
  }
}

void PrintFairness(Time measurementWindow) {
  double average = 0;
  uint64_t sumSquares = 0;
  uint64_t sum = 0;
  double fairness = 0;
  for (std::size_t i = 0; i < 10; i++) {
    sum += rxS1R1Bytes[i];
    sumSquares += (rxS1R1Bytes[i] * rxS1R1Bytes[i]);
  }
  average = ((sum / 10) * 8 / measurementWindow.GetSeconds()) / 1e6;
  fairness = static_cast<double>(sum * sum) / (10 * sumSquares);
  fairnessIndex << "Average throughput for S1-R1 flows: " << std::fixed
                << std::setprecision(2) << average
                << " Mbps; fairness: " << std::fixed << std::setprecision(3)
                << fairness << std::endl;
  average = 0;
  sumSquares = 0;
  sum = 0;
  fairness = 0;
  for (std::size_t i = 0; i < 20; i++) {
    sum += rxS2R2Bytes[i];
    sumSquares += (rxS2R2Bytes[i] * rxS2R2Bytes[i]);
  }
  average = ((sum / 20) * 8 / measurementWindow.GetSeconds()) / 1e6;
  fairness = static_cast<double>(sum * sum) / (20 * sumSquares);
  fairnessIndex << "Average throughput for S2-R2 flows: " << std::fixed
                << std::setprecision(2) << average
                << " Mbps; fairness: " << std::fixed << std::setprecision(3)
                << fairness << std::endl;
  average = 0;
  sumSquares = 0;
  sum = 0;
  fairness = 0;
  for (std::size_t i = 0; i < 10; i++) {
    sum += rxS3R1Bytes[i];
    sumSquares += (rxS3R1Bytes[i] * rxS3R1Bytes[i]);
  }
  average = ((sum / 10) * 8 / measurementWindow.GetSeconds()) / 1e6;
  fairness = static_cast<double>(sum * sum) / (10 * sumSquares);
  fairnessIndex << "Average throughput for S3-R1 flows: " << std::fixed
                << std::setprecision(2) << average
                << " Mbps; fairness: " << std::fixed << std::setprecision(3)
                << fairness << std::endl;
  sum = 0;
  for (std::size_t i = 0; i < 10; i++) {
    sum += rxS1R1Bytes[i];
  }
  for (std::size_t i = 0; i < 20; i++) {
    sum += rxS2R2Bytes[i];
  }
  fairnessIndex << "Aggregate user-level throughput for flows through T1: "
                << static_cast<double>(sum * 8) / 1e9 << " Gbps" << std::endl;
  sum = 0;
  for (std::size_t i = 0; i < 10; i++) {
    sum += rxS3R1Bytes[i];
  }
  for (std::size_t i = 0; i < 10; i++) {
    sum += rxS1R1Bytes[i];
  }
  fairnessIndex << "Aggregate user-level throughput for flows to R1: "
                << static_cast<double>(sum * 8) / 1e9 << " Gbps" << std::endl;
}

void CheckT1QueueSize(Ptr<QueueDisc> queue) {
  uint32_t qSize = queue->GetNPackets();
  Time backlog = Seconds(static_cast<double>(qSize * 1500 * 8) / 1e10);
  t1QueueLength << std::fixed << std::setprecision(2)
                << Simulator::Now().GetSeconds() << " " << qSize << " "
                << backlog.GetMicroSeconds() << std::endl;
  Simulator::Schedule(MilliSeconds(10), &CheckT1QueueSize, queue);
}

void CheckT2QueueSize(Ptr<QueueDisc> queue) {
  uint32_t qSize = queue->GetNPackets();
  Time backlog = Seconds(static_cast<double>(qSize * 1500 * 8) / 1e9);
  t2QueueLength << std::fixed << std::setprecision(2)
                << Simulator::Now().GetSeconds() << " " << qSize << " "
                << backlog.GetMicroSeconds() << std::endl;
  Simulator::Schedule(MilliSeconds(10), &CheckT2QueueSize, queue);
}

int main(int argc, char *argv[]) {
  MtpInterface::Enable(4);
  std::string outputFilePath = ".";
  std::string tcpTypeId = "TcpDctcp";
  Time flowStartupWindow = Seconds(1);
  Time convergenceTime = Seconds(3);
  Time measurementWindow = Seconds(1);
  bool enableSwitchEcn = true;
  Time progressInterval = MilliSeconds(100);

  CommandLine cmd(__FILE__);
  cmd.AddValue("tcpTypeId", "ns-3 TCP TypeId", tcpTypeId);
  cmd.AddValue("flowStartupWindow",
               "startup time window (TCP staggered starts)", flowStartupWindow);
  cmd.AddValue("convergenceTime", "convergence time", convergenceTime);
  cmd.AddValue("measurementWindow", "measurement window", measurementWindow);
  cmd.AddValue("enableSwitchEcn", "enable ECN at switches", enableSwitchEcn);
  cmd.Parse(argc, argv);

  Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                     StringValue("ns3::" + tcpTypeId));

  Time startTime = Seconds(0);
  Time stopTime = flowStartupWindow + convergenceTime + measurementWindow;

  rxS1R1Bytes.reserve(10);
  rxS2R2Bytes.reserve(20);
  rxS3R1Bytes.reserve(10);

  NodeContainer S1;
  NodeContainer S2;
  NodeContainer S3;
  NodeContainer R2;
  Ptr<Node> T1 = CreateObject<Node>();
  Ptr<Node> T2 = CreateObject<Node>();
  Ptr<Node> R1 = CreateObject<Node>();
  S1.Create(10);
  S2.Create(20);
  S3.Create(10);
  R2.Create(20);

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));

  Config::SetDefault("ns3::RedQueueDisc::UseEcn",
                     BooleanValue(enableSwitchEcn));
  Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(false));
  Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(1500));
  Config::SetDefault("ns3::RedQueueDisc::MaxSize",
                     QueueSizeValue(QueueSize("2666p")));
  Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1));
  Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(20));
  Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(60));

  PointToPointHelper pointToPointSR;
  pointToPointSR.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  pointToPointSR.SetChannelAttribute("Delay", StringValue("10us"));

  PointToPointHelper pointToPointT;
  pointToPointT.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
  pointToPointT.SetChannelAttribute("Delay", StringValue("10us"));

  std::vector<NetDeviceContainer> S1T1;
  S1T1.reserve(10);
  std::vector<NetDeviceContainer> S2T1;
  S2T1.reserve(20);
  std::vector<NetDeviceContainer> S3T2;
  S3T2.reserve(10);
  std::vector<NetDeviceContainer> R2T2;
  R2T2.reserve(20);
  NetDeviceContainer T1T2 = pointToPointT.Install(T1, T2);
  NetDeviceContainer R1T2 = pointToPointSR.Install(R1, T2);

  for (std::size_t i = 0; i < 10; i++) {
    Ptr<Node> n = S1.Get(i);
    S1T1.push_back(pointToPointSR.Install(n, T1));
  }
  for (std::size_t i = 0; i < 20; i++) {
    Ptr<Node> n = S2.Get(i);
    S2T1.push_back(pointToPointSR.Install(n, T1));
  }
  for (std::size_t i = 0; i < 10; i++) {
    Ptr<Node> n = S3.Get(i);
    S3T2.push_back(pointToPointSR.Install(n, T2));
  }
  for (std::size_t i = 0; i < 20; i++) {
    Ptr<Node> n = R2.Get(i);
    R2T2.push_back(pointToPointSR.Install(n, T2));
  }

  InternetStackHelper stack;
  stack.InstallAll();

  TrafficControlHelper tchRed10;
  tchRed10.SetRootQueueDisc(
      "ns3::RedQueueDisc", "LinkBandwidth", StringValue("10Gbps"), "LinkDelay",
      StringValue("10us"), "MinTh", DoubleValue(50), "MaxTh", DoubleValue(150));
  QueueDiscContainer queueDiscs1 = tchRed10.Install(T1T2);

  TrafficControlHelper tchRed1;
  tchRed1.SetRootQueueDisc(
      "ns3::RedQueueDisc", "LinkBandwidth", StringValue("1Gbps"), "LinkDelay",
      StringValue("10us"), "MinTh", DoubleValue(20), "MaxTh", DoubleValue(60));
  QueueDiscContainer queueDiscs2 = tchRed1.Install(R1T2.Get(1));
  for (std::size_t i = 0; i < 10; i++) {
    tchRed1.Install(S1T1[i].Get(1));
  }
  for (std::size_t i = 0; i < 20; i++) {
    tchRed1.Install(S2T1[i].Get(1));
  }
  for (std::size_t i = 0; i < 10; i++) {
    tchRed1.Install(S3T2[i].Get(1));
  }
  for (std::size_t i = 0; i < 20; i++) {
    tchRed1.Install(R2T2[i].Get(1));
  }

  Ipv4AddressHelper address;
  std::vector<Ipv4InterfaceContainer> ipS1T1;
  ipS1T1.reserve(10);
  std::vector<Ipv4InterfaceContainer> ipS2T1;
  ipS2T1.reserve(20);
  std::vector<Ipv4InterfaceContainer> ipS3T2;
  ipS3T2.reserve(10);
  std::vector<Ipv4InterfaceContainer> ipR2T2;
  ipR2T2.reserve(20);
  address.SetBase("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipT1T2 = address.Assign(T1T2);
  address.SetBase("192.168.0.0", "255.255.255.0");
  Ipv4InterfaceContainer ipR1T2 = address.Assign(R1T2);
  address.SetBase("10.1.1.0", "255.255.255.0");
  for (std::size_t i = 0; i < 10; i++) {
    ipS1T1.push_back(address.Assign(S1T1[i]));
    address.NewNetwork();
  }
  address.SetBase("10.2.1.0", "255.255.255.0");
  for (std::size_t i = 0; i < 20; i++) {
    ipS2T1.push_back(address.Assign(S2T1[i]));
    address.NewNetwork();
  }
  address.SetBase("10.3.1.0", "255.255.255.0");
  for (std::size_t i = 0; i < 10; i++) {
    ipS3T2.push_back(address.Assign(S3T2[i]));
    address.NewNetwork();
  }
  address.SetBase("10.4.1.0", "255.255.255.0");
  for (std::size_t i = 0; i < 20; i++) {
    ipR2T2.push_back(address.Assign(R2T2[i]));
    address.NewNetwork();
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  std::vector<Ptr<PacketSink>> r2Sinks;
  r2Sinks.reserve(20);
  for (std::size_t i = 0; i < 20; i++) {
    uint16_t port = 50000 + i;
    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(R2.Get(i));
    Ptr<PacketSink> packetSink = sinkApp.Get(0)->GetObject<PacketSink>();
    r2Sinks.push_back(packetSink);
    sinkApp.Start(startTime);
    sinkApp.Stop(stopTime);

    OnOffHelper clientHelper1("ns3::TcpSocketFactory", Address());
    clientHelper1.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper1.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    clientHelper1.SetAttribute("DataRate", DataRateValue(DataRate("1Gbps")));
    clientHelper1.SetAttribute("PacketSize", UintegerValue(1000));

    ApplicationContainer clientApps1;
    AddressValue remoteAddress(
        InetSocketAddress(ipR2T2[i].GetAddress(0), port));
    clientHelper1.SetAttribute("Remote", remoteAddress);
    clientApps1.Add(clientHelper1.Install(S2.Get(i)));
    clientApps1.Start(i * flowStartupWindow / 20 + startTime +
                      MilliSeconds(i * 5));
    clientApps1.Stop(stopTime);
  }

  std::vector<Ptr<PacketSink>> s1r1Sinks;
  std::vector<Ptr<PacketSink>> s3r1Sinks;
  s1r1Sinks.reserve(10);
  s3r1Sinks.reserve(10);
  for (std::size_t i = 0; i < 20; i++) {
    uint16_t port = 50000 + i;
    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(R1);
    Ptr<PacketSink> packetSink = sinkApp.Get(0)->GetObject<PacketSink>();
    if (i < 10) {
      s1r1Sinks.push_back(packetSink);
    } else {
      s3r1Sinks.push_back(packetSink);
    }
    sinkApp.Start(startTime);
    sinkApp.Stop(stopTime);

    OnOffHelper clientHelper1("ns3::TcpSocketFactory", Address());
    clientHelper1.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper1.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    clientHelper1.SetAttribute("DataRate", DataRateValue(DataRate("1Gbps")));
    clientHelper1.SetAttribute("PacketSize", UintegerValue(1000));

    ApplicationContainer clientApps1;
    AddressValue remoteAddress(InetSocketAddress(ipR1T2.GetAddress(0), port));
    clientHelper1.SetAttribute("Remote", remoteAddress);
    if (i < 10) {
      clientApps1.Add(clientHelper1.Install(S1.Get(i)));
      clientApps1.Start(i * flowStartupWindow / 10 + startTime +
                        MilliSeconds(i * 5));
    } else {
      clientApps1.Add(clientHelper1.Install(S3.Get(i - 10)));
      clientApps1.Start((i - 10) * flowStartupWindow / 10 + startTime +
                        MilliSeconds(i * 5));
    }

    clientApps1.Stop(stopTime);
  }

  rxS1R1Throughput.open("dctcp-example-s1-r1-throughput.dat", std::ios::out);
  rxS1R1Throughput << "#Time(s) flow thruput(Mb/s)" << std::endl;
  rxS2R2Throughput.open("dctcp-example-s2-r2-throughput.dat", std::ios::out);
  rxS2R2Throughput << "#Time(s) flow thruput(Mb/s)" << std::endl;
  rxS3R1Throughput.open("dctcp-example-s3-r1-throughput.dat", std::ios::out);
  rxS3R1Throughput << "#Time(s) flow thruput(Mb/s)" << std::endl;
  fairnessIndex.open("dctcp-example-fairness.dat", std::ios::out);
  t1QueueLength.open("dctcp-example-t1-length.dat", std::ios::out);
  t1QueueLength << "#Time(s) qlen(pkts) qlen(us)" << std::endl;
  t2QueueLength.open("dctcp-example-t2-length.dat", std::ios::out);
  t2QueueLength << "#Time(s) qlen(pkts) qlen(us)" << std::endl;
  for (std::size_t i = 0; i < 10; i++) {
    s1r1Sinks[i]->TraceConnectWithoutContext(
        "Rx", MakeBoundCallback(&TraceS1R1Sink, i));
  }
  for (std::size_t i = 0; i < 20; i++) {
    r2Sinks[i]->TraceConnectWithoutContext(
        "Rx", MakeBoundCallback(&TraceS2R2Sink, i));
  }
  for (std::size_t i = 0; i < 10; i++) {
    s3r1Sinks[i]->TraceConnectWithoutContext(
        "Rx", MakeBoundCallback(&TraceS3R1Sink, i));
  }
  Simulator::Schedule(flowStartupWindow + convergenceTime, &InitializeCounters);
  Simulator::Schedule(flowStartupWindow + convergenceTime + measurementWindow,
                      &PrintThroughput, measurementWindow);
  Simulator::Schedule(flowStartupWindow + convergenceTime + measurementWindow,
                      &PrintFairness, measurementWindow);
  Simulator::Schedule(progressInterval, &PrintProgress, progressInterval);
  Simulator::Schedule(flowStartupWindow + convergenceTime, &CheckT1QueueSize,
                      queueDiscs1.Get(0));
  Simulator::Schedule(flowStartupWindow + convergenceTime, &CheckT2QueueSize,
                      queueDiscs2.Get(0));
  Simulator::Stop(stopTime + TimeStep(1));

  Simulator::Run();

  rxS1R1Throughput.close();
  rxS2R2Throughput.close();
  rxS3R1Throughput.close();
  fairnessIndex.close();
  t1QueueLength.close();
  t2QueueLength.close();
  Simulator::Destroy();
  return 0;
}
