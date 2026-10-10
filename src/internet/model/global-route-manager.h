
#ifndef GLOBAL_ROUTE_MANAGER_H
#define GLOBAL_ROUTE_MANAGER_H

#include <cstdint>

namespace ns3 {

class GlobalRouteManager {
public:
  GlobalRouteManager(const GlobalRouteManager &) = delete;
  GlobalRouteManager &operator=(const GlobalRouteManager &) = delete;

  static uint32_t AllocateRouterId();

  static void DeleteGlobalRoutes();

  static void BuildGlobalRoutingDatabase();

  static void InitializeRoutes();
};

} // namespace ns3

#endif
