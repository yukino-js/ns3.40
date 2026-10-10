
#include "ns3/applications-module.h"
#include "ns3/assert.h"
#include "ns3/core-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

std::vector<std::vector<bool>> readNxNMatrix(std::string adj_mat_file_name);
std::vector<std::vector<double>>
readCoordinatesFile(std::string node_coordinates_file_name);
void printCoordinateArray(const char *description,
                          std::vector<std::vector<double>> coord_array);
void printMatrix(const char *description, std::vector<std::vector<bool>> array);

NS_LOG_COMPONENT_DEFINE("GenericTopologyCreation");

int main(int argc, char *argv[]) {

  double SimTime = 3.00;
  double SinkStartTime = 1.0001;
  double SinkStopTime = 2.90001;
  double AppStartTime = 2.0001;
  double AppStopTime = 2.80001;

  std::string AppPacketRate("40Kbps");
  Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("1000"));
  Config::SetDefault("ns3::OnOffApplication::DataRate",
                     StringValue(AppPacketRate));
  std::string LinkRate("10Mbps");
  std::string LinkDelay("2ms");

  srand((unsigned)time(nullptr));

  std::string tr_name("n-node-ppp.tr");
  std::string pcap_name("n-node-ppp");
  std::string flow_name("n-node-ppp.xml");
  std::string anim_name("n-node-ppp.anim.xml");

  std::string adj_mat_file_name(
      "examples/matrix-topology/adjacency_matrix.txt");
  std::string node_coordinates_file_name(
      "examples/matrix-topology/node_coordinates.txt");

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  std::vector<std::vector<bool>> Adj_Matrix;
  Adj_Matrix = readNxNMatrix(adj_mat_file_name);

  std::vector<std::vector<double>> coord_array;
  coord_array = readCoordinatesFile(node_coordinates_file_name);

  int n_nodes = coord_array.size();
  int matrixDimension = Adj_Matrix.size();

  if (matrixDimension != n_nodes) {
    NS_FATAL_ERROR(
        "The number of lines in coordinate file is: "
        << n_nodes
        << " not equal to the number of nodes in adjacency matrix size "
        << matrixDimension);
  }

  NS_LOG_INFO("Create Nodes.");

  NodeContainer nodes;
  nodes.Create(n_nodes);

  NS_LOG_INFO("Create P2P Link Attributes.");

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(LinkRate));
  p2p.SetChannelAttribute("Delay", StringValue(LinkDelay));

  NS_LOG_INFO("Install Internet Stack to Nodes.");

  InternetStackHelper internet;
  internet.Install(NodeContainer::GetGlobal());

  NS_LOG_INFO("Assign Addresses to Nodes.");

  Ipv4AddressHelper ipv4_n;
  ipv4_n.SetBase("10.0.0.0", "255.255.255.252");

  NS_LOG_INFO("Create Links Between Nodes.");

  uint32_t linkCount = 0;

  for (size_t i = 0; i < Adj_Matrix.size(); i++) {
    for (size_t j = 0; j < Adj_Matrix[i].size(); j++) {
      if (Adj_Matrix[i][j]) {
        NodeContainer n_links = NodeContainer(nodes.Get(i), nodes.Get(j));
        NetDeviceContainer n_devs = p2p.Install(n_links);
        ipv4_n.Assign(n_devs);
        ipv4_n.NewNetwork();
        linkCount++;
        NS_LOG_INFO("matrix element [" << i << "][" << j << "] is 1");
      } else {
        NS_LOG_INFO("matrix element [" << i << "][" << j << "] is 0");
      }
    }
  }
  NS_LOG_INFO("Number of links in the adjacency matrix is: " << linkCount);
  NS_LOG_INFO("Number of all nodes is: " << nodes.GetN());

  NS_LOG_INFO("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Allocate Positions to Nodes.");

  MobilityHelper mobility_n;
  Ptr<ListPositionAllocator> positionAlloc_n =
      CreateObject<ListPositionAllocator>();

  for (size_t m = 0; m < coord_array.size(); m++) {
    positionAlloc_n->Add(Vector(coord_array[m][0], coord_array[m][1], 0));
    Ptr<Node> n0 = nodes.Get(m);
    Ptr<ConstantPositionMobilityModel> nLoc =
        n0->GetObject<ConstantPositionMobilityModel>();
    if (!nLoc) {
      nLoc = CreateObject<ConstantPositionMobilityModel>();
      n0->AggregateObject(nLoc);
    }
    Vector nVec(coord_array[m][0], -coord_array[m][1], 0);
    nLoc->SetPosition(nVec);
  }
  mobility_n.SetPositionAllocator(positionAlloc_n);
  mobility_n.Install(nodes);

  NS_LOG_INFO("Setup Packet Sinks.");

  uint16_t port = 9;

  for (int i = 0; i < n_nodes; i++) {
    PacketSinkHelper sink("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer apps_sink = sink.Install(nodes.Get(i));
    apps_sink.Start(Seconds(SinkStartTime));
    apps_sink.Stop(Seconds(SinkStopTime));
  }

  NS_LOG_INFO("Setup CBR Traffic Sources.");

  for (int i = 0; i < n_nodes; i++) {
    for (int j = 0; j < n_nodes; j++) {
      if (i != j) {

        Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
        x->SetAttribute("Min", DoubleValue(0));
        x->SetAttribute("Max", DoubleValue(1));
        double rn = x->GetValue();
        Ptr<Node> n = nodes.Get(j);
        Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
        Ipv4InterfaceAddress ipv4_int_addr = ipv4->GetAddress(1, 0);
        Ipv4Address ip_addr = ipv4_int_addr.GetLocal();
        OnOffHelper onoff("ns3::UdpSocketFactory",
                          InetSocketAddress(ip_addr, port));
        onoff.SetConstantRate(DataRate(AppPacketRate));
        ApplicationContainer apps = onoff.Install(nodes.Get(i));
        apps.Start(Seconds(AppStartTime + rn));
        apps.Stop(Seconds(AppStopTime));
      }
    }
  }

  NS_LOG_INFO("Configure Tracing.");

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream(tr_name));

  AnimationInterface anim(anim_name);
  NS_LOG_INFO("Run Simulation.");

  Simulator::Stop(Seconds(SimTime));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

