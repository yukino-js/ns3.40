

#include "tcp-swift.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/enum.h"
#include "ns3/error-model.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/tcp-header.h"
#include "ns3/traffic-control-module.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpSwiftSimulator");

static std::vector<uint32_t> rxPkts;

static void CountRxPkts(uint32_t sinkId, Ptr<const Packet> packet,
                        const Address &srcAddr) {
  rxPkts[sinkId]++;
}

static void PrintRxCount() {
  uint32_t size = rxPkts.size();
  NS_LOG_UNCOND("RxPkts:");
  for (uint32_t i = 0; i < size; i++) {
    NS_LOG_UNCOND("---SinkId: " << i << " RxPkts: " << rxPkts.at(i));
  }
}

int main(int argc, char *argv[]) {
  uint32_t nLeaf = 3;
  uint16_t tcpTrafficPort = 5000;
  uint16_t udpTrafficPort = 7000;
  std::string transport_prot = "TcpSwift";
  double error_p = 0.0;
  std::string bottleneck_bandwidth = "2Gbps";
  std::string bottleneck_delay = "5us";
  std::string access_bandwidth = "10Gbps";
  std::string access_delay = "2us";
  std::string prefix_file_name = "TcpSwiftSimulator";
  uint64_t data_mbytes = 0;
  uint32_t mtu_bytes = 1500;
  double duration = 10.0;
  uint32_t run = 0;
  bool flow_monitor = true;
  bool sack = true;
  bool enable_udp_burst = false;
  std::string queue_disc_type = "ns3::RedQueueDisc";
  std::string recovery = "ns3::TcpClassicRecovery";

  CommandLine cmd;
  cmd.AddValue("simSeed", "Run number for random streams. Default: 0", run);
  cmd.AddValue("nLeaf", "Number of left and right side leaf nodes", nLeaf);
  cmd.AddValue("transport_prot",
               "Transport protocol to use: TcpSwift, TcpNewReno, TcpCubic or "
               "TcpBbr",
               transport_prot);
  cmd.AddValue("error_p", "Packet error rate", error_p);
  cmd.AddValue("bottleneck_bandwidth", "Bottleneck bandwidth",
               bottleneck_bandwidth);
  cmd.AddValue("bottleneck_delay", "Bottleneck delay", bottleneck_delay);
  cmd.AddValue("access_bandwidth", "Access link bandwidth", access_bandwidth);
  cmd.AddValue("access_delay", "Access link delay", access_delay);
  cmd.AddValue("prefix_name", "Prefix of output trace file", prefix_file_name);
  cmd.AddValue("data", "Number of Megabytes of data to transmit", data_mbytes);
  cmd.AddValue("mtu", "Size of IP packets to send in bytes", mtu_bytes);
  cmd.AddValue("duration", "Time to allow flows to run in seconds", duration);
  cmd.AddValue("flow_monitor", "Enable flow monitor", flow_monitor);
  cmd.AddValue("queue_disc_type",
               "Queue disc type for gateway (ns3::RedQueueDisc, "
               "ns3::PfifoFastQueueDisc or ns3::CoDelQueueDisc)",
               queue_disc_type);
  cmd.AddValue("sack", "Enable or disable SACK option", sack);
  cmd.AddValue("enable_udp_burst", "Enable or disable UDP burst traffic",
               enable_udp_burst);
  cmd.AddValue("recovery",
               "Recovery algorithm type to use (e.g., ns3::TcpPrrRecovery)",
               recovery);
  cmd.Parse(argc, argv);

  transport_prot = std::string("ns3::") + transport_prot;

  SeedManager::SetSeed(1);
  SeedManager::SetRun(run);

  NS_LOG_UNCOND("Simulation parameters:");
  NS_LOG_UNCOND("--run: " << run);
  NS_LOG_UNCOND("--Tcp version: " << transport_prot);
  NS_LOG_UNCOND("AccessBW: " << access_bandwidth);
  NS_LOG_UNCOND("BottleneckBW: " << bottleneck_bandwidth);

  Header *temp_header = new Ipv4Header();
  uint32_t ip_header = temp_header->GetSerializedSize();
  NS_LOG_LOGIC("IP Header size is: " << ip_header);
  delete temp_header;
  temp_header = new TcpHeader();
  uint32_t tcp_header = temp_header->GetSerializedSize();
  NS_LOG_LOGIC("TCP Header size is: " << tcp_header);
  delete temp_header;
  uint32_t tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);
  NS_LOG_LOGIC("TCP ADU size is: " << tcp_adu_size);

  double start_time = 0.1;
  double traffic_stop_time = start_time + duration;
  double simulation_stop_time = traffic_stop_time + 1.0;

  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(1 << 24));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(1 << 24));
  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(sack));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2));

  Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                     TypeIdValue(TypeId::LookupByName(recovery)));
  TypeId tcpTid;
  if (!TypeId::LookupByNameFailSafe(transport_prot, &tcpTid)) {
    std::cerr << "Unsupported TCP protocol: " << transport_prot << std::endl;
    return 2;
  }
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(tcpTid));

  Config::SetDefault("ns3::TcpSocketBase::UseEcn",
                     EnumValue(TcpSocketState::On));

  Ptr<RateErrorModel> errorModel;
  if (error_p > 0.0) {
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    uv->SetStream(50);
    errorModel = CreateObject<RateErrorModel>();
    errorModel->SetRandomVariable(uv);
    errorModel->SetUnit(RateErrorModel::ERROR_UNIT_PACKET);
    errorModel->SetRate(error_p);
  }

  PointToPointHelper bottleNeckLink;
  bottleNeckLink.SetDeviceAttribute("DataRate",
                                    StringValue(bottleneck_bandwidth));
  bottleNeckLink.SetChannelAttribute("Delay", StringValue(bottleneck_delay));
  if (errorModel) {
    bottleNeckLink.SetDeviceAttribute("ReceiveErrorModel",
                                      PointerValue(errorModel));
  }

  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute("DataRate",
                                      StringValue(access_bandwidth));
  pointToPointLeaf.SetChannelAttribute("Delay", StringValue(access_delay));

  PointToPointDumbbellHelper d(nLeaf, pointToPointLeaf, nLeaf, pointToPointLeaf,
                               bottleNeckLink);

  InternetStackHelper stack;
  stack.InstallAll();

  TrafficControlHelper tchPfifo;
  tchPfifo.SetRootQueueDisc("ns3::PfifoFastQueueDisc");

  TrafficControlHelper tchCoDel;
  tchCoDel.SetRootQueueDisc("ns3::CoDelQueueDisc");

  TrafficControlHelper tchRed;
  tchRed.SetRootQueueDisc("ns3::RedQueueDisc");

  DataRate access_b(access_bandwidth);
  DataRate bottle_b(bottleneck_bandwidth);
  Time access_d(access_delay);
  Time bottle_d(bottleneck_delay);

  double fullRtt = ((access_d + bottle_d + access_d) * 2).GetSeconds();
  uint32_t bdp_bytes = static_cast<uint32_t>(
      static_cast<double>(bottle_b.GetBitRate()) / 8.0 * fullRtt);
  uint32_t queue_packets =
      std::max(bdp_bytes * nLeaf / mtu_bytes, static_cast<uint32_t>(100));
  uint32_t queue_bytes = queue_packets * mtu_bytes;

  NS_LOG_UNCOND("--Queue size: " << queue_packets << " packets (" << queue_bytes
                                 << " bytes), BDP=" << bdp_bytes << " bytes");

  Config::SetDefault(
      "ns3::PfifoFastQueueDisc::MaxSize",
      QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queue_packets)));
  Config::SetDefault(
      "ns3::CoDelQueueDisc::MaxSize",
      QueueSizeValue(QueueSize(QueueSizeUnit::BYTES, queue_bytes)));

  Config::SetDefault(
      "ns3::RedQueueDisc::MaxSize",
      QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, queue_packets)));
  Config::SetDefault("ns3::RedQueueDisc::MinTh",
                     DoubleValue(0.3 * queue_packets));
  Config::SetDefault("ns3::RedQueueDisc::MaxTh",
                     DoubleValue(0.9 * queue_packets));
  Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(true));
  Config::SetDefault("ns3::RedQueueDisc::MeanPktSize",
                     UintegerValue(mtu_bytes));
  Config::SetDefault("ns3::RedQueueDisc::LinkBandwidth",
                     DataRateValue(bottle_b));
  Config::SetDefault("ns3::RedQueueDisc::LinkDelay", TimeValue(bottle_d));

  if (queue_disc_type.compare("ns3::PfifoFastQueueDisc") == 0) {
    tchPfifo.Install(d.GetLeft()->GetDevice(1));
    tchPfifo.Install(d.GetRight()->GetDevice(1));
  } else if (queue_disc_type.compare("ns3::CoDelQueueDisc") == 0) {
    tchCoDel.Install(d.GetLeft()->GetDevice(1));
    tchCoDel.Install(d.GetRight()->GetDevice(1));
  } else if (queue_disc_type.compare("ns3::RedQueueDisc") == 0) {
    tchRed.Install(d.GetLeft()->GetDevice(1));
    tchRed.Install(d.GetRight()->GetDevice(1));
  } else {
    NS_FATAL_ERROR(
        "Queue not recognized. Allowed values are ns3::RedQueueDisc, "
        "ns3::CoDelQueueDisc or ns3::PfifoFastQueueDisc");
  }

  d.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                        Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                        Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

  NS_LOG_INFO("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Address sinkLocalAddress(
      InetSocketAddress(Ipv4Address::GetAny(), tcpTrafficPort));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApps;
  for (uint32_t i = 0; i < d.RightCount(); ++i) {
    sinkHelper.SetAttribute("Protocol",
                            TypeIdValue(TcpSocketFactory::GetTypeId()));
    sinkApps.Add(sinkHelper.Install(d.GetRight(i)));
  }
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(simulation_stop_time));
  Ptr<PacketSink> sink = StaticCast<PacketSink>(sinkApps.Get(0));

  for (uint32_t i = 0; i < d.LeftCount(); ++i) {
    AddressValue remoteAddress(
        InetSocketAddress(d.GetRightIpv4Address(i), tcpTrafficPort));
    Config::SetDefault("ns3::TcpSocket::SegmentSize",
                       UintegerValue(tcp_adu_size));
    BulkSendHelper ftp("ns3::TcpSocketFactory", Address());
    ftp.SetAttribute("Remote", remoteAddress);
    ftp.SetAttribute("SendSize", UintegerValue(tcp_adu_size));
    ftp.SetAttribute("MaxBytes", UintegerValue(data_mbytes * 1000000));

    ApplicationContainer clientApp = ftp.Install(d.GetLeft(i));
    clientApp.Start(Seconds(start_time * (i + 1)));
    clientApp.Stop(Seconds(traffic_stop_time));
  }

  if (enable_udp_burst) {
    Address udpSinkLocalAddress(
        InetSocketAddress(Ipv4Address::GetAny(), udpTrafficPort));
    PacketSinkHelper udpSinkHelper("ns3::UdpSocketFactory",
                                   udpSinkLocalAddress);
    ApplicationContainer udpSinkApp = udpSinkHelper.Install(d.GetRight(0));
    udpSinkApp.Start(Seconds(0.0));
    udpSinkApp.Stop(Seconds(simulation_stop_time));
    OnOffHelper udpBurstHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(d.GetRightIpv4Address(0), udpTrafficPort));
    udpBurstHelper.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
    udpBurstHelper.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
    constexpr double udpDutyCycle = 0.5;
    constexpr double udpAverageLoadFraction = 0.32;
    DataRate udpPeakRate(static_cast<uint64_t>(
        bottle_b.GetBitRate() * udpAverageLoadFraction / udpDutyCycle));
    udpBurstHelper.SetAttribute("DataRate", DataRateValue(udpPeakRate));
    udpBurstHelper.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer udpBurstApp = udpBurstHelper.Install(d.GetLeft(0));
    udpBurstApp.Start(Seconds(0.5));
    udpBurstApp.Stop(Seconds(traffic_stop_time));
  }

  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor;
  if (flow_monitor) {
    monitor = flowHelper.InstallAll();
  }

  for (uint32_t i = 0; i < d.RightCount(); ++i) {
    rxPkts.push_back(0);
    Ptr<PacketSink> pktSink = DynamicCast<PacketSink>(sinkApps.Get(i));
    pktSink->TraceConnectWithoutContext("Rx",
                                        MakeBoundCallback(&CountRxPkts, i));
  }

  Simulator::Stop(Seconds(simulation_stop_time));
  Simulator::Run();

  if (flow_monitor) {
    monitor->CheckForLostPackets();
    flowHelper.SerializeToXmlFile(prefix_file_name + ".flowmonitor", true,
                                  true);

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    double aggThroughput = 0.0;
    uint64_t aggTxPackets = 0;
    uint64_t aggLostPackets = 0;

    for (const auto &flow : stats) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);
      NS_LOG_UNCOND("Flow " << flow.first << " Src Addr: " << t.sourceAddress
                            << " Dst Addr: " << t.destinationAddress);
      NS_LOG_UNCOND(
          "Time Last Rx Packet: " << flow.second.timeLastRxPacket.GetSeconds());
      NS_LOG_UNCOND("Time First Tx Packet: "
                    << flow.second.timeFirstTxPacket.GetSeconds());
      NS_LOG_UNCOND("Tx Packets Count: " << flow.second.txPackets);
      NS_LOG_UNCOND("Rx Packets Count: " << flow.second.rxPackets);

      double lossRate = 0.0;
      if (flow.second.txPackets > 0) {
        lossRate =
            (flow.second.lostPackets / (double)flow.second.txPackets) * 100;
      }
      NS_LOG_UNCOND("Loss Rate: " << lossRate << "%");

      double throughput = 0.0;
      double rxTime = flow.second.timeLastRxPacket.GetSeconds() -
                      flow.second.timeFirstTxPacket.GetSeconds();
      if (rxTime > 0) {
        throughput = flow.second.rxBytes * 8.0 / rxTime / 1e6;
      }
      NS_LOG_UNCOND("Throughput: " << throughput << " Mbps");

      bool isForwardTcp = t.protocol == 6 &&
                          (t.sourceAddress.Get() & 0xffff0000U) ==
                              Ipv4Address("10.1.0.0").Get() &&
                          (t.destinationAddress.Get() & 0xffff0000U) ==
                              Ipv4Address("10.2.0.0").Get();
      if (isForwardTcp) {
        aggThroughput += throughput;
        aggTxPackets += flow.second.txPackets;
        aggLostPackets += flow.second.lostPackets;
      }
    }

    double aggLossRate = 0.0;
    if (aggTxPackets > 0) {
      aggLossRate = (aggLostPackets / (double)aggTxPackets) * 100;
    }
    NS_LOG_UNCOND("AggregateThroughput: " << aggThroughput << " Mbps");
    NS_LOG_UNCOND("AggregateLossRate: " << aggLossRate << " %");
  }

  PrintRxCount();
  NS_LOG_UNCOND("Total Rx Bytes Count: " << sink->GetTotalRx());

  monitor = nullptr;
  sink = nullptr;

  Simulator::Destroy();
  return 0;
}
