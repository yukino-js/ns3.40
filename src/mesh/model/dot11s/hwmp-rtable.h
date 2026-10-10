
#ifndef HWMP_RTABLE_H
#define HWMP_RTABLE_H

#include "hwmp-protocol.h"

#include "ns3/mac48-address.h"
#include "ns3/nstime.h"

#include <map>

namespace ns3 {
namespace dot11s {
class HwmpRtable : public Object {
public:
  const static uint32_t INTERFACE_ANY = 0xffffffff;
  const static uint32_t MAX_METRIC = 0xffffffff;

  struct LookupResult {
    Mac48Address retransmitter;
    uint32_t ifIndex;
    uint32_t metric;
    uint32_t seqnum;
    Time lifetime;
    LookupResult(Mac48Address r = Mac48Address::GetBroadcast(),
                 uint32_t i = INTERFACE_ANY, uint32_t m = MAX_METRIC,
                 uint32_t s = 0, Time l = Seconds(0.0));
    bool IsValid() const;
    bool operator==(const LookupResult &o) const;
  };

  typedef std::vector<std::pair<uint32_t, Mac48Address>> PrecursorList;

public:
  static TypeId GetTypeId();
  HwmpRtable();
  ~HwmpRtable() override;
  void DoDispose() override;

  void AddReactivePath(Mac48Address destination, Mac48Address retransmitter,
                       uint32_t interface, uint32_t metric, Time lifetime,
                       uint32_t seqnum);
  void AddProactivePath(uint32_t metric, Mac48Address root,
                        Mac48Address retransmitter, uint32_t interface,
                        Time lifetime, uint32_t seqnum);
  void AddPrecursor(Mac48Address destination, uint32_t precursorInterface,
                    Mac48Address precursorAddress, Time lifetime);

  PrecursorList GetPrecursors(Mac48Address destination);

  void DeleteProactivePath();
  void DeleteProactivePath(Mac48Address root);
  void DeleteReactivePath(Mac48Address destination);

  LookupResult LookupReactive(Mac48Address destination);
  LookupResult LookupReactiveExpired(Mac48Address destination);
  LookupResult LookupProactive();
  LookupResult LookupProactiveExpired();

  std::vector<HwmpProtocol::FailedDestination>
  GetUnreachableDestinations(Mac48Address peerAddress);

private:
  struct Precursor {
    Mac48Address address;
    uint32_t interface;
    Time whenExpire;
  };

  struct ReactiveRoute {
    Mac48Address retransmitter;
    uint32_t interface;
    uint32_t metric;
    Time whenExpire;
    uint32_t seqnum;
    std::vector<Precursor> precursors;
  };

  struct ProactiveRoute {
    Mac48Address root;
    Mac48Address retransmitter;
    uint32_t interface;
    uint32_t metric;
    Time whenExpire;
    uint32_t seqnum;
    std::vector<Precursor> precursors;
  };

  std::map<Mac48Address, ReactiveRoute> m_routes;
  ProactiveRoute m_root;
};
} // namespace dot11s
} // namespace ns3
#endif
