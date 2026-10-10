
#include "global-route-manager.h"

#include "global-route-manager-impl.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulation-singleton.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GlobalRouteManager");

void GlobalRouteManager::DeleteGlobalRoutes() {
  NS_LOG_FUNCTION_NOARGS();
  SimulationSingleton<GlobalRouteManagerImpl>::Get()->DeleteGlobalRoutes();
}

void GlobalRouteManager::BuildGlobalRoutingDatabase() {
  NS_LOG_FUNCTION_NOARGS();
  SimulationSingleton<GlobalRouteManagerImpl>::Get()
      ->BuildGlobalRoutingDatabase();
}

void GlobalRouteManager::InitializeRoutes() {
  NS_LOG_FUNCTION_NOARGS();
  SimulationSingleton<GlobalRouteManagerImpl>::Get()->InitializeRoutes();
}

uint32_t GlobalRouteManager::AllocateRouterId() {
  NS_LOG_FUNCTION_NOARGS();
  static uint32_t routerId = 0;
  return routerId++;
}

} // namespace ns3
