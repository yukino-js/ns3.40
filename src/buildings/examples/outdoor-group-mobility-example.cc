

#include "ns3/buildings-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include <ns3/mobility-module.h>

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OutdoorGroupMobilityExample");

std::ofstream g_timeSeries;

void PrintPosition(Ptr<Node> node) {
  if (!node) {
    return;
  }
  Ptr<MobilityModel> model = node->GetObject<MobilityModel>();
  if (!model) {
    return;
  }
  NS_LOG_LOGIC("Node: " << node->GetId()
                        << " Position: " << model->GetPosition());
  g_timeSeries << Simulator::Now().GetSeconds() << " " << node->GetId() << " "
               << model->GetPosition() << std::endl;
}

void PrintGnuplottableBuildingListToFile(std::string filename) {
  std::ofstream outFile;
  outFile.open(filename, std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open()) {
    NS_LOG_ERROR("Can't open file " << filename);
    return;
  }
  uint32_t index = 1;
  for (auto it = BuildingList::Begin(); it != BuildingList::End(); ++it) {
    ++index;
    Box box = (*it)->GetBoundaries();
    outFile << "set object " << index << " rect from " << box.xMin << ","
            << box.yMin << " to " << box.xMax << "," << box.yMax << std::endl;
  }
}

int main(int argc, char *argv[]) {
  Time simTime = Seconds(800);
  uint32_t numPrints = 800;
  bool useHelper = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("useHelper", "Whether to use helper code", useHelper);
  cmd.Parse(argc, argv);

  g_timeSeries.open("outdoor-group-mobility-time-series.mob");

  NodeContainer n;
  n.Create(3);

  std::vector<Ptr<Building>> buildingVector;
  Ptr<Building> building = CreateObject<Building>();
  building->SetBoundaries(Box(45, 55, 5, 15, 0, 10));
  buildingVector.push_back(building);
  building = CreateObject<Building>();
  building->SetBoundaries(Box(45, 55, 35, 45, 0, 10));
  buildingVector.push_back(building);
  building = CreateObject<Building>();
  building->SetBoundaries(Box(17.5, 22.5, 22.5, 27.5, 0, 10));
  buildingVector.push_back(building);
  building = CreateObject<Building>();
  building->SetBoundaries(Box(77.5, 82.5, 22.5, 27.5, 0, 10));
  buildingVector.push_back(building);

  PrintGnuplottableBuildingListToFile("outdoor-group-mobility-buildings.txt");

  int64_t streamIndex = 1;
  if (!useHelper) {
    Ptr<RandomWalk2dOutdoorMobilityModel> outdoorMm =
        CreateObject<RandomWalk2dOutdoorMobilityModel>();
    outdoorMm->SetAttribute("Bounds", RectangleValue(Rectangle(0, 100, 0, 50)));
    outdoorMm->SetAttribute("Tolerance", DoubleValue(2));
    outdoorMm->SetPosition(Vector(10, 10, 0));
    streamIndex += outdoorMm->AssignStreams(streamIndex);

    Ptr<HierarchicalMobilityModel> hierarchical0 =
        CreateObject<HierarchicalMobilityModel>();
    hierarchical0->SetParent(outdoorMm);

    Ptr<ConstantPositionMobilityModel> child0 =
        CreateObject<ConstantPositionMobilityModel>();
    child0->SetPosition(Vector(1, 0, 0));
    hierarchical0->SetChild(child0);
    n.Get(0)->AggregateObject(hierarchical0);

    Ptr<HierarchicalMobilityModel> hierarchical1 =
        CreateObject<HierarchicalMobilityModel>();
    hierarchical1->SetParent(outdoorMm);
    Ptr<ConstantPositionMobilityModel> child1 =
        CreateObject<ConstantPositionMobilityModel>();
    child1->SetPosition(Vector(-1, 0, 0));
    hierarchical1->SetChild(child1);
    n.Get(1)->AggregateObject(hierarchical1);
    Ptr<HierarchicalMobilityModel> hierarchical2 =
        CreateObject<HierarchicalMobilityModel>();
    hierarchical2->SetParent(outdoorMm);
    Ptr<ConstantPositionMobilityModel> child2 =
        CreateObject<ConstantPositionMobilityModel>();
    child2->SetPosition(Vector(0, 1, 0));
    hierarchical2->SetChild(child2);
    n.Get(2)->AggregateObject(hierarchical2);
  } else {
    GroupMobilityHelper group;

    group.SetReferenceMobilityModel(
        "ns3::RandomWalk2dOutdoorMobilityModel", "Bounds",
        RectangleValue(Rectangle(0, 100, 0, 50)), "Tolerance", DoubleValue(2));
    Ptr<ListPositionAllocator> listPosition =
        CreateObject<ListPositionAllocator>();
    listPosition->Add(Vector(10, 10, 0));
    group.SetReferencePositionAllocator(listPosition);

    group.SetMemberMobilityModel("ns3::ConstantPositionMobilityModel");
    listPosition = CreateObject<ListPositionAllocator>();
    listPosition->Add(Vector(1, 0, 0));
    listPosition->Add(Vector(-1, 0, 0));
    listPosition->Add(Vector(0, 1, 0));
    group.SetMemberPositionAllocator(listPosition);

    group.Install(n);

    group.AssignStreams(n, streamIndex);
  }

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll(
      ascii.CreateFileStream("outdoor-group-mobility-course-change.mob"));

  for (unsigned int i = 0; i < numPrints; i++) {
    for (auto nodeIt = n.Begin(); nodeIt != n.End(); ++nodeIt) {
      Simulator::Schedule(NanoSeconds(i * simTime.GetNanoSeconds() / numPrints),
                          &PrintPosition, (*nodeIt));
    }
  }

  Simulator::Stop(simTime);
  Simulator::Run();
  g_timeSeries.close();
  Simulator::Destroy();

  return 0;
}
