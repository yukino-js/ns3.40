
#include "time-printer.h"

#include "log.h"
#include "nstime.h"
#include "simulator.h"

#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TimePrinter");

void DefaultTimePrinter(std::ostream &os) {
  std::ios_base::fmtflags ff = os.flags();
  std::streamsize oldPrecision = os.precision();
  os << std::fixed;
  switch (Time::GetResolution()) {
  case Time::US:
    os << std::setprecision(6);
    break;
  case Time::NS:
    os << std::setprecision(9);
    break;
  case Time::PS:
    os << std::setprecision(12);
    break;
  case Time::FS:
    os << std::setprecision(15);
    break;

  default:
    os << std::setprecision(5);
  }
  os << Simulator::Now().As(Time::S);

  os << std::setprecision(oldPrecision);
  os.flags(ff);
}

} // namespace ns3
