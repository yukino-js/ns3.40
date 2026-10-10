
#ifndef IPV6_PMTU_CACHE_H
#define IPV6_PMTU_CACHE_H

#include "ns3/event-id.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/type-id.h"

#include <map>

namespace ns3 {

class Ipv6PmtuCache : public Object {
public:
  class Entry;

  static TypeId GetTypeId();

  Ipv6PmtuCache();

  ~Ipv6PmtuCache() override;

  void DoDispose() override;

  uint32_t GetPmtu(Ipv6Address dst);

  void SetPmtu(Ipv6Address dst, uint32_t pmtu);

  Time GetPmtuValidityTime() const;

  bool SetPmtuValidityTime(Time validity);

private:
  void ClearPmtu(Ipv6Address dst);

  std::map<Ipv6Address, uint32_t> m_pathMtu;

  typedef std::map<Ipv6Address, EventId>::iterator pathMtuTimerIter;

  std::map<Ipv6Address, EventId> m_pathMtuTimer;

  Time m_validityTime;
};

} // namespace ns3

#endif
