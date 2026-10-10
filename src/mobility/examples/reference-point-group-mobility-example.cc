

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include <ns3/mobility-module.h>

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ReferencePointGroupMobilityExample");

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

int main(int argc, char *argv[]) {
  Time simTime = Seconds(800);
  uint32_t numPrints = 800;
  bool useHelper = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("useHelper", "Whether to use helper code", useHelper);
  cmd.Parse(argc, argv);

  g_timeSeries.open("reference-point-time-series.mob");

  NodeContainer n;
  n.Create(3);

  Ptr<WaypointMobilityModel> waypointMm = CreateObject<WaypointMobilityModel>();
  waypointMm->AddWaypoint(Waypoint(Seconds(0), Vector(10, 10, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(100), Vector(10, 40, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(200), Vector(90, 40, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(300), Vector(90, 10, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(400), Vector(10, 10, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(500), Vector(10, 40, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(600), Vector(90, 40, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(700), Vector(90, 10, 0)));
  waypointMm->AddWaypoint(Waypoint(Seconds(800), Vector(10, 10, 0)));

  int64_t streamIndex = 1;
  if (!useHelper) {
    streamIndex += waypointMm->AssignStreams(streamIndex);

    Ptr<HierarchicalMobilityModel> hierarchical0 =
        CreateObject<HierarchicalMobilityModel>();
    hierarchical0->SetParent(waypointMm);

    Ptr<RandomWalk2dMobilityModel> childRandomWalk0 =
        CreateObject<RandomWalk2dMobilityModel>();
    childRandomWalk0->SetAttribute("Bounds",
                                   RectangleValue(Rectangle(-5, 5, -5, 5)));
    childRandomWalk0->SetAttribute(
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=0.1]"));
    streamIndex += childRandomWalk0->AssignStreams(streamIndex);
    hierarchical0->SetChild(childRandomWalk0);
    n.Get(0)->AggregateObject(hierarchical0);
    Ptr<HierarchicalMobilityModel> hierarchical1 =
        CreateObject<HierarchicalMobilityModel>();
    hierarchical1->SetParent(waypointMm);
    Ptr<RandomWalk2dMobilityModel> childRandomWalk1 =
        CreateObject<RandomWalk2dMobilityModel>();
    childRandomWalk1->SetAttribute("Bounds",
                                   RectangleValue(Rectangle(-5, 5, -5, 5)));
    childRandomWalk1->SetAttribute(
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=0.1]"));
    streamIndex += childRandomWalk1->AssignStreams(streamIndex);
    hierarchical1->SetChild(childRandomWalk1);
    n.Get(1)->AggregateObject(hierarchical1);
    Ptr<HierarchicalMobilityModel> hierarchical2 =
        CreateObject<HierarchicalMobilityModel>();
    hierarchical2->SetParent(waypointMm);
    Ptr<RandomWalk2dMobilityModel> childRandomWalk2 =
        CreateObject<RandomWalk2dMobilityModel>();
    childRandomWalk2->SetAttribute("Bounds",
                                   RectangleValue(Rectangle(-5, 5, -5, 5)));
    childRandomWalk2->SetAttribute(
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=0.1]"));
    streamIndex += childRandomWalk2->AssignStreams(streamIndex);
    hierarchical2->SetChild(childRandomWalk2);
    n.Get(2)->AggregateObject(hierarchical2);
  } else {
    GroupMobilityHelper group;

    group.SetReferenceMobilityModel(waypointMm);

    group.SetMemberMobilityModel(
        "ns3::RandomWalk2dMobilityModel", "Bounds",
        RectangleValue(Rectangle(-5, 5, -5, 5)), "Speed",
        StringValue("ns3::ConstantRandomVariable[Constant=0.1]"));

    group.Install(n);

    group.AssignStreams(n, streamIndex);
  }

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll(
      ascii.CreateFileStream("reference-point-course-change.mob"));

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
}
