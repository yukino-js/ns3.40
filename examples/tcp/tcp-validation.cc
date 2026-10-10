

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpValidation");

uint32_t g_firstBytesReceived = 0;
uint32_t g_secondBytesReceived = 0;
uint32_t g_marksObserved = 0;
uint32_t g_dropsObserved = 0;
std::string g_validate = "";
bool g_validationFailed = false;

void TraceFirstCwnd(std::ofstream *ofStream, uint32_t oldCwnd,
                    uint32_t newCwnd) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " "
              << static_cast<double>(newCwnd) / 1448 << std::endl;
  }
  if (g_validate == "cubic-50ms-no-ecn" || g_validate == "cubic-50ms-ecn") {
    double now = Simulator::Now().GetSeconds();
    double cwnd = static_cast<double>(newCwnd) / 1448;
    if ((now > 5.43) && (now < 5.465) && (cwnd < 500)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " cwnd " << cwnd
                         << " (expected >= 500)");
      g_validationFailed = true;
    } else if ((now > 5.795) && (now < 6) && (cwnd > 190)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " cwnd " << cwnd
                         << " (expected <= 190)");
      g_validationFailed = true;
    } else if ((now > 14) && (now < 14.197) && (cwnd < 224)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " cwnd " << cwnd
                         << " (expected >= 224)");
      g_validationFailed = true;
    } else if ((now > 17) && (now < 18.026) && (cwnd < 212)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " cwnd " << cwnd
                         << " (expected >= 212)");
      g_validationFailed = true;
    }
  }
}

void TraceFirstDctcp(std::ofstream *ofStream, uint32_t bytesMarked,
                     uint32_t bytesAcked, double alpha) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << alpha << std::endl;
  }
  if (g_validate == "dctcp-80ms") {
    double now = Simulator::Now().GetSeconds();
    if ((now < 7.5) && (alpha < 0.1)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " alpha " << alpha
                         << " (expected >= 0.1)");
      g_validationFailed = true;
    } else if ((now > 11) && (now < 30) && (alpha > 0.01)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " alpha " << alpha
                         << " (expected <= 0.01)");
      g_validationFailed = true;
    } else if ((now > 34) && (alpha < 0.015) && (alpha > 0.025)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " alpha " << alpha
                         << " (expected 0.015 <= alpha <= 0.025)");
      g_validationFailed = true;
    }
  } else if (g_validate == "dctcp-10ms") {
    double now = Simulator::Now().GetSeconds();
    if ((now > 5.6) && (alpha > 0.1)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " alpha " << alpha
                         << " (expected <= 0.1)");
      g_validationFailed = true;
    }
    if ((now > 7) && ((alpha > 0.09) || (alpha < 0.055))) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " alpha " << alpha
                         << " (expected 0.09 <= alpha <= 0.055)");
      g_validationFailed = true;
    }
  }
}

void TraceFirstRtt(std::ofstream *ofStream, Time oldRtt, Time newRtt) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " "
              << newRtt.GetSeconds() * 1000 << std::endl;
  }
}

void TraceSecondCwnd(std::ofstream *ofStream, uint32_t oldCwnd,
                     uint32_t newCwnd) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " "
              << static_cast<double>(newCwnd) / 1448 << std::endl;
  }
}

void TraceSecondRtt(std::ofstream *ofStream, Time oldRtt, Time newRtt) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " "
              << newRtt.GetSeconds() * 1000 << std::endl;
  }
}

void TraceSecondDctcp(std::ofstream *ofStream, uint32_t bytesMarked,
                      uint32_t bytesAcked, double alpha) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << alpha << std::endl;
  }
}

void TracePingRtt(std::ofstream *ofStream, uint16_t, Time rtt) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << rtt.GetSeconds() * 1000
              << std::endl;
  }
}

void TraceFirstRx(Ptr<const Packet> packet, const Address &address) {
  g_firstBytesReceived += packet->GetSize();
}

void TraceSecondRx(Ptr<const Packet> packet, const Address &address) {
  g_secondBytesReceived += packet->GetSize();
}

void TraceQueueDrop(std::ofstream *ofStream, Ptr<const QueueDiscItem> item) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << std::hex
              << item->Hash() << std::endl;
  }
  g_dropsObserved++;
}

void TraceQueueMark(std::ofstream *ofStream, Ptr<const QueueDiscItem> item,
                    const char *reason) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << std::hex
              << item->Hash() << std::endl;
  }
  g_marksObserved++;
}

