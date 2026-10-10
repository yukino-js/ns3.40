#include "ns3/command-line.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

#include <iostream>

using namespace ns3;

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();

  std::cout << uv->GetValue() << std::endl;

  return 0;
}
