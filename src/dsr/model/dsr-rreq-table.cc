
#include "dsr-rreq-table.h"

#include "ns3/log.h"

#include <algorithm>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrRreqTable");

namespace dsr {

NS_OBJECT_ENSURE_REGISTERED(DsrRreqTable);

TypeId DsrRreqTable::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrRreqTable")
                          .SetParent<Object>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrRreqTable>();
  return tid;
}

DsrRreqTable::DsrRreqTable() : m_linkStates(PROBABLE) {}

DsrRreqTable::~DsrRreqTable() { NS_LOG_FUNCTION_NOARGS(); }

void DsrRreqTable::RemoveLeastExpire() {
  NS_LOG_FUNCTION(this);
  Ipv4Address firstExpire;
  Time max = Seconds(0.0);
  for (auto i = m_rreqDstMap.begin(); i != m_rreqDstMap.end(); ++i) {
    Ipv4Address dst = i->first;
    RreqTableEntry rreqTableEntry = i->second;
    if (rreqTableEntry.m_expire > max) {
      max = rreqTableEntry.m_expire;
      firstExpire = dst;
    }
  }
  m_rreqDstMap.erase(firstExpire);
}

void DsrRreqTable::FindAndUpdate(Ipv4Address dst) {
  NS_LOG_FUNCTION(this << dst);
  auto i = m_rreqDstMap.find(dst);
  if (i == m_rreqDstMap.end()) {
    NS_LOG_LOGIC("The request table entry for " << dst << " not found");
    if (m_rreqDstMap.size() >= m_requestTableSize) {
      RemoveLeastExpire();
      NS_LOG_INFO("The request table size after erase "
                  << (uint32_t)m_rreqDstMap.size());
    }
    RreqTableEntry rreqTableEntry;
    rreqTableEntry.m_reqNo = 1;
    rreqTableEntry.m_expire = Simulator::Now();
    m_rreqDstMap[dst] = rreqTableEntry;
  } else {
    NS_LOG_LOGIC("Find the request table entry for  "
                 << dst << ", increment the request count");
    Ipv4Address dst = i->first;
    RreqTableEntry rreqTableEntry = i->second;
    rreqTableEntry.m_reqNo = rreqTableEntry.m_reqNo + 1;
    rreqTableEntry.m_expire = Simulator::Now();
    m_rreqDstMap[dst] = rreqTableEntry;
  }
}

void DsrRreqTable::RemoveRreqEntry(Ipv4Address dst) {
  NS_LOG_FUNCTION(this << dst);
  auto i = m_rreqDstMap.find(dst);
  if (i == m_rreqDstMap.end()) {
    NS_LOG_LOGIC("The request table entry not found");
  } else {
    m_rreqDstMap.erase(dst);
  }
}

uint32_t DsrRreqTable::GetRreqCnt(Ipv4Address dst) {
  NS_LOG_FUNCTION(this << dst);
  auto i = m_rreqDstMap.find(dst);
  if (i == m_rreqDstMap.end()) {
    NS_LOG_LOGIC("Request table entry not found");
    return 0;
  }

  RreqTableEntry rreqTableEntry = i->second;
  return rreqTableEntry.m_reqNo;
}

uint32_t DsrRreqTable::CheckUniqueRreqId(Ipv4Address dst) {
  NS_LOG_LOGIC("The size of id cache " << m_rreqIdCache.size());
  auto i = m_rreqIdCache.find(dst);
  if (i == m_rreqIdCache.end()) {
    NS_LOG_LOGIC("No Request id for " << dst << " found, initialize it to 0");
    m_rreqIdCache[dst] = 0;
    return 0;
  }

  NS_LOG_LOGIC("Request id for " << dst << " found in the cache");
  uint32_t rreqId = m_rreqIdCache[dst];
  if (rreqId >= m_maxRreqId) {
    NS_LOG_DEBUG("The request id increase past the max value, "
                 << m_maxRreqId << " so reset it to 0");
    rreqId = 0;
    m_rreqIdCache[dst] = rreqId;
  } else {
    rreqId++;
    m_rreqIdCache[dst] = rreqId;
  }
  NS_LOG_INFO("The Request id for " << dst << " is " << rreqId);
  return rreqId;
}

uint32_t DsrRreqTable::GetRreqSize() { return m_rreqIdCache.size(); }

void DsrRreqTable::Invalidate() {
  if (m_linkStates == QUESTIONABLE) {
    return;
  }
  m_linkStates = QUESTIONABLE;
}

BlackList *DsrRreqTable::FindUnidirectional(Ipv4Address neighbor) {
  PurgeNeighbor();
  for (auto i = m_blackList.begin(); i != m_blackList.end(); ++i) {
    if (i->m_neighborAddress == neighbor) {
      return &(*i);
    }
  }
  return nullptr;
}

bool DsrRreqTable::MarkLinkAsUnidirectional(Ipv4Address neighbor,
                                            Time blacklistTimeout) {
  NS_LOG_LOGIC("Add neighbor address in blacklist " << m_blackList.size());
  for (auto i = m_blackList.begin(); i != m_blackList.end(); i++) {
    if (i->m_neighborAddress == neighbor) {
      NS_LOG_DEBUG(
          "Update the blacklist list timeout if found the blacklist entry");
      i->m_expireTime =
          std::max(blacklistTimeout + Simulator::Now(), i->m_expireTime);
    }
    BlackList blackList(neighbor, blacklistTimeout + Simulator::Now());
    m_blackList.push_back(blackList);
    PurgeNeighbor();
    return true;
  }
  return false;
}

void DsrRreqTable::PurgeNeighbor() {
  m_blackList.erase(
      remove_if(m_blackList.begin(), m_blackList.end(), IsExpired()),
      m_blackList.end());
}

bool DsrRreqTable::FindSourceEntry(Ipv4Address src, Ipv4Address dst,
                                   uint16_t id) {
  NS_LOG_FUNCTION(this << src << dst << id);
  DsrReceivedRreqEntry rreqEntry;
  rreqEntry.SetDestination(dst);
  rreqEntry.SetIdentification(id);
  std::list<DsrReceivedRreqEntry> receivedRreqEntryList;
  auto i = m_sourceRreqMap.find(src);
  if (i == m_sourceRreqMap.end()) {
    NS_LOG_LOGIC("The source request table entry for " << src << " not found");

    receivedRreqEntryList.clear();
    receivedRreqEntryList.push_back(rreqEntry);

    m_sourceRreqMap[src] = receivedRreqEntryList;
    return false;
  }

  NS_LOG_LOGIC("Find the request table entry for  "
               << src << ", check if it is exact duplicate");
  receivedRreqEntryList = i->second;
  if (receivedRreqEntryList.size() >= m_requestIdSize) {
    receivedRreqEntryList.pop_front();
  }

  for (auto j = receivedRreqEntryList.begin(); j != receivedRreqEntryList.end();
       ++j) {
    if (*j == rreqEntry) {
      return true;
    }
  }
  receivedRreqEntryList.push_back(rreqEntry);
  m_sourceRreqMap[src] = receivedRreqEntryList;
  return false;
}

} // namespace dsr
} // namespace ns3