void TraceQueueLength(std::ofstream *ofStream, DataRate queueLinkRate,
                      uint32_t oldVal, uint32_t newVal) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << std::fixed
              << static_cast<double>(newVal * 8) /
                     (queueLinkRate.GetBitRate() / 1000)
              << std::endl;
  }
}

void TraceMarksFrequency(std::ofstream *ofStream, Time marksSamplingInterval) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << g_marksObserved
              << std::endl;
  }
  g_marksObserved = 0;
  Simulator::Schedule(marksSamplingInterval, &TraceMarksFrequency, ofStream,
                      marksSamplingInterval);
}

void TraceFirstThroughput(std::ofstream *ofStream, Time throughputInterval) {
  double throughput =
      g_firstBytesReceived * 8 / throughputInterval.GetSeconds() / 1e6;
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " " << throughput
              << std::endl;
  }
  g_firstBytesReceived = 0;
  Simulator::Schedule(throughputInterval, &TraceFirstThroughput, ofStream,
                      throughputInterval);
  if (g_validate == "dctcp-80ms") {
    double now = Simulator::Now().GetSeconds();
    if ((now < 14) && (throughput > 20)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " throughput " << throughput
                         << " (expected <= 20)");
      g_validationFailed = true;
    }
    if ((now < 30) && (throughput > 48)) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " throughput " << throughput
                         << " (expected <= 48)");
      g_validationFailed = true;
    }
    if ((now > 32) && ((throughput < 47.5) || (throughput > 48.5))) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " throughput " << throughput
                         << " (expected 47.5 <= throughput <= 48.5)");
      g_validationFailed = true;
    }
  } else if (g_validate == "dctcp-10ms") {
    double now = Simulator::Now().GetSeconds();
    if ((now > 5.6) && ((throughput < 48) || (throughput > 49))) {
      NS_LOG_WARN("now " << Now().As(Time::S) << " throughput " << throughput
                         << " (expected 48 <= throughput <= 49)");
      g_validationFailed = true;
    }
  }
}

void TraceSecondThroughput(std::ofstream *ofStream, Time throughputInterval) {
  if (g_validate.empty()) {
    *ofStream << Simulator::Now().GetSeconds() << " "
              << g_secondBytesReceived * 8 / throughputInterval.GetSeconds() /
                     1e6
              << std::endl;
  }
  g_secondBytesReceived = 0;
  Simulator::Schedule(throughputInterval, &TraceSecondThroughput, ofStream,
                      throughputInterval);
}

void ScheduleFirstTcpCwndTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContextFailSafe(
      "/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
      MakeBoundCallback(&TraceFirstCwnd, ofStream));
}

void ScheduleFirstTcpRttTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContextFailSafe(
      "/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/RTT",
      MakeBoundCallback(&TraceFirstRtt, ofStream));
}

void ScheduleFirstDctcpTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContextFailSafe(
      "/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/"
      "CongestionOps/$ns3::TcpDctcp/CongestionEstimate",
      MakeBoundCallback(&TraceFirstDctcp, ofStream));
}

void ScheduleSecondDctcpTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContextFailSafe(
      "/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/"
      "CongestionOps/$ns3::TcpDctcp/CongestionEstimate",
      MakeBoundCallback(&TraceSecondDctcp, ofStream));
}

void ScheduleFirstPacketSinkConnection() {
  Config::ConnectWithoutContextFailSafe(
      "/NodeList/6/ApplicationList/*/$ns3::PacketSink/Rx",
      MakeCallback(&TraceFirstRx));
}

void ScheduleSecondTcpCwndTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContext(
      "/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
      MakeBoundCallback(&TraceSecondCwnd, ofStream));
}

void ScheduleSecondTcpRttTraceConnection(std::ofstream *ofStream) {
  Config::ConnectWithoutContext(
      "/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTT",
      MakeBoundCallback(&TraceSecondRtt, ofStream));
}

void ScheduleSecondPacketSinkConnection() {
  Config::ConnectWithoutContext(
      "/NodeList/7/ApplicationList/*/$ns3::PacketSink/Rx",
      MakeCallback(&TraceSecondRx));
}

