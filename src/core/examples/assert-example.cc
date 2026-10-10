

#include "ns3/core-module.h"

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AssertExample");

int main(int argc, char **argv) {
  CommandLine cmd;
  cmd.Parse(argc, argv);

  std::cout << "NS_ASSERT_MSG example\n"
            << "  if an argument is given this example will assert.\n";

  NS_ASSERT_MSG(argc == 1, "An argument was given, so we assert");

  return 0;
}
