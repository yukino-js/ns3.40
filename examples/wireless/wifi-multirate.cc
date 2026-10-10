

#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/gnuplot.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/olsr-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/rectangle.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("multirate");

class Experiment {
public:
  Experiment();
  Experiment(std::string name);
  Gnuplot2dDataset Run(const WifiHelper &wifi, const YansWifiPhyHelper &wifiPhy,
                       const WifiMacHelper &wifiMac,
                       const YansWifiChannelHelper &wifiChannel,
                       const MobilityHelper &mobility);

  bool CommandSetup(int argc, char **argv);

  bool IsRouting() const { return m_enableRouting; }

  bool IsMobility() const { return m_enableMobility; }

  uint32_t GetScenario() const { return m_scenario; }

  std::string GetRtsThreshold() const { return m_rtsThreshold; }

  std::string GetOutputFileName() const { return m_outputFileName; }

  std::string GetRateManager() const { return m_rateManager; }

private:
  Ptr<Socket> SetupPacketReceive(Ptr<Node> node);
  NodeContainer GenerateNeighbors(NodeContainer c, uint32_t senderId);

  void ApplicationSetup(Ptr<Node> client, Ptr<Node> server, double start,
                        double stop);
  void AssignNeighbors(NodeContainer c);
  void SelectSrcDest(NodeContainer c);
  void ReceivePacket(Ptr<Socket> socket);
  void CheckThroughput();
  void SendMultiDestinations(Ptr<Node> sender, NodeContainer c);

  Gnuplot2dDataset m_output;

  double m_totalTime;
  double m_expMean;
  double m_samplingPeriod;

  uint32_t m_bytesTotal;
  uint32_t m_packetSize;
  uint32_t m_gridSize;
  uint32_t m_nodeDistance;
  uint32_t m_port;
  uint32_t m_scenario;

  bool m_enablePcap;
  bool m_enableTracing;
  bool m_enableFlowMon;
  bool m_enableRouting;
  bool m_enableMobility;

  NodeContainer m_containerA;
  NodeContainer m_containerB;
  NodeContainer m_containerC;
  NodeContainer m_containerD;
  std::string m_rtsThreshold;
  std::string m_rateManager;
  std::string m_outputFileName;
};

Experiment::Experiment() {}

Experiment::Experiment(std::string name)
    : m_output(name), m_totalTime(0.3), m_expMean(0.1), m_samplingPeriod(0.1),
      m_bytesTotal(0), m_packetSize(2000), m_gridSize(10), m_nodeDistance(30),
      m_port(5000), m_scenario(4), m_enablePcap(false), m_enableTracing(true),
      m_enableFlowMon(false), m_enableRouting(false), m_enableMobility(false),
      m_rtsThreshold("2200"), m_rateManager("ns3::MinstrelWifiManager"),
      m_outputFileName("minstrel") {
  m_output.SetStyle(Gnuplot2dDataset::LINES);
}

Ptr<Socket> Experiment::SetupPacketReceive(Ptr<Node> node) {
  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket(node, tid);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
  sink->Bind(local);
  sink->SetRecvCallback(MakeCallback(&Experiment::ReceivePacket, this));

  return sink;
}

void Experiment::ReceivePacket(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  while ((packet = socket->Recv())) {
    m_bytesTotal += packet->GetSize();
  }
}

void Experiment::CheckThroughput() {
  double mbs = ((m_bytesTotal * 8.0) / 1000000 / m_samplingPeriod);
  m_bytesTotal = 0;
  m_output.Add((Simulator::Now()).GetSeconds(), mbs);

  Simulator::Schedule(Seconds(m_samplingPeriod), &Experiment::CheckThroughput,
                      this);
}

