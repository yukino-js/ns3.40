#ifndef IPV4_GLOBAL_ROUTING_HELPER_H
#define IPV4_GLOBAL_ROUTING_HELPER_H

#include "ipv4-routing-helper.h"

#include "ns3/node-container.h"

namespace ns3 {

class Ipv4GlobalRoutingHelper : public Ipv4RoutingHelper {
public:
  Ipv4GlobalRoutingHelper();

  Ipv4GlobalRoutingHelper(const Ipv4GlobalRoutingHelper &o);

  Ipv4GlobalRoutingHelper &operator=(const Ipv4GlobalRoutingHelper &) = delete;

  Ipv4GlobalRoutingHelper *Copy() const override;

  Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

  static void PopulateRoutingTables();
  static void RecomputeRoutingTables();
};

} // namespace ns3

#endif
