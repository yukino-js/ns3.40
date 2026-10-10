
#ifndef DSR_RCACHE_H
#define DSR_RCACHE_H

#include "dsr-option-header.h"

#include "ns3/arp-cache.h"
#include "ns3/callback.h"
#include "ns3/enum.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"
#include "ns3/simulator.h"
#include "ns3/timer.h"

#include <cassert>
#include <iostream>
#include <map>
#include <stdint.h>
#include <sys/types.h>
#include <vector>

namespace ns3 {

class Time;
class WifiMacHeader;

namespace dsr {

struct Link {
  Ipv4Address m_low;
  Ipv4Address m_high;

  Link(Ipv4Address ip1, Ipv4Address ip2) {
    if (ip1 < ip2) {
      m_low = ip1;
      m_high = ip2;
    } else {
      m_low = ip2;
      m_high = ip1;
    }
  }

  bool operator<(const Link &L) const {
    if (m_low < L.m_low) {
      return true;
    } else if (m_low == L.m_low) {
      return (m_high < L.m_high);
    } else {
      return false;
    }
  }

  void Print() const;
};

class DsrLinkStab {
public:
  DsrLinkStab(Time linkStab = Simulator::Now());
  virtual ~DsrLinkStab();

  void SetLinkStability(Time linkStab) {
    m_linkStability = linkStab + Simulator::Now();
  }

  Time GetLinkStability() const { return m_linkStability - Simulator::Now(); }

  void Print() const;

private:
  Time m_linkStability;
};

class DsrNodeStab {
public:
  DsrNodeStab(Time nodeStab = Simulator::Now());
  virtual ~DsrNodeStab();

  void SetNodeStability(Time nodeStab) {
    m_nodeStability = nodeStab + Simulator::Now();
  }

  Time GetNodeStability() const { return m_nodeStability - Simulator::Now(); }

private:
  Time m_nodeStability;
};

class DsrRouteCacheEntry {
public:
  typedef std::vector<Ipv4Address> IP_VECTOR;
  typedef std::vector<Ipv4Address>::iterator Iterator;

  DsrRouteCacheEntry(IP_VECTOR const &ip = IP_VECTOR(),
                     Ipv4Address dst = Ipv4Address(),
                     Time exp = Simulator::Now());
  virtual ~DsrRouteCacheEntry();

  void Invalidate(Time badLinkLifetime);

  void SetUnidirectional(bool u) { m_blackListState = u; }

  bool IsUnidirectional() const { return m_blackListState; }

  void SetBlacklistTimeout(Time t) { m_blackListTimeout = t; }

  Time GetBlacklistTimeout() const { return m_blackListTimeout; }

  Ipv4Address GetDestination() const { return m_dst; }

  void SetDestination(Ipv4Address d) { m_dst = d; }

  IP_VECTOR GetVector() const { return m_path; }

  void SetVector(IP_VECTOR v) { m_path = v; }

  void SetExpireTime(Time exp) { m_expire = exp + Simulator::Now(); }

  Time GetExpireTime() const { return m_expire - Simulator::Now(); }

  void Print(std::ostream &os) const;

  bool operator==(const DsrRouteCacheEntry &o) const {
    if (m_path.size() != o.m_path.size()) {
      NS_ASSERT(false);
      return false;
    }
    auto j = o.m_path.begin();
    for (auto i = m_path.begin(); i != m_path.end(); i++, j++) {
      if (((*i) == nullptr) || ((*j) == nullptr)) {
        return false;
      } else if (!((*i) == (*j))) {
        return false;
      } else {
        return true;
      }
    }
    return false;
  }

private:
  Timer m_ackTimer;
  Ipv4Address m_dst;
  IP_VECTOR m_path;
  Time m_expire;
  Ipv4InterfaceAddress m_iface;
  uint8_t m_reqCount;
  bool m_blackListState;
  Time m_blackListTimeout;
  Ptr<Ipv4Route> m_ipv4Route;
  Ptr<Ipv4> m_ipv4;
};

class DsrRouteCache : public Object {
public:
  static TypeId GetTypeId();

  DsrRouteCache();
  ~DsrRouteCache() override;

  DsrRouteCache &operator=(const DsrRouteCache &) = delete;

  void RemoveLastEntry(std::list<DsrRouteCacheEntry> &rtVector);
  typedef std::list<DsrRouteCacheEntry::IP_VECTOR> routeVector;

  bool GetSubRoute() const { return m_subRoute; }

  void SetSubRoute(bool subRoute) { m_subRoute = subRoute; }

  uint32_t GetMaxCacheLen() const { return m_maxCacheLen; }

  void SetMaxCacheLen(uint32_t len) { m_maxCacheLen = len; }

  Time GetCacheTimeout() const { return RouteCacheTimeout; }

  void SetCacheTimeout(Time t) { RouteCacheTimeout = t; }

  uint32_t GetMaxEntriesEachDst() const { return m_maxEntriesEachDst; }

  void SetMaxEntriesEachDst(uint32_t entries) { m_maxEntriesEachDst = entries; }

  Time GetBadLinkLifetime() const { return m_badLinkLifetime; }

  void SetBadLinkLifetime(Time t) { m_badLinkLifetime = t; }