void Experiment::AssignNeighbors(NodeContainer c) {
  uint32_t totalNodes = c.GetN();
  for (uint32_t i = 0; i < totalNodes; i++) {
    if ((i % m_gridSize) <= (m_gridSize / 2 - 1)) {
      if (i < totalNodes / 2) {
        m_containerA.Add(c.Get(i));
      }

      if (i >= (uint32_t)(4 * totalNodes) / 10) {
        m_containerC.Add(c.Get(i));
      }
    }
    if ((i % m_gridSize) >= (m_gridSize / 2 - 1)) {
      if (i < totalNodes / 2) {
        m_containerB.Add(c.Get(i));
      }

      if (i >= (uint32_t)(4 * totalNodes) / 10) {
        m_containerD.Add(c.Get(i));
      }
    }
  }
}

NodeContainer Experiment::GenerateNeighbors(NodeContainer c,
                                            uint32_t senderId) {
  NodeContainer nc;
  uint32_t limit = senderId + 2;
  for (uint32_t i = senderId - 2; i <= limit; i++) {
    nc.Add(c.Get(i));
    nc.Add(c.Get(i + 10));
    nc.Add(c.Get(i + 20));
    nc.Add(c.Get(i - 10));
    nc.Add(c.Get(i - 20));
  }
  return nc;
}

void Experiment::SelectSrcDest(NodeContainer c) {
  uint32_t totalNodes = c.GetN();
  Ptr<UniformRandomVariable> uvSrc = CreateObject<UniformRandomVariable>();
  uvSrc->SetAttribute("Min", DoubleValue(0));
  uvSrc->SetAttribute("Max", DoubleValue(totalNodes / 2 - 1));
  Ptr<UniformRandomVariable> uvDest = CreateObject<UniformRandomVariable>();
  uvDest->SetAttribute("Min", DoubleValue(totalNodes / 2));
  uvDest->SetAttribute("Max", DoubleValue(totalNodes));

  for (uint32_t i = 0; i < totalNodes / 3; i++) {
    ApplicationSetup(c.Get(uvSrc->GetInteger()), c.Get(uvDest->GetInteger()), 0,
                     m_totalTime);
  }
}

void Experiment::SendMultiDestinations(Ptr<Node> sender, NodeContainer c) {
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  uv->SetAttribute("Min", DoubleValue(0));
  uv->SetAttribute("Max", DoubleValue(c.GetN()));

  Ptr<ExponentialRandomVariable> ev = CreateObject<ExponentialRandomVariable>();
  ev->SetAttribute("Mean", DoubleValue(m_expMean));
  ev->SetAttribute("Bound", DoubleValue(m_totalTime));

  double start = 0.0;
  double stop;
  uint32_t destIndex;

  for (uint32_t i = 0; i < c.GetN(); i++) {
    stop = start + ev->GetValue();
    NS_LOG_DEBUG("Start=" << start << " Stop=" << stop);

    do {
      destIndex = (uint32_t)uv->GetValue();
    } while ((c.Get(destIndex))->GetId() == sender->GetId());

    ApplicationSetup(sender, c.Get(destIndex), start, stop);

    start = stop;

    if (start > m_totalTime) {
      break;
    }
  }
}

static inline std::string PrintPosition(Ptr<Node> client, Ptr<Node> server) {
  Vector serverPos = server->GetObject<MobilityModel>()->GetPosition();
  Vector clientPos = client->GetObject<MobilityModel>()->GetPosition();

  Ptr<Ipv4> ipv4Server = server->GetObject<Ipv4>();
  Ptr<Ipv4> ipv4Client = client->GetObject<Ipv4>();

  Ipv4InterfaceAddress iaddrServer = ipv4Server->GetAddress(1, 0);
  Ipv4InterfaceAddress iaddrClient = ipv4Client->GetAddress(1, 0);

  Ipv4Address ipv4AddrServer = iaddrServer.GetLocal();
  Ipv4Address ipv4AddrClient = iaddrClient.GetLocal();

  std::ostringstream oss;
  oss << "Set up Server Device " << (server->GetDevice(0))->GetAddress()
      << " with ip " << ipv4AddrServer << " position (" << serverPos.x << ","
      << serverPos.y << "," << serverPos.z << ")";

  oss << "Set up Client Device " << (client->GetDevice(0))->GetAddress()
      << " with ip " << ipv4AddrClient << " position (" << clientPos.x << ","
      << clientPos.y << "," << clientPos.z << ")"
      << "\n";
  return oss.str();
}

