
#ifndef IPV6_ROUTING_HELPER_H
#define IPV6_ROUTING_HELPER_H

#include "ns3/ipv6-list-routing.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ptr.h"

namespace ns3 {

class Ipv6RoutingProtocol;
class Node;

class Ipv6RoutingHelper {
public:
  virtual ~Ipv6RoutingHelper();

  virtual Ipv6RoutingHelper *Copy() const = 0;

  virtual Ptr<Ipv6RoutingProtocol> Create(Ptr<Node> node) const = 0;

  static void PrintRoutingTableAllAt(Time printTime,
                                     Ptr<OutputStreamWrapper> stream,
                                     Time::Unit unit = Time::S);

  static void PrintRoutingTableAllEvery(Time printInterval,
                                        Ptr<OutputStreamWrapper> stream,
                                        Time::Unit unit = Time::S);

  static void PrintRoutingTableAt(Time printTime, Ptr<Node> node,
                                  Ptr<OutputStreamWrapper> stream,
                                  Time::Unit unit = Time::S);

  static void PrintRoutingTableEvery(Time printInterval, Ptr<Node> node,
                                     Ptr<OutputStreamWrapper> stream,
                                     Time::Unit unit = Time::S);

  static void PrintNeighborCacheAllAt(Time printTime,
                                      Ptr<OutputStreamWrapper> stream,
                                      Time::Unit unit = Time::S);

  static void PrintNeighborCacheAllEvery(Time printInterval,
                                         Ptr<OutputStreamWrapper> stream,
                                         Time::Unit unit = Time::S);

  static void PrintNeighborCacheAt(Time printTime, Ptr<Node> node,
                                   Ptr<OutputStreamWrapper> stream,
                                   Time::Unit unit = Time::S);

  static void PrintNeighborCacheEvery(Time printInterval, Ptr<Node> node,
                                      Ptr<OutputStreamWrapper> stream,
                                      Time::Unit unit = Time::S);

  template <class T>
  static Ptr<T> GetRouting(Ptr<Ipv6RoutingProtocol> protocol);

private:
  static void Print(Ptr<Node> node, Ptr<OutputStreamWrapper> stream,
                    Time::Unit unit);

  static void PrintEvery(Time printInterval, Ptr<Node> node,
                         Ptr<OutputStreamWrapper> stream, Time::Unit unit);

  static void PrintNdiscCache(Ptr<Node> node, Ptr<OutputStreamWrapper> stream,
                              Time::Unit unit = Time::S);

  static void PrintNdiscCacheEvery(Time printInterval, Ptr<Node> node,
                                   Ptr<OutputStreamWrapper> stream,
                                   Time::Unit unit = Time::S);
};

template <class T>
Ptr<T> Ipv6RoutingHelper::GetRouting(Ptr<Ipv6RoutingProtocol> protocol) {
  Ptr<T> ret = DynamicCast<T>(protocol);
  if (!ret) {
    Ptr<Ipv6ListRouting> lrp = DynamicCast<Ipv6ListRouting>(protocol);
    if (lrp) {
      for (uint32_t i = 0; i < lrp->GetNRoutingProtocols(); i++) {
        int16_t priority;
        ret = GetRouting<T>(lrp->GetRoutingProtocol(i, priority));
        if (ret) {
          break;
        }
      }
    }
  }

  return ret;
}

} // namespace ns3

#endif
