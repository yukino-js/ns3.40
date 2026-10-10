
#ifndef FLAME_RTABLE_H
#define FLAME_RTABLE_H

#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

#include <map>

namespace ns3 {
namespace flame {
class FlameRtable : public Object {
public:
  const static uint32_t INTERFACE_ANY = 0xffffffff;
  const static uint32_t MAX_COST = 0xff;

  struct LookupResult {
    Mac48Address retransmitter;
    uint32_t ifIndex;
    uint8_t cost;
    uint16_t seqnum;

    LookupResult(Mac48Address r = Mac48Address::GetBroadcast(),
                 uint32_t i = INTERFACE_ANY, uint8_t c = MAX_COST,
                 uint16_t s = 0)
        : retransmitter(r), ifIndex(i), cost(c), seqnum(s) {}

    bool IsValid() const;
    bool operator==(const LookupResult &o) const;
  };

public:
  static TypeId GetTypeId();

  FlameRtable();
  ~FlameRtable() override;

  FlameRtable(const FlameRtable &) = delete;
  FlameRtable &operator=(const FlameRtable &) = delete;

  void DoDispose() override;

  void AddPath(const Mac48Address destination, const Mac48Address retransmitter,
               const uint32_t interface, const uint8_t cost,
               const uint16_t seqnum);
  LookupResult Lookup(Mac48Address destination);

private:
  struct Route {
    Mac48Address retransmitter;
    uint32_t interface;
    uint32_t cost;
    Time whenExpire;
    uint32_t seqnum;
  };

  Time m_lifetime;
  std::map<Mac48Address, Route> m_routes;
};

} // namespace flame
} // namespace ns3
#endif
