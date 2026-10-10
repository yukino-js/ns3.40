

#include "wifi-example-apps.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/stats-module.h"
#include "ns3/wifi-module.h"

#include <ctime>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WiFiDistanceExperiment");

void TxCallback(Ptr<CounterCalculator<uint32_t>> datac, std::string path,
                Ptr<const Packet> packet) {
  NS_LOG_INFO("Sent frame counted in " << datac->GetKey());
  datac->Update();
}

int main(int argc, char *argv[]) {
  double distance = 50.0;
  std::string format("omnet");

  std::string experiment("wifi-distance-test");
  std::string strategy("wifi-default");
  std::string input;
  std::string runID;

  {
    std::stringstream sstr;
    sstr << "run-" << time(nullptr);
    runID = sstr.str();
  }

  CommandLine cmd(__FILE__);
  cmd.AddValue("distance", "Distance apart to place nodes (in meters).",
               distance);
  cmd.AddValue("format", "Format to use for data output.", format);
  cmd.AddValue("experiment", "Identifier for experiment.", experiment);
  cmd.AddValue("strategy", "Identifier for strategy.", strategy);
  cmd.AddValue("run", "Identifier for run.", runID);
  cmd.Parse(argc, argv);

  if (format != "omnet" && format != "db") {
    NS_LOG_ERROR("Unknown output format '" << format << "'");
    return -1;
  }

#ifndef HAVE_SQLITE3
  if (format == "db") {
    NS_LOG_ERROR("sqlite support not compiled in.");
    return -1;
  }
#endif

  {
    std::stringstream sstr("");
    sstr << distance;
    input = sstr.str();
  }

  NS_LOG_INFO("Creating nodes.");
  NodeContainer nodes;
  nodes.Create(2);

  NS_LOG_INFO("Installing WiFi and Internet stack.");
  WifiHelper wifi;
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());
  NetDeviceContainer nodeDevices = wifi.Install(wifiPhy, wifiMac, nodes);

  InternetStackHelper internet;
  internet.Install(nodes);
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase("192.168.0.0", "255.255.255.0");
  ipAddrs.Assign(nodeDevices);

  NS_LOG_INFO("Installing static mobility; distance " << distance << " .");
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, distance, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(nodes);

  NS_LOG_INFO("Create traffic source & sink.");
  Ptr<Node> appSource = NodeList::GetNode(0);
  Ptr<Sender> sender = CreateObject<Sender>();
  appSource->AddApplication(sender);
  sender->SetStartTime(Seconds(1));

  Ptr<Node> appSink = NodeList::GetNode(1);
  Ptr<Receiver> receiver = CreateObject<Receiver>();
  appSink->AddApplication(receiver);
  receiver->SetStartTime(Seconds(0));

  Config::Set("/NodeList/*/ApplicationList/*/$Sender/Destination",
              Ipv4AddressValue("192.168.0.2"));

  DataCollector data;
  data.DescribeRun(experiment, strategy, input, runID);

  data.AddMetadata("author", "tjkopena");

  Ptr<CounterCalculator<uint32_t>> totalTx =
      CreateObject<CounterCalculator<uint32_t>>();
  totalTx->SetKey("wifi-tx-frames");
  totalTx->SetContext("node[0]");
  Config::Connect("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
                  MakeBoundCallback(&TxCallback, totalTx));
  data.AddDataCalculator(totalTx);

  Ptr<PacketCounterCalculator> totalRx =
      CreateObject<PacketCounterCalculator>();
  totalRx->SetKey("wifi-rx-frames");
  totalRx->SetContext("node[1]");
  Config::Connect(
      "/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
      MakeCallback(&PacketCounterCalculator::PacketUpdate, totalRx));
  data.AddDataCalculator(totalRx);

  Ptr<PacketCounterCalculator> appTx = CreateObject<PacketCounterCalculator>();
  appTx->SetKey("sender-tx-packets");
  appTx->SetContext("node[0]");
  Config::Connect("/NodeList/0/ApplicationList/*/$Sender/Tx",
                  MakeCallback(&PacketCounterCalculator::PacketUpdate, appTx));
  data.AddDataCalculator(appTx);

  Ptr<CounterCalculator<>> appRx = CreateObject<CounterCalculator<>>();
  appRx->SetKey("receiver-rx-packets");
  appRx->SetContext("node[1]");
  receiver->SetCounter(appRx);
  data.AddDataCalculator(appRx);

  Ptr<PacketSizeMinMaxAvgTotalCalculator> appTxPkts =
      CreateObject<PacketSizeMinMaxAvgTotalCalculator>();
  appTxPkts->SetKey("tx-pkt-size");
  appTxPkts->SetContext("node[0]");
  Config::Connect(
      "/NodeList/0/ApplicationList/*/$Sender/Tx",
      MakeCallback(&PacketSizeMinMaxAvgTotalCalculator::PacketUpdate,
                   appTxPkts));
  data.AddDataCalculator(appTxPkts);

  Ptr<TimeMinMaxAvgTotalCalculator> delayStat =
      CreateObject<TimeMinMaxAvgTotalCalculator>();
  delayStat->SetKey("delay");
  delayStat->SetContext(".");
  receiver->SetDelayTracker(delayStat);
  data.AddDataCalculator(delayStat);

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();

  Ptr<DataOutputInterface> output = nullptr;
  if (format == "omnet") {
    NS_LOG_INFO("Creating omnet formatted data output.");
    output = CreateObject<OmnetDataOutput>();
  } else if (format == "db") {
#ifdef HAVE_SQLITE3
    NS_LOG_INFO("Creating sqlite formatted data output.");
    output = CreateObject<SqliteDataOutput>();
#endif
  } else {
    NS_LOG_ERROR("Unknown output format " << format);
  }

  if (output) {
    output->Output(data);
  }

  Simulator::Destroy();

  return 0;
}
