

#include "scratch-subdir-additional-header.h"

#include "ns3/core-module.h"

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScratchSubdir");

int main(int argc, char *argv[]) {
  std::string message = ScratchSubdirGetMessage();
  NS_LOG_UNCOND(message);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
