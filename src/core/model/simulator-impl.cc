
#include "simulator-impl.h"

#include "log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SimulatorImpl");

NS_OBJECT_ENSURE_REGISTERED(SimulatorImpl);

TypeId SimulatorImpl::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::SimulatorImpl").SetParent<Object>().SetGroupName("Core");
  return tid;
}

} // namespace ns3
