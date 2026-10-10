
#include "node-printer.h"

#include "log.h"
#include "simulator.h"

#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NodePrinter");

void DefaultNodePrinter(std::ostream &os) {
  if (Simulator::GetContext() == Simulator::NO_CONTEXT) {
    os << "-1";
  } else {
    os << Simulator::GetContext();
  }
}

} // namespace ns3