int main(int argc, char *argv[]) {
  uint32_t pingSize = 100;
  bool enableSecondTcp = false;
  bool enableLogging = false;
  Time pingInterval = MilliSeconds(100);
  Time marksSamplingInterval = MilliSeconds(100);
  Time throughputSamplingInterval = MilliSeconds(200);
  std::string pingTraceFile = "tcp-validation-ping.dat";
  std::string firstTcpRttTraceFile = "tcp-validation-first-tcp-rtt.dat";
  std::string firstTcpCwndTraceFile = "tcp-validation-first-tcp-cwnd.dat";
  std::string firstDctcpTraceFile = "tcp-validation-first-dctcp-alpha.dat";
  std::string firstTcpThroughputTraceFile =
      "tcp-validation-first-tcp-throughput.dat";
  std::string secondTcpRttTraceFile = "tcp-validation-second-tcp-rtt.dat";
  std::string secondTcpCwndTraceFile = "tcp-validation-second-tcp-cwnd.dat";
  std::string secondTcpThroughputTraceFile =
      "tcp-validation-second-tcp-throughput.dat";
  std::string secondDctcpTraceFile = "tcp-validation-second-dctcp-alpha.dat";
  std::string queueMarkTraceFile = "tcp-validation-queue-mark.dat";
  std::string queueDropTraceFile = "tcp-validation-queue-drop.dat";
  std::string queueMarksFrequencyTraceFile =
      "tcp-validation-queue-marks-frequency.dat";
  std::string queueLengthTraceFile = "tcp-validation-queue-length.dat";

  std::string firstTcpType = "cubic";
  std::string secondTcpType = "";
  std::string queueType = "codel";
  Time stopTime = Seconds(70);
  Time baseRtt = MilliSeconds(80);
  DataRate linkRate("50Mbps");
  bool queueUseEcn = false;
  Time ceThreshold = MilliSeconds(1);
  bool enablePcap = false;

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(32768000));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(32768000));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(10));
  Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                     TypeIdValue(TcpPrrRecovery::GetTypeId()));

  CommandLine cmd(__FILE__);
  cmd.AddValue("firstTcpType", "first TCP type (cubic, dctcp, or reno)",
               firstTcpType);
  cmd.AddValue("secondTcpType", "second TCP type (cubic, dctcp, or reno)",
               secondTcpType);
  cmd.AddValue("queueType", "bottleneck queue type (fq, codel, pie, or red)",
               queueType);
  cmd.AddValue("baseRtt", "base RTT", baseRtt);
  cmd.AddValue("ceThreshold", "CoDel CE threshold (for DCTCP)", ceThreshold);
  cmd.AddValue("linkRate", "data rate of bottleneck link", linkRate);
  cmd.AddValue("stopTime", "simulation stop time", stopTime);
  cmd.AddValue("queueUseEcn", "use ECN on queue", queueUseEcn);
  cmd.AddValue("enablePcap", "enable Pcap", enablePcap);
  cmd.AddValue("validate", "validation case to run", g_validate);
  cmd.Parse(argc, argv);

  if (!g_validate.empty()) {
    NS_ABORT_MSG_UNLESS(
        g_validate == "dctcp-10ms" || g_validate == "dctcp-80ms" ||
            g_validate == "cubic-50ms-no-ecn" || g_validate == "cubic-50ms-ecn",
        "Unknown test");
    if (g_validate == "dctcp-10ms" || g_validate == "dctcp-80ms") {
      NS_ABORT_MSG_UNLESS(firstTcpType == "dctcp", "Incorrect TCP");
      NS_ABORT_MSG_UNLESS(secondTcpType.empty(), "Incorrect TCP");
      NS_ABORT_MSG_UNLESS(linkRate == DataRate("50Mbps"),
                          "Incorrect data rate");
      NS_ABORT_MSG_UNLESS(queueUseEcn == true, "Incorrect ECN configuration");
      NS_ABORT_MSG_UNLESS(stopTime >= Seconds(15), "Incorrect stopTime");
      if (g_validate == "dctcp-10ms") {
        NS_ABORT_MSG_UNLESS(baseRtt == MilliSeconds(10), "Incorrect RTT");
      } else if (g_validate == "dctcp-80ms") {
        NS_ABORT_MSG_UNLESS(baseRtt == MilliSeconds(80), "Incorrect RTT");
      }
    } else if (g_validate == "cubic-50ms-no-ecn" ||
               g_validate == "cubic-50ms-ecn") {
      NS_ABORT_MSG_UNLESS(firstTcpType == "cubic", "Incorrect TCP");
      NS_ABORT_MSG_UNLESS(secondTcpType.empty(), "Incorrect TCP");
      NS_ABORT_MSG_UNLESS(baseRtt == MilliSeconds(50), "Incorrect RTT");
      NS_ABORT_MSG_UNLESS(linkRate == DataRate("50Mbps"),
                          "Incorrect data rate");
      NS_ABORT_MSG_UNLESS(stopTime >= Seconds(20), "Incorrect stopTime");
      if (g_validate == "cubic-50ms-no-ecn") {
        NS_ABORT_MSG_UNLESS(queueUseEcn == false,
                            "Incorrect ECN configuration");
      } else if (g_validate == "cubic-50ms-ecn") {
        NS_ABORT_MSG_UNLESS(queueUseEcn == true, "Incorrect ECN configuration");
      }
    }
  }

  if (enableLogging) {
    LogComponentEnable("TcpSocketBase",
                       (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_NODE |
                                  LOG_PREFIX_TIME | LOG_LEVEL_ALL));
    LogComponentEnable("TcpDctcp",
                       (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_NODE |
                                  LOG_PREFIX_TIME | LOG_LEVEL_ALL));
  }

  Time oneWayDelay = baseRtt / 2;

  TypeId firstTcpTypeId;
  if (firstTcpType == "reno") {
    firstTcpTypeId = TcpLinuxReno::GetTypeId();
  } else if (firstTcpType == "cubic") {
    firstTcpTypeId = TcpCubic::GetTypeId();
  } else if (firstTcpType == "dctcp") {
    firstTcpTypeId = TcpDctcp::GetTypeId();
    Config::SetDefault("ns3::CoDelQueueDisc::CeThreshold",
                       TimeValue(ceThreshold));
    Config::SetDefault("ns3::FqCoDelQueueDisc::CeThreshold",
                       TimeValue(ceThreshold));
    if (!queueUseEcn) {
      std::cout << "Warning: using DCTCP with queue ECN disabled" << std::endl;
    }
  } else {
    NS_FATAL_ERROR("Fatal error:  tcp unsupported");
  }
  TypeId secondTcpTypeId;
  if (secondTcpType == "reno") {
    enableSecondTcp = true;
    secondTcpTypeId = TcpLinuxReno::GetTypeId();
  } else if (secondTcpType == "cubic") {
    enableSecondTcp = true;
    secondTcpTypeId = TcpCubic::GetTypeId();
  } else if (secondTcpType == "dctcp") {
    enableSecondTcp = true;
    secondTcpTypeId = TcpDctcp::GetTypeId();
  } else if (secondTcpType.empty()) {
    enableSecondTcp = false;
    NS_LOG_DEBUG("No second TCP selected");
  } else {
    NS_FATAL_ERROR("Fatal error:  tcp unsupported");
  }
  TypeId queueTypeId;
  if (queueType == "fq") {
    queueTypeId = FqCoDelQueueDisc::GetTypeId();
  } else if (queueType == "codel") {
    queueTypeId = CoDelQueueDisc::GetTypeId();
  } else if (queueType == "pie") {
    queueTypeId = PieQueueDisc::GetTypeId();
  } else if (queueType == "red") {
    queueTypeId = RedQueueDisc::GetTypeId();
  } else {
    NS_FATAL_ERROR("Fatal error:  queueType unsupported");
  }

  if (queueUseEcn) {
    Config::SetDefault("ns3::CoDelQueueDisc::UseEcn", BooleanValue(true));
    Config::SetDefault("ns3::FqCoDelQueueDisc::UseEcn", BooleanValue(true));
    Config::SetDefault("ns3::PieQueueDisc::UseEcn", BooleanValue(true));
    Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(true));
  }
  Config::SetDefault("ns3::TcpSocketBase::UseEcn", StringValue("On"));

  if (enableSecondTcp) {
    NS_LOG_DEBUG("first TCP: " << firstTcpTypeId.GetName()
                               << "; second TCP: " << secondTcpTypeId.GetName()
                               << "; queue: " << queueTypeId.GetName()
                               << "; ceThreshold: "
                               << ceThreshold.GetSeconds() * 1000 << "ms");
  } else {
    NS_LOG_DEBUG("first TCP: " << firstTcpTypeId.GetName() << "; queue: "
                               << queueTypeId.GetName() << "; ceThreshold: "
                               << ceThreshold.GetSeconds() * 1000 << "ms");
  }

  std::ofstream pingOfStream;
  std::ofstream firstTcpRttOfStream;
  std::ofstream firstTcpCwndOfStream;
  std::ofstream firstTcpThroughputOfStream;
  std::ofstream firstTcpDctcpOfStream;
  std::ofstream secondTcpRttOfStream;
  std::ofstream secondTcpCwndOfStream;
  std::ofstream secondTcpThroughputOfStream;
  std::ofstream secondTcpDctcpOfStream;
  std::ofstream queueDropOfStream;
  std::ofstream queueMarkOfStream;
  std::ofstream queueMarksFrequencyOfStream;
  std::ofstream queueLengthOfStream;
  if (g_validate.empty()) {
    pingOfStream.open(pingTraceFile, std::ofstream::out);
    firstTcpRttOfStream.open(firstTcpRttTraceFile, std::ofstream::out);
    firstTcpCwndOfStream.open(firstTcpCwndTraceFile, std::ofstream::out);
    firstTcpThroughputOfStream.open(firstTcpThroughputTraceFile,
                                    std::ofstream::out);
    if (firstTcpType == "dctcp") {
      firstTcpDctcpOfStream.open(firstDctcpTraceFile, std::ofstream::out);
    }
    if (enableSecondTcp) {
      secondTcpRttOfStream.open(secondTcpRttTraceFile, std::ofstream::out);
      secondTcpCwndOfStream.open(secondTcpCwndTraceFile, std::ofstream::out);
      secondTcpThroughputOfStream.open(secondTcpThroughputTraceFile,
                                       std::ofstream::out);
      if (secondTcpType == "dctcp") {
        secondTcpDctcpOfStream.open(secondDctcpTraceFile, std::ofstream::out);
      }
    }
    queueDropOfStream.open(queueDropTraceFile, std::ofstream::out);
    queueMarkOfStream.open(queueMarkTraceFile, std::ofstream::out);
    queueMarksFrequencyOfStream.open(queueMarksFrequencyTraceFile,
                                     std::ofstream::out);
    queueLengthOfStream.open(queueLengthTraceFile, std::ofstream::out);
  }

  Ptr<Node> pingServer = CreateObject<Node>();
  Ptr<Node> firstServer = CreateObject<Node>();
  Ptr<Node> secondServer = CreateObject<Node>();
  Ptr<Node> wanRouter = CreateObject<Node>();
  Ptr<Node> lanRouter = CreateObject<Node>();
  Ptr<Node> pingClient = CreateObject<Node>();
  Ptr<Node> firstClient = CreateObject<Node>();
  Ptr<Node> secondClient = CreateObject<Node>();

  NetDeviceContainer pingServerDevices;
  NetDeviceContainer firstServerDevices;
  NetDeviceContainer secondServerDevices;
  NetDeviceContainer wanLanDevices;
  NetDeviceContainer pingClientDevices;
  NetDeviceContainer firstClientDevices;
  NetDeviceContainer secondClientDevices;

  PointToPointHelper p2p;
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize",
               QueueSizeValue(QueueSize("3p")));
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1000Mbps")));
  p2p.SetChannelAttribute("Delay", TimeValue(MicroSeconds(1)));
  pingServerDevices = p2p.Install(wanRouter, pingServer);
  firstServerDevices = p2p.Install(wanRouter, firstServer);
  secondServerDevices = p2p.Install(wanRouter, secondServer);
  p2p.SetChannelAttribute("Delay", TimeValue(oneWayDelay));
  wanLanDevices = p2p.Install(wanRouter, lanRouter);
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize",
               QueueSizeValue(QueueSize("3p")));
  p2p.SetChannelAttribute("Delay", TimeValue(MicroSeconds(1)));
  pingClientDevices = p2p.Install(lanRouter, pingClient);
  firstClientDevices = p2p.Install(lanRouter, firstClient);
  secondClientDevices = p2p.Install(lanRouter, secondClient);

  Ptr<PointToPointNetDevice> p =
      wanLanDevices.Get(0)->GetObject<PointToPointNetDevice>();
  p->SetAttribute("DataRate", DataRateValue(linkRate));

  InternetStackHelper stackHelper;
  stackHelper.Install(pingServer);
  Ptr<TcpL4Protocol> proto;
  stackHelper.Install(firstServer);
  proto = firstServer->GetObject<TcpL4Protocol>();
  proto->SetAttribute("SocketType", TypeIdValue(firstTcpTypeId));
  stackHelper.Install(secondServer);
  stackHelper.Install(wanRouter);
  stackHelper.Install(lanRouter);
  stackHelper.Install(pingClient);

  stackHelper.Install(firstClient);
  proto = firstClient->GetObject<TcpL4Protocol>();
  proto->SetAttribute("SocketType", TypeIdValue(firstTcpTypeId));
  stackHelper.Install(secondClient);

  if (enableSecondTcp) {
    proto = secondClient->GetObject<TcpL4Protocol>();
    proto->SetAttribute("SocketType", TypeIdValue(secondTcpTypeId));
    proto = secondServer->GetObject<TcpL4Protocol>();
    proto->SetAttribute("SocketType", TypeIdValue(secondTcpTypeId));
  }

  TrafficControlHelper tchFq;
  tchFq.SetRootQueueDisc("ns3::FqCoDelQueueDisc");
  tchFq.SetQueueLimits("ns3::DynamicQueueLimits", "HoldTime",
                       StringValue("1ms"));
  tchFq.Install(pingServerDevices);
  tchFq.Install(firstServerDevices);
  tchFq.Install(secondServerDevices);
  tchFq.Install(wanLanDevices.Get(1));
  tchFq.Install(pingClientDevices);
  tchFq.Install(firstClientDevices);
  tchFq.Install(secondClientDevices);
  TrafficControlHelper tchBottleneck;
  tchBottleneck.SetRootQueueDisc(queueTypeId.GetName());
  tchBottleneck.SetQueueLimits("ns3::DynamicQueueLimits", "HoldTime",
                               StringValue("1ms"));
  tchBottleneck.Install(wanLanDevices.Get(0));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer pingServerIfaces = ipv4.Assign(pingServerDevices);
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer firstServerIfaces = ipv4.Assign(firstServerDevices);
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer secondServerIfaces = ipv4.Assign(secondServerDevices);
  ipv4.SetBase("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer wanLanIfaces = ipv4.Assign(wanLanDevices);
  ipv4.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer pingClientIfaces = ipv4.Assign(pingClientDevices);
  ipv4.SetBase("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer firstClientIfaces = ipv4.Assign(firstClientDevices);
  ipv4.SetBase("192.168.3.0", "255.255.255.0");
  Ipv4InterfaceContainer secondClientIfaces = ipv4.Assign(secondClientDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  PingHelper pingHelper(Ipv4Address("192.168.1.2"));
  pingHelper.SetAttribute("Interval", TimeValue(pingInterval));
  pingHelper.SetAttribute("Size", UintegerValue(pingSize));
  pingHelper.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::SILENT));
  ApplicationContainer pingContainer = pingHelper.Install(pingServer);
  Ptr<Ping> ping = pingContainer.Get(0)->GetObject<Ping>();
  ping->TraceConnectWithoutContext(
      "Rtt", MakeBoundCallback(&TracePingRtt, &pingOfStream));
  pingContainer.Start(Seconds(1));
  pingContainer.Stop(stopTime - Seconds(1));

  ApplicationContainer firstApp;
  uint16_t firstPort = 5000;
  BulkSendHelper tcp("ns3::TcpSocketFactory", Address());
  tcp.SetAttribute("MaxBytes", UintegerValue(7500000000));
  InetSocketAddress firstDestAddress(firstClientIfaces.GetAddress(1),
                                     firstPort);
  tcp.SetAttribute("Remote", AddressValue(firstDestAddress));
  firstApp = tcp.Install(firstServer);
  firstApp.Start(Seconds(5));
  firstApp.Stop(stopTime - Seconds(1));

  Address firstSinkAddress(InetSocketAddress(Ipv4Address::GetAny(), firstPort));
  ApplicationContainer firstSinkApp;
  PacketSinkHelper firstSinkHelper("ns3::TcpSocketFactory", firstSinkAddress);
  firstSinkApp = firstSinkHelper.Install(firstClient);
  firstSinkApp.Start(Seconds(5));
  firstSinkApp.Stop(stopTime - MilliSeconds(500));

  if (enableSecondTcp) {
    BulkSendHelper tcp("ns3::TcpSocketFactory", Address());
    uint16_t secondPort = 5000;
    ApplicationContainer secondApp;
    InetSocketAddress secondDestAddress(secondClientIfaces.GetAddress(1),
                                        secondPort);
    tcp.SetAttribute("Remote", AddressValue(secondDestAddress));
    secondApp = tcp.Install(secondServer);
    secondApp.Start(Seconds(15));
    secondApp.Stop(stopTime - Seconds(1));

    Address secondSinkAddress(
        InetSocketAddress(Ipv4Address::GetAny(), secondPort));
    PacketSinkHelper secondSinkHelper("ns3::TcpSocketFactory",
                                      secondSinkAddress);
    ApplicationContainer secondSinkApp;
    secondSinkApp = secondSinkHelper.Install(secondClient);
    secondSinkApp.Start(Seconds(15));
    secondSinkApp.Stop(stopTime - MilliSeconds(500));
  }

  Ptr<TrafficControlLayer> tc;
  Ptr<QueueDisc> qd;
  tc = wanLanDevices.Get(0)->GetNode()->GetObject<TrafficControlLayer>();
  qd = tc->GetRootQueueDiscOnDevice(wanLanDevices.Get(0));
  qd->TraceConnectWithoutContext(
      "Drop", MakeBoundCallback(&TraceQueueDrop, &queueDropOfStream));
  qd->TraceConnectWithoutContext(
      "Mark", MakeBoundCallback(&TraceQueueMark, &queueMarkOfStream));
  qd->TraceConnectWithoutContext(
      "BytesInQueue",
      MakeBoundCallback(&TraceQueueLength, &queueLengthOfStream, linkRate));

  Simulator::Schedule(Seconds(5) + MilliSeconds(100),
                      &ScheduleFirstTcpRttTraceConnection,
                      &firstTcpRttOfStream);
  Simulator::Schedule(Seconds(5) + MilliSeconds(100),
                      &ScheduleFirstTcpCwndTraceConnection,
                      &firstTcpCwndOfStream);
  Simulator::Schedule(Seconds(5) + MilliSeconds(100),
                      &ScheduleFirstPacketSinkConnection);
  if (firstTcpType == "dctcp") {
    Simulator::Schedule(Seconds(5) + MilliSeconds(100),
                        &ScheduleFirstDctcpTraceConnection,
                        &firstTcpDctcpOfStream);
  }
  Simulator::Schedule(throughputSamplingInterval, &TraceFirstThroughput,
                      &firstTcpThroughputOfStream, throughputSamplingInterval);
  if (enableSecondTcp) {
    Simulator::Schedule(Seconds(15) + MilliSeconds(100),
                        &ScheduleSecondTcpRttTraceConnection,
                        &secondTcpRttOfStream);
    Simulator::Schedule(Seconds(15) + MilliSeconds(100),
                        &ScheduleSecondTcpCwndTraceConnection,
                        &secondTcpCwndOfStream);
    Simulator::Schedule(Seconds(15) + MilliSeconds(100),
                        &ScheduleSecondPacketSinkConnection);
    Simulator::Schedule(throughputSamplingInterval, &TraceSecondThroughput,
                        &secondTcpThroughputOfStream,
                        throughputSamplingInterval);
    if (secondTcpType == "dctcp") {
      Simulator::Schedule(Seconds(15) + MilliSeconds(100),
                          &ScheduleSecondDctcpTraceConnection,
                          &secondTcpDctcpOfStream);
    }
  }
  Simulator::Schedule(marksSamplingInterval, &TraceMarksFrequency,
                      &queueMarksFrequencyOfStream, marksSamplingInterval);

  if (enablePcap) {
    p2p.EnablePcapAll("tcp-validation", false);
  }

  Simulator::Stop(stopTime);
  Simulator::Run();
  Simulator::Destroy();

  if (g_validate.empty()) {
    pingOfStream.close();
    firstTcpCwndOfStream.close();
    firstTcpRttOfStream.close();
    if (firstTcpType == "dctcp") {
      firstTcpDctcpOfStream.close();
    }
    firstTcpThroughputOfStream.close();
    if (enableSecondTcp) {
      secondTcpCwndOfStream.close();
      secondTcpRttOfStream.close();
      secondTcpThroughputOfStream.close();
      if (secondTcpType == "dctcp") {
        secondTcpDctcpOfStream.close();
      }
    }
    queueDropOfStream.close();
    queueMarkOfStream.close();
    queueMarksFrequencyOfStream.close();
    queueLengthOfStream.close();
  }

  if (g_validationFailed) {
    NS_FATAL_ERROR("Validation failed");
  }

  return 0;
}
