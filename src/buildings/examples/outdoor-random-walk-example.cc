
#include "ns3/buildings-module.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OutdoorRandomWalkExample");

void PrintGnuplottableBuildingListToFile(std::string filename) {
  std::ofstream outFile;
  outFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open()) {
    NS_LOG_ERROR("Can't open file " << filename);
    return;
  }
  uint32_t index = 0;
  for (auto it = BuildingList::Begin(); it != BuildingList::End(); ++it) {
    ++index;
    Box box = (*it)->GetBoundaries();
    outFile << "set object " << index << " rect from " << box.xMin << ","
            << box.yMin << " to " << box.xMax << "," << box.yMax << std::endl;
  }
}

int main(int argc, char *argv[]) {
  LogComponentEnable("RandomWalk2dOutdoor", LOG_LEVEL_LOGIC);
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  double buildingSizeX = 100;
  double buildingSizeY = 50;
  double streetWidth = 25;
  double buildingHeight = 10;
  uint32_t numBuildingsX = 10;
  uint32_t numBuildingsY = 10;
  double maxAxisX = (buildingSizeX + streetWidth) * numBuildingsX;
  double maxAxisY = (buildingSizeY + streetWidth) * numBuildingsY;

  std::vector<Ptr<Building>> buildingVector;
  for (uint32_t buildingIdX = 0; buildingIdX < numBuildingsX; ++buildingIdX) {
    for (uint32_t buildingIdY = 0; buildingIdY < numBuildingsY; ++buildingIdY) {
      Ptr<Building> building;
      building = CreateObject<Building>();

      building->SetBoundaries(
          Box(buildingIdX * (buildingSizeX + streetWidth),
              buildingIdX * (buildingSizeX + streetWidth) + buildingSizeX,
              buildingIdY * (buildingSizeY + streetWidth),
              buildingIdY * (buildingSizeY + streetWidth) + buildingSizeY, 0.0,
              buildingHeight));
      building->SetNRoomsX(1);
      building->SetNRoomsY(1);
      building->SetNFloors(1);
      buildingVector.push_back(building);
    }
  }

  PrintGnuplottableBuildingListToFile("buildings.txt");

  NodeContainer nodes;
  nodes.Create(1);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::RandomWalk2dOutdoorMobilityModel", "Bounds",
                            RectangleValue(Rectangle(-streetWidth, maxAxisX,
                                                     -streetWidth, maxAxisY)));
  Ptr<OutdoorPositionAllocator> position =
      CreateObject<OutdoorPositionAllocator>();
  Ptr<UniformRandomVariable> xPos = CreateObject<UniformRandomVariable>();
  xPos->SetAttribute("Min", DoubleValue(-streetWidth));
  xPos->SetAttribute("Max", DoubleValue(maxAxisX));
  Ptr<UniformRandomVariable> yPos = CreateObject<UniformRandomVariable>();
  yPos->SetAttribute("Min", DoubleValue(-streetWidth));
  yPos->SetAttribute("Max", DoubleValue(maxAxisY));
  position->SetAttribute("X", PointerValue(xPos));
  position->SetAttribute("Y", PointerValue(yPos));
  mobility.SetPositionAllocator(position);
  mobility.Install(nodes.Get(0));

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll(
      ascii.CreateFileStream("mobility-trace-example.mob"));

  Simulator::Stop(Seconds(1e4));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
