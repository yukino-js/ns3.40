
#ifndef IPV4_ROUTING_HELPER_H
#define IPV4_ROUTING_HELPER_H

#include "ns3/ipv4-list-routing.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ptr.h"

namespace ns3 {

class Ipv4RoutingProtocol;
class Node;

class Ipv4RoutingHelper {
public:
  virtual ~Ipv4RoutingHelper();

  virtual Ipv4RoutingHelper *Copy() const = 0;

  virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const = 0;

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
  static Ptr<T> GetRouting(Ptr<Ipv4RoutingProtocol> protocol);

private:
  static void Print(Ptr<Node> node, Ptr<OutputStreamWrapper> stream,
                    Time::Unit unit = Time::S);

  static void PrintEvery(Time printInterval, Ptr<Node> node,
                         Ptr<OutputStreamWrapper> stream,
                         Time::Unit unit = Time::S);

  static void PrintArpCache(Ptr<Node> node, Ptr<OutputStreamWrapper> stream,
                            Time::Unit unit = Time::S);

  static void PrintArpCacheEvery(Time printInterval, Ptr<Node> node,
                                 Ptr<OutputStreamWrapper> stream,
                                 Time::Unit unit = Time::S);
};

template <class T>
Ptr<T> Ipv4RoutingHelper::GetRouting(Ptr<Ipv4RoutingProtocol> protocol) {
  Ptr<T> ret = DynamicCast<T>(protocol);
  if (!ret) {
    Ptr<Ipv4ListRouting> lrp = DynamicCast<Ipv4ListRouting>(protocol);
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
