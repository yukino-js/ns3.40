
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NodeContainer nodes;

  nodes.Create(120);

  MobilityHelper mobility;
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(-100.0), "MinY",
      DoubleValue(-100.0), "DeltaX", DoubleValue(5.0), "DeltaY",
      DoubleValue(20.0), "GridWidth", UintegerValue(20), "LayoutType",
      StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  mobility.Install(nodes);

  for (auto j = nodes.Begin(); j != nodes.End(); ++j) {
    Ptr<Node> object = *j;
    Ptr<MobilityModel> position = object->GetObject<MobilityModel>();
    NS_ASSERT(position);
    Vector pos = position->GetPosition();
    std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z
              << std::endl;
  }

  Simulator::Destroy();
  return 0;
}