std::vector<std::vector<bool>> readNxNMatrix(std::string adj_mat_file_name) {
  std::ifstream adj_mat_file;
  adj_mat_file.open(adj_mat_file_name, std::ios::in);
  if (adj_mat_file.fail()) {
    NS_FATAL_ERROR("File " << adj_mat_file_name << " not found");
  }
  std::vector<std::vector<bool>> array;
  int i = 0;
  int n_nodes = 0;

  while (!adj_mat_file.eof()) {
    std::string line;
    getline(adj_mat_file, line);
    if (line.empty()) {
      NS_LOG_WARN("WARNING: Ignoring blank row in the array: " << i);
      break;
    }

    std::istringstream iss(line);
    bool element;
    std::vector<bool> row;
    int j = 0;

    while (iss >> element) {
      row.push_back(element);
      j++;
    }

    if (i == 0) {
      n_nodes = j;
    }

    if (j != n_nodes) {
      NS_LOG_ERROR("ERROR: Number of elements in line "
                   << i << ": " << j
                   << " not equal to number of elements in line 0: "
                   << n_nodes);
      NS_FATAL_ERROR(
          "ERROR: The number of rows is not equal to the number of columns! in "
          "the adjacency matrix");
    } else {
      array.push_back(row);
    }
    i++;
  }

  if (i != n_nodes) {
    NS_LOG_ERROR("There are " << i << " rows and " << n_nodes << " columns.");
    NS_FATAL_ERROR("ERROR: The number of rows is not equal to the number of "
                   "columns! in the "
                   "adjacency matrix");
  }

  adj_mat_file.close();
  return array;
}

std::vector<std::vector<double>>
readCoordinatesFile(std::string node_coordinates_file_name) {
  std::ifstream node_coordinates_file;
  node_coordinates_file.open(node_coordinates_file_name, std::ios::in);
  if (node_coordinates_file.fail()) {
    NS_FATAL_ERROR("File " << node_coordinates_file_name << " not found");
  }
  std::vector<std::vector<double>> coord_array;
  int m = 0;

  while (!node_coordinates_file.eof()) {
    std::string line;
    getline(node_coordinates_file, line);

    if (line.empty()) {
      NS_LOG_WARN("WARNING: Ignoring blank row: " << m);
      break;
    }

    std::istringstream iss(line);
    double coordinate;
    std::vector<double> row;
    int n = 0;
    while (iss >> coordinate) {
      row.push_back(coordinate);
      n++;
    }

    if (n != 2) {
      NS_LOG_ERROR("ERROR: Number of elements at line#"
                   << m << " is " << n
                   << " which is not equal to 2 for node coordinates file");
      exit(1);
    }

    else {
      coord_array.push_back(row);
    }
    m++;
  }
  node_coordinates_file.close();
  return coord_array;
}

void printMatrix(const char *description,
                 std::vector<std::vector<bool>> array) {
  std::cout << "**** Start " << description << "********" << std::endl;
  for (size_t m = 0; m < array.size(); m++) {
    for (size_t n = 0; n < array[m].size(); n++) {
      std::cout << array[m][n] << ' ';
    }
    std::cout << std::endl;
  }
  std::cout << "**** End " << description << "********" << std::endl;
}

void printCoordinateArray(const char *description,
                          std::vector<std::vector<double>> coord_array) {
  std::cout << "**** Start " << description << "********" << std::endl;
  for (size_t m = 0; m < coord_array.size(); m++) {
    for (size_t n = 0; n < coord_array[m].size(); n++) {
      std::cout << coord_array[m][n] << ' ';
    }
    std::cout << std::endl;
  }
  std::cout << "**** End " << description << "********" << std::endl;
}
