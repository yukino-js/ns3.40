#ifndef AODV_RTABLE_H
#define AODV_RTABLE_H

#include "ns3/ipv4-route.h"
#include "ns3/ipv4.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/timer.h"

#include <cassert>
#include <map>
#include <stdint.h>
#include <sys/types.h>

namespace ns3 {
namespace aodv {

enum RouteFlags {
  VALID = 0,
  INVALID = 1,
  IN_SEARCH = 2,
};

class RoutingTableEntry {
public:
  RoutingTableEntry(Ptr<NetDevice> dev = nullptr,
                    Ipv4Address dst = Ipv4Address(), bool vSeqNo = false,
                    uint32_t seqNo = 0,
                    Ipv4InterfaceAddress iface = Ipv4InterfaceAddress(),
                    uint16_t hops = 0, Ipv4Address nextHop = Ipv4Address(),
                    Time lifetime = Simulator::Now());

  ~RoutingTableEntry();

  bool InsertPrecursor(Ipv4Address id);
  bool LookupPrecursor(Ipv4Address id);
  bool DeletePrecursor(Ipv4Address id);
  void DeleteAllPrecursors();
  bool IsPrecursorListEmpty() const;
  void GetPrecursors(std::vector<Ipv4Address> &prec) const;

  void Invalidate(Time badLinkLifetime);

  Ipv4Address GetDestination() const { return m_ipv4Route->GetDestination(); }

  Ptr<Ipv4Route> GetRoute() const { return m_ipv4Route; }

  void SetRoute(Ptr<Ipv4Route> r) { m_ipv4Route = r; }

  void SetNextHop(Ipv4Address nextHop) { m_ipv4Route->SetGateway(nextHop); }

  Ipv4Address GetNextHop() const { return m_ipv4Route->GetGateway(); }

  void SetOutputDevice(Ptr<NetDevice> dev) {
    m_ipv4Route->SetOutputDevice(dev);
  }

  Ptr<NetDevice> GetOutputDevice() const {
    return m_ipv4Route->GetOutputDevice();
  }

  Ipv4InterfaceAddress GetInterface() const { return m_iface; }

  void SetInterface(Ipv4InterfaceAddress iface) { m_iface = iface; }

  void SetValidSeqNo(bool s) { m_validSeqNo = s; }

  bool GetValidSeqNo() const { return m_validSeqNo; }

  void SetSeqNo(uint32_t sn) { m_seqNo = sn; }

  uint32_t GetSeqNo() const { return m_seqNo; }

  void SetHop(uint16_t hop) { m_hops = hop; }

  uint16_t GetHop() const { return m_hops; }

  void SetLifeTime(Time lt) { m_lifeTime = lt + Simulator::Now(); }

  Time GetLifeTime() const { return m_lifeTime - Simulator::Now(); }

  void SetFlag(RouteFlags flag) { m_flag = flag; }

  RouteFlags GetFlag() const { return m_flag; }

  void SetRreqCnt(uint8_t n) { m_reqCount = n; }

  uint8_t GetRreqCnt() const { return m_reqCount; }

  void IncrementRreqCnt() { m_reqCount++; }

  void SetUnidirectional(bool u) { m_blackListState = u; }

  bool IsUnidirectional() const { return m_blackListState; }

  void SetBlacklistTimeout(Time t) { m_blackListTimeout = t; }

  Time GetBlacklistTimeout() const { return m_blackListTimeout; }

  Timer m_ackTimer;

  bool operator==(const Ipv4Address dst) const {
    return (m_ipv4Route->GetDestination() == dst);
  }

  void Print(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

private:
  bool m_validSeqNo;
  uint32_t m_seqNo;
  uint16_t m_hops;
  Time m_lifeTime;
  Ptr<Ipv4Route> m_ipv4Route;
  Ipv4InterfaceAddress m_iface;
  RouteFlags m_flag;

  std::vector<Ipv4Address> m_precursorList;
  Time m_routeRequestTimeout;
  uint8_t m_reqCount;
  bool m_blackListState;
  Time m_blackListTimeout;
};

class RoutingTable {
public:
  RoutingTable(Time t);

  Time GetBadLinkLifetime() const { return m_badLinkLifetime; }

  void SetBadLinkLifetime(Time t) { m_badLinkLifetime = t; }

  bool AddRoute(RoutingTableEntry &r);
  bool DeleteRoute(Ipv4Address dst);
  bool LookupRoute(Ipv4Address dst, RoutingTableEntry &rt);
  bool LookupValidRoute(Ipv4Address dst, RoutingTableEntry &rt);
  bool Update(RoutingTableEntry &rt);
  bool SetEntryState(Ipv4Address dst, RouteFlags state);
  void
  GetListOfDestinationWithNextHop(Ipv4Address nextHop,
                                  std::map<Ipv4Address, uint32_t> &unreachable);
  void
  InvalidateRoutesWithDst(const std::map<Ipv4Address, uint32_t> &unreachable);
  void DeleteAllRoutesFromInterface(Ipv4InterfaceAddress iface);

  void Clear() { m_ipv4AddressEntry.clear(); }

  void Purge();
  bool MarkLinkAsUnidirectional(Ipv4Address neighbor, Time blacklistTimeout);
  void Print(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

private:
  std::map<Ipv4Address, RoutingTableEntry> m_ipv4AddressEntry;
  Time m_badLinkLifetime;
  void Purge(std::map<Ipv4Address, RoutingTableEntry> &table) const;
};

} // namespace aodv
} // namespace ns3

#endif
