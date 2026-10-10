#ifndef IPV6_LIST_ROUTING_HELPER_H
#define IPV6_LIST_ROUTING_HELPER_H

#include "ipv6-routing-helper.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class Ipv6ListRoutingHelper : public Ipv6RoutingHelper {
public:
  Ipv6ListRoutingHelper();

  ~Ipv6ListRoutingHelper() override;

  Ipv6ListRoutingHelper(const Ipv6ListRoutingHelper &o);

  Ipv6ListRoutingHelper &operator=(const Ipv6ListRoutingHelper &) = delete;

  Ipv6ListRoutingHelper *Copy() const override;

  void Add(const Ipv6RoutingHelper &routing, int16_t priority);
  Ptr<Ipv6RoutingProtocol> Create(Ptr<Node> node) const override;

private:
  std::list<std::pair<const Ipv6RoutingHelper *, int16_t>> m_list;
};

} // namespace ns3

#endif
