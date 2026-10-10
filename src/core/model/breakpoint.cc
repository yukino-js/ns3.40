
#include "breakpoint.h"

#include "log.h"

#include "ns3/core-config.h"
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Breakpoint");

#if defined(HAVE_SIGNAL_H) && defined(SIGTRAP)

void BreakpointFallback() {
  NS_LOG_FUNCTION_NOARGS();

  raise(SIGTRAP);
}

#else

void BreakpointFallback() {
  NS_LOG_FUNCTION_NOARGS();

  int *a = nullptr;
  if (a == nullptr) {
    *a = 0;
  }
}

#endif

} // namespace ns3
