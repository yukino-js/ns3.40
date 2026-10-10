
#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/simulator.h"

#include <iostream>

using namespace ns3;

void FatalNoMsg() {
  std::cerr << "\nEvent triggered fatal error without message, and continuing:"
            << std::endl;
  NS_FATAL_ERROR_NO_MSG_CONT();
}

void FatalCont() {
  std::cerr
      << "\nEvent triggered fatal error, with custom message, and continuing:"
      << std::endl;
  NS_FATAL_ERROR_CONT("fatal error, but continuing");
}

void Fatal() {
  std::cerr << "\nEvent triggered fatal error, with message, and terminating:"
            << std::endl;
  NS_FATAL_ERROR("fatal error, terminating");
}

int main(int argc, char **argv) {
  Simulator::Schedule(Seconds(1), FatalNoMsg);
  Simulator::Schedule(Seconds(2), FatalCont);
  Simulator::Schedule(Seconds(3), Fatal);

  std::cerr << "\nFatal error with custom message, and continuing:"
            << std::endl;
  NS_FATAL_ERROR_CONT("fatal error, but continuing");

  std::cerr << "\nFatal error without message, and continuing:" << std::endl;
  NS_FATAL_ERROR_NO_MSG_CONT();

  Simulator::Run();

  NS_FATAL_ERROR("fatal error, terminating");
  NS_ASSERT_MSG(false, "Should not get here.");

  return 0;
}
