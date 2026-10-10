#ifndef IPV4_LIST_ROUTING_HELPER_H
#define IPV4_LIST_ROUTING_HELPER_H

#include "ipv4-routing-helper.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class Ipv4ListRoutingHelper : public Ipv4RoutingHelper {
public:
  Ipv4ListRoutingHelper();

  ~Ipv4ListRoutingHelper() override;

  Ipv4ListRoutingHelper(const Ipv4ListRoutingHelper &o);

  Ipv4ListRoutingHelper &operator=(const Ipv4ListRoutingHelper &) = delete;

  Ipv4ListRoutingHelper *Copy() const override;

  void Add(const Ipv4RoutingHelper &routing, int16_t priority);
  Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

private:
  std::list<std::pair<const Ipv4RoutingHelper *, int16_t>> m_list;
};

} // namespace ns3

#endif
