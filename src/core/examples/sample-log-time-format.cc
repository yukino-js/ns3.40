

#include "ns3/command-line.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

#include <map>

using namespace ns3;

namespace {

void ReplacementTimePrinter(std::ostream &os) {
  os << Simulator::Now().GetSeconds() << "s";
}

void ReplaceTimePrinter() {
  std::cout << "Replacing time printer function after Simulator::Run ()"
            << std::endl;
  LogSetTimePrinter(&ReplacementTimePrinter);
}

} // namespace

int main(int argc, char *argv[]) {
  bool replaceTimePrinter = false;
  std::string resolution = "Time::NS";
  LogComponentEnable("RandomVariableStream", LOG_LEVEL_ALL);
  LogComponentEnableAll(LOG_PREFIX_TIME);

  std::map<std::string, Time::Unit> resolutionMap = {
      {"Time::US", Time::US},
      {"Time::NS", Time::NS},
      {"Time::PS", Time::PS},
      {"Time::FS", Time::FS},
  };

  CommandLine cmd(__FILE__);
  cmd.AddValue("replaceTimePrinter", "replace time printing function",
               replaceTimePrinter);
  cmd.AddValue("resolution", "time resolution", resolution);
  cmd.Parse(argc, argv);

  auto search = resolutionMap.find(resolution);
  if (search != resolutionMap.end()) {
    Time::SetResolution(search->second);
  }

  Ptr<UniformRandomVariable> uniformRv = CreateObject<UniformRandomVariable>();

  if (replaceTimePrinter) {
    Simulator::Schedule(Seconds(0), &ReplaceTimePrinter);
  }

  Simulator::Schedule(NanoSeconds(1), &UniformRandomVariable::SetAntithetic,
                      uniformRv, false);
  Simulator::Schedule(NanoSeconds(123), &UniformRandomVariable::SetAntithetic,
                      uniformRv, false);
  Simulator::Schedule(NanoSeconds(123456),
                      &UniformRandomVariable::SetAntithetic, uniformRv, false);
  Simulator::Schedule(NanoSeconds(123456789),
                      &UniformRandomVariable::SetAntithetic, uniformRv, false);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
