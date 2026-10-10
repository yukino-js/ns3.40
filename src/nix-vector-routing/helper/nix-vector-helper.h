
#ifndef NIX_VECTOR_HELPER_H
#define NIX_VECTOR_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/ipv6-routing-helper.h"
#include "ns3/object-factory.h"

namespace ns3 {

template <typename T>
class NixVectorHelper
    : public std::enable_if_t<std::is_same_v<Ipv4RoutingHelper, T> ||
                                  std::is_same_v<Ipv6RoutingHelper, T>,
                              T> {
  static constexpr bool IsIpv4 = std::is_same_v<Ipv4RoutingHelper, T>;
  using Ip = typename std::conditional_t<IsIpv4, Ipv4, Ipv6>;
  using IpAddress =
      typename std::conditional_t<IsIpv4, Ipv4Address, Ipv6Address>;
  using IpRoutingProtocol =
      typename std::conditional_t<IsIpv4, Ipv4RoutingProtocol,
                                  Ipv6RoutingProtocol>;

public:
  NixVectorHelper();

  NixVectorHelper(const NixVectorHelper<T> &o);

  NixVectorHelper &operator=(const NixVectorHelper &) = delete;

  NixVectorHelper<T> *Copy() const override;

  Ptr<IpRoutingProtocol> Create(Ptr<Node> node) const override;

  void PrintRoutingPathAt(Time printTime, Ptr<Node> source, IpAddress dest,
                          Ptr<OutputStreamWrapper> stream,
                          Time::Unit unit = Time::S);

private:
  ObjectFactory m_agentFactory;

  static void PrintRoute(Ptr<Node> source, IpAddress dest,
                         Ptr<OutputStreamWrapper> stream,
                         Time::Unit unit = Time::S);
};

typedef NixVectorHelper<Ipv4RoutingHelper> Ipv4NixVectorHelper;

typedef NixVectorHelper<Ipv6RoutingHelper> Ipv6NixVectorHelper;
} // namespace ns3

#endif