void Experiment::ApplicationSetup(Ptr<Node> client, Ptr<Node> server,
                                  double start, double stop) {
  Ptr<Ipv4> ipv4Server = server->GetObject<Ipv4>();

  Ipv4InterfaceAddress iaddrServer = ipv4Server->GetAddress(1, 0);
  Ipv4Address ipv4AddrServer = iaddrServer.GetLocal();

  NS_LOG_DEBUG(PrintPosition(client, server));

  OnOffHelper onoff(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address("10.0.0.1"), m_port)));
  onoff.SetConstantRate(DataRate(60000000));
  onoff.SetAttribute("PacketSize", UintegerValue(m_packetSize));
  onoff.SetAttribute("Remote",
                     AddressValue(InetSocketAddress(ipv4AddrServer, m_port)));

  ApplicationContainer apps = onoff.Install(client);
  apps.Start(Seconds(start));
  apps.Stop(Seconds(stop));

  Ptr<Socket> sink = SetupPacketReceive(server);
}

Gnuplot2dDataset Experiment::Run(const WifiHelper &wifi,
                                 const YansWifiPhyHelper &wifiPhy,
                                 const WifiMacHelper &wifiMac,
                                 const YansWifiChannelHelper &wifiChannel,
                                 const MobilityHelper &mobility) {
  uint32_t nodeSize = m_gridSize * m_gridSize;
  NodeContainer c;
  c.Create(nodeSize);

  YansWifiPhyHelper phy = wifiPhy;
  phy.SetChannel(wifiChannel.Create());

  NetDeviceContainer devices = wifi.Install(phy, wifiMac, c);

  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;

  if (m_enableRouting) {
    list.Add(staticRouting, 0);
    list.Add(olsr, 10);
  }

  InternetStackHelper internet;

  if (m_enableRouting) {
    internet.SetRoutingHelper(list);
  }
  internet.Install(c);

  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer ipInterfaces;
  ipInterfaces = address.Assign(devices);

  MobilityHelper mobil = mobility;
  mobil.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(m_nodeDistance), "DeltaY",
      DoubleValue(m_nodeDistance), "GridWidth", UintegerValue(m_gridSize),
      "LayoutType", StringValue("RowFirst"));

  mobil.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  if (m_enableMobility && m_enableRouting) {
    mobil.SetMobilityModel(
        "ns3::RandomDirection2dMobilityModel", "Bounds",
        RectangleValue(Rectangle(0, 500, 0, 500)), "Speed",
        StringValue("ns3::ConstantRandomVariable[Constant=10]"), "Pause",
        StringValue("ns3::ConstantRandomVariable[Constant=0.2]"));
  }
  mobil.Install(c);

  if (m_scenario == 1 && m_enableRouting) {
    SelectSrcDest(c);
  } else if (m_scenario == 2) {
    for (uint32_t i = 0; i < nodeSize - 1; i = i + 2) {
      ApplicationSetup(c.Get(i), c.Get(i + 1), 0, m_totalTime);
    }
  } else if (m_scenario == 3) {
    AssignNeighbors(c);
    NS_LOG_DEBUG(">>>>>>>>>region A<<<<<<<<<");
    SendMultiDestinations(c.Get(22), m_containerA);

    NS_LOG_DEBUG(">>>>>>>>>region B<<<<<<<<<");
    SendMultiDestinations(c.Get(26), m_containerB);

    NS_LOG_DEBUG(">>>>>>>>>region C<<<<<<<<<");
    SendMultiDestinations(c.Get(72), m_containerC);

    NS_LOG_DEBUG(">>>>>>>>>region D<<<<<<<<<");
    SendMultiDestinations(c.Get(76), m_containerD);
  } else if (m_scenario == 4) {
    NodeContainer c1;
    NodeContainer c2;
    NodeContainer c3;
    NodeContainer c4;
    NodeContainer c5;
    NodeContainer c6;
    NodeContainer c7;
    NodeContainer c8;
    NodeContainer c9;

    c1 = GenerateNeighbors(c, 22);
    c2 = GenerateNeighbors(c, 24);
    c3 = GenerateNeighbors(c, 26);
    c4 = GenerateNeighbors(c, 42);
    c5 = GenerateNeighbors(c, 44);
    c6 = GenerateNeighbors(c, 46);
    c7 = GenerateNeighbors(c, 62);
    c8 = GenerateNeighbors(c, 64);
    c9 = GenerateNeighbors(c, 66);

    SendMultiDestinations(c.Get(22), c1);
    SendMultiDestinations(c.Get(24), c2);
    SendMultiDestinations(c.Get(26), c3);
    SendMultiDestinations(c.Get(42), c4);
    SendMultiDestinations(c.Get(44), c5);
    SendMultiDestinations(c.Get(46), c6);
    SendMultiDestinations(c.Get(62), c7);
    SendMultiDestinations(c.Get(64), c8);
    SendMultiDestinations(c.Get(66), c9);
  }

  CheckThroughput();

  if (m_enablePcap) {
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcapAll(GetOutputFileName());
  }

  if (m_enableTracing) {
    AsciiTraceHelper ascii;
    phy.EnableAsciiAll(ascii.CreateFileStream(GetOutputFileName() + ".tr"));
  }

  FlowMonitorHelper flowmonHelper;

  if (m_enableFlowMon) {
    flowmonHelper.InstallAll();
  }

  Simulator::Stop(Seconds(m_totalTime));
  Simulator::Run();

  if (m_enableFlowMon) {
    flowmonHelper.SerializeToXmlFile((GetOutputFileName() + ".flomon"), false,
                                     false);
  }

  Simulator::Destroy();

  return m_output;
}

