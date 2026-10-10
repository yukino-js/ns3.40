
#ifndef DSR_RREQ_TABLE_H
#define DSR_RREQ_TABLE_H

#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include "ns3/timer.h"

#include <list>
#include <map>
#include <vector>

namespace ns3 {
namespace dsr {

enum LinkStates {
  PROBABLE = 0,
  QUESTIONABLE = 1,
};

struct BlackList {
  Ipv4Address m_neighborAddress;
  Time m_expireTime;
  LinkStates m_linkStates;

  BlackList(Ipv4Address ip, Time t)
      : m_neighborAddress(ip), m_expireTime(t), m_linkStates(PROBABLE) {}
};

struct RreqTableEntry {
  uint32_t m_reqNo;
  Time m_expire;
};

class DsrReceivedRreqEntry {
public:
  DsrReceivedRreqEntry(Ipv4Address d = Ipv4Address(), uint16_t i = 0)
      : m_destination(d), m_identification(i) {}

  bool operator==(const DsrReceivedRreqEntry &o) const {
    return ((m_destination == o.m_destination) &&
            (m_identification == o.m_identification));
  }

  Ipv4Address GetDestination() const { return m_destination; }

  void SetDestination(Ipv4Address d) { m_destination = d; }

  Ipv4Address GetSource() const { return m_source; }

  void SetSource(Ipv4Address s) { m_source = s; }

  uint16_t GetIdentification() const { return m_identification; }

  void SetIdentification(uint16_t i) { m_identification = i; }

  void SetExpireTime(Time exp) { m_expire = exp + Simulator::Now(); }

  Time GetExpireTime() const { return m_expire - Simulator::Now(); }

private:
  Ipv4Address m_destination;
  Ipv4Address m_source;
  uint16_t m_identification;
  Time m_expire;
};

class DsrRreqTable : public Object {
public:
  static TypeId GetTypeId();

  DsrRreqTable();
  ~DsrRreqTable() override;

  void SetInitHopLimit(uint32_t hl) { m_initHopLimit = hl; }

  uint32_t GetInitHopLimit() const { return m_initHopLimit; }

  void SetRreqTableSize(uint32_t rt) { m_requestTableSize = rt; }

  uint32_t GetRreqTableSize() const { return m_requestTableSize; }

  void SetRreqIdSize(uint32_t id) { m_requestIdSize = id; }

  uint32_t GetRreqIdSize() const { return m_requestIdSize; }

  void SetUniqueRreqIdSize(uint32_t uid) { m_maxRreqId = uid; }

  uint32_t GetUniqueRreqIdSize() const { return m_maxRreqId; }

  void RemoveLeastExpire();
  void FindAndUpdate(Ipv4Address dst);
  void RemoveRreqEntry(Ipv4Address dst);
  uint32_t GetRreqCnt(Ipv4Address dst);

  uint32_t CheckUniqueRreqId(Ipv4Address dst);
  uint32_t GetRreqSize();

  void Invalidate();
  BlackList *FindUnidirectional(Ipv4Address neighbor);
  bool MarkLinkAsUnidirectional(Ipv4Address neighbor, Time blacklistTimeout);
  void PurgeNeighbor();

  bool FindSourceEntry(Ipv4Address src, Ipv4Address dst, uint16_t id);

private:
  Time MaxRequestPeriod;
  Time RequestPeriod;
  Time NonpropRequestTimeout;
  Time m_rreqEntryExpire;
  uint32_t m_initHopLimit;
  uint32_t m_requestTableSize;
  uint32_t m_requestIdSize;
  uint32_t m_maxRreqId;
  LinkStates m_linkStates;
  std::list<DsrReceivedRreqEntry> m_sourceRequests;
  std::map<Ipv4Address, uint32_t> m_rreqIdCache;
  std::map<Ipv4Address, RreqTableEntry> m_rreqDstMap;
  std::map<Ipv4Address, std::list<DsrReceivedRreqEntry>> m_sourceRreqMap;

  std::vector<BlackList> m_blackList;

  struct IsExpired {
    bool operator()(const BlackList &b) const {
      return (b.m_expireTime < Simulator::Now());
    }
  };
};
} // namespace dsr
} // namespace ns3

#endif