  uint64_t GetStabilityDecrFactor() const { return m_stabilityDecrFactor; }

  void SetStabilityDecrFactor(uint64_t decrFactor) {
    m_stabilityDecrFactor = decrFactor;
  }

  uint64_t GetStabilityIncrFactor() const { return m_stabilityIncrFactor; }

  void SetStabilityIncrFactor(uint64_t incrFactor) {
    m_stabilityIncrFactor = incrFactor;
  }

  Time GetInitStability() const { return m_initStability; }

  void SetInitStability(Time initStability) { m_initStability = initStability; }

  Time GetMinLifeTime() const { return m_minLifeTime; }

  void SetMinLifeTime(Time minLifeTime) { m_minLifeTime = minLifeTime; }

  Time GetUseExtends() const { return m_useExtends; }

  void SetUseExtends(Time useExtends) { m_useExtends = useExtends; }

  bool UpdateRouteEntry(Ipv4Address dst);
  bool AddRoute(DsrRouteCacheEntry &rt);
  bool LookupRoute(Ipv4Address id, DsrRouteCacheEntry &rt);
  void PrintVector(std::vector<Ipv4Address> &vec);
  void PrintRouteVector(std::list<DsrRouteCacheEntry> route);
  bool FindSameRoute(DsrRouteCacheEntry &rt,
                     std::list<DsrRouteCacheEntry> &rtVector);
  bool DeleteRoute(Ipv4Address dst);
  void DeleteAllRoutesIncludeLink(Ipv4Address errorSrc, Ipv4Address unreachNode,
                                  Ipv4Address node);

  void Clear() {
    m_routeEntryVector.erase(m_routeEntryVector.begin(),
                             m_routeEntryVector.end());
  }

  void Purge();
  void Print(std::ostream &os);

  uint16_t CheckUniqueAckId(Ipv4Address nextHop);
  uint16_t GetAckSize();

  struct Neighbor {
    Ipv4Address m_neighborAddress;
    Mac48Address m_hardwareAddress;
    Time m_expireTime;
    bool close;

    Neighbor(Ipv4Address ip, Mac48Address mac, Time t)
        : m_neighborAddress(ip), m_hardwareAddress(mac), m_expireTime(t),
          close(false) {}

    Neighbor() {}
  };

  Time GetExpireTime(Ipv4Address addr);
  bool IsNeighbor(Ipv4Address addr);
  void UpdateNeighbor(std::vector<Ipv4Address> nodeList, Time expire);
  void AddNeighbor(std::vector<Ipv4Address> nodeList, Ipv4Address ownAddress,
                   Time expire);
  void PurgeMac();
  void ScheduleTimer();

  void ClearMac() { m_nb.clear(); }

  void AddArpCache(Ptr<ArpCache> a);
  void DelArpCache(Ptr<ArpCache>);

  void SetCallback(Callback<void, Ipv4Address, uint8_t> cb) {
    m_handleLinkFailure = cb;
  }

  Callback<void, Ipv4Address, uint8_t> GetCallback() const {
    return m_handleLinkFailure;
  }

private:
  DsrRouteCacheEntry::IP_VECTOR m_vector;
  uint32_t m_maxCacheLen;
  Time RouteCacheTimeout;
  Time m_badLinkLifetime;
  uint32_t m_stabilityDecrFactor;
  uint32_t m_stabilityIncrFactor;
  Time m_initStability;
  Time m_minLifeTime;
  Time m_useExtends;
  typedef std::list<DsrRouteCacheEntry> routeEntryVector;

  std::map<Ipv4Address, routeEntryVector> m_sortedRoutes;

  routeEntryVector m_routeEntryVector;

  uint32_t m_maxEntriesEachDst;

  std::map<Ipv4Address, uint16_t> m_ackIdCache;

  bool m_isLinkCache;

  bool m_subRoute;
#define MAXWEIGHT 0xFFFF;
  std::map<Ipv4Address, std::map<Ipv4Address, uint32_t>> m_netGraph;

  std::map<Ipv4Address, DsrRouteCacheEntry::IP_VECTOR> m_bestRoutesTable_link;
  std::map<Link, DsrLinkStab> m_linkCache;
  std::map<Ipv4Address, DsrNodeStab> m_nodeCache;
  bool LookupRoute_Link(Ipv4Address id, DsrRouteCacheEntry &rt);
  bool IncStability(Ipv4Address node);
  bool DecStability(Ipv4Address node);

public:
  void SetCacheType(std::string type);
  bool IsLinkCache();
  bool AddRoute_Link(DsrRouteCacheEntry::IP_VECTOR nodelist, Ipv4Address node);
  void RebuildBestRouteTable(Ipv4Address source);
  void PurgeLinkNode();
  void UseExtends(DsrRouteCacheEntry::IP_VECTOR rt);
  void UpdateNetGraph();
  Callback<void, Ipv4Address, uint8_t> m_handleLinkFailure;

  Timer m_ntimer;

  std::vector<Neighbor> m_nb;

  std::vector<Ptr<ArpCache>> m_arp;

  Time m_delay;

  Mac48Address LookupMacAddress(Ipv4Address addr);

  void ProcessTxError(const WifiMacHeader &hdr);
};
} // namespace dsr
} // namespace ns3
#endif
