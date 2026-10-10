
#ifndef AODV_ID_CACHE_H
#define AODV_ID_CACHE_H

#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3 {
namespace aodv {
class IdCache {
public:
  IdCache(Time lifetime) : m_lifetime(lifetime) {}

  bool IsDuplicate(Ipv4Address addr, uint32_t id);
  void Purge();
  uint32_t GetSize();

  void SetLifetime(Time lifetime) { m_lifetime = lifetime; }

  Time GetLifeTime() const { return m_lifetime; }

private:
  struct UniqueId {
    Ipv4Address m_context;
    uint32_t m_id;
    Time m_expire;
  };

  struct IsExpired {
    bool operator()(const UniqueId &u) const {
      return (u.m_expire < Simulator::Now());
    }
  };

  std::vector<UniqueId> m_idCache;
  Time m_lifetime;
};

} // namespace aodv
} // namespace ns3

#endif
