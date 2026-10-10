

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

void showPosition(Ptr<Node> node, double deltaTime) {
  uint32_t nodeId = node->GetId();
  Ptr<MobilityModel> mobModel = node->GetObject<MobilityModel>();
  Vector3D pos = mobModel->GetPosition();
  Vector3D speed = mobModel->GetVelocity();
  std::cout << "At " << Simulator::Now().GetSeconds() << " node " << nodeId
            << ": Position(" << pos.x << ", " << pos.y << ", " << pos.z
            << ");   Speed(" << speed.x << ", " << speed.y << ", " << speed.z
            << ")" << std::endl;

  Simulator::Schedule(Seconds(deltaTime), &showPosition, node, deltaTime);
}

int main(int argc, char *argv[]) {
  std::cout.precision(2);
  std::cout.setf(std::ios::fixed);

  double deltaTime = 100;
  std::string traceFile = "src/mobility/examples/bonnmotion.ns_movements";

  CommandLine cmd(__FILE__);
  cmd.AddValue("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue("deltaTime", "time interval (s) between updates (default 100)",
               deltaTime);
  cmd.Parse(argc, argv);

  Ptr<Node> n0 = CreateObject<Node>();

  Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);
  ns2.Install();

  Simulator::Schedule(Seconds(0.0), &showPosition, n0, deltaTime);

  Simulator::Stop(Seconds(1000.0));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