bool Experiment::CommandSetup(int argc, char **argv) {
  CommandLine cmd(__FILE__);
  cmd.AddValue("packetSize", "packet size", m_packetSize);
  cmd.AddValue("totalTime", "simulation time", m_totalTime);
  if (m_totalTime < 1.0) {
    m_samplingPeriod = 0.1;
  } else {
    m_samplingPeriod = 1.0;
  }
  cmd.AddValue("samplingPeriod", "sampling period", m_samplingPeriod);
  cmd.AddValue("rtsThreshold", "rts threshold", m_rtsThreshold);
  cmd.AddValue("rateManager", "type of rate", m_rateManager);
  cmd.AddValue("outputFileName", "output filename", m_outputFileName);
  cmd.AddValue("enableRouting", "enable Routing", m_enableRouting);
  cmd.AddValue("enableMobility", "enable Mobility", m_enableMobility);
  cmd.AddValue("scenario", "scenario ", m_scenario);

  cmd.Parse(argc, argv);
  return true;
}

int main(int argc, char *argv[]) {
  Experiment experiment;
  experiment = Experiment("multirate");

  experiment.CommandSetup(argc, argv);

  std::ofstream outfile(experiment.GetOutputFileName() + ".plt");

  MobilityHelper mobility;
  Gnuplot gnuplot;
  Gnuplot2dDataset dataset;

  WifiHelper wifi;
  WifiMacHelper wifiMac;
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();

  wifiMac.SetType("ns3::AdhocWifiMac", "Ssid", StringValue("Testbed"));
  wifi.SetStandard(WIFI_STANDARD_80211a);
  wifi.SetRemoteStationManager(experiment.GetRateManager());

  NS_LOG_INFO("Scenario: " << experiment.GetScenario());
  NS_LOG_INFO("Rts Threshold: " << experiment.GetRtsThreshold());
  NS_LOG_INFO("Name:  " << experiment.GetOutputFileName());
  NS_LOG_INFO("Rate:  " << experiment.GetRateManager());
  NS_LOG_INFO("Routing: " << experiment.IsRouting());
  NS_LOG_INFO("Mobility: " << experiment.IsMobility());

  dataset = experiment.Run(wifi, wifiPhy, wifiMac, wifiChannel, mobility);

  gnuplot.AddDataset(dataset);
  gnuplot.GenerateOutput(outfile);

  return 0;
}
