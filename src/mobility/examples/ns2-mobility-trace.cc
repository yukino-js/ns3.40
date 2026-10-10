
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace ns3;

static void CourseChange(std::ostream *os, std::string foo,
                         Ptr<const MobilityModel> mobility) {
  Vector pos = mobility->GetPosition();
  Vector vel = mobility->GetVelocity();

  *os << Simulator::Now() << " POS: x=" << pos.x << ", y=" << pos.y
      << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
      << ", z=" << vel.z << std::endl;
}

int main(int argc, char *argv[]) {
  std::string traceFile;
  std::string logFile;

  int nodeNum;
  double duration;

  LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_DEBUG);

  CommandLine cmd(__FILE__);
  cmd.AddValue("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue("nodeNum", "Number of nodes", nodeNum);
  cmd.AddValue("duration", "Duration of Simulation", duration);
  cmd.AddValue("logFile", "Log file", logFile);
  cmd.Parse(argc, argv);

  if (traceFile.empty() || nodeNum <= 0 || duration <= 0 || logFile.empty()) {
    std::cout
        << "Usage of " << argv[0]
        << " :\n\n"
           "./ns3 run \"ns2-mobility-trace"
           " --traceFile=src/mobility/examples/default.ns_movements"
           " --nodeNum=2 --duration=100.0 --logFile=ns2-mob.log\" \n\n"
           "NOTE: ns2-traces-file could be an absolute or relative path. You "
           "could use "
           "the file default.ns_movements\n"
           "      included in the same directory of this example file.\n\n"
           "NOTE 2: Number of nodes present in the trace file must match with "
           "the "
           "command line argument and must\n"
           "        be a positive number. Note that you must know it before to "
           "be able "
           "to load it.\n\n"
           "NOTE 3: Duration must be a positive number. Note that you must "
           "know it "
           "before to be able to load it.\n\n";

    return 0;
  }

  Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);

  std::ofstream os;
  os.open(logFile);

  NodeContainer stas;
  stas.Create(nodeNum);

  ns2.Install();

  Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
                  MakeBoundCallback(&CourseChange, &os));

  Simulator::Stop(Seconds(duration));
  Simulator::Run();
  Simulator::Destroy();

  os.close();
  return 0;
}
