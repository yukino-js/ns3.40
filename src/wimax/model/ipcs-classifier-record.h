
#ifndef IPCS_CLASSIFIER_RECORD_H
#define IPCS_CLASSIFIER_RECORD_H

#include "wimax-tlv.h"

#include "ns3/ipv4-address.h"

#include <stdint.h>

namespace ns3 {

class IpcsClassifierRecord {
public:
  IpcsClassifierRecord();
  ~IpcsClassifierRecord();
  IpcsClassifierRecord(Ipv4Address srcAddress, Ipv4Mask srcMask,
                       Ipv4Address dstAddress, Ipv4Mask dstMask,
                       uint16_t srcPortLow, uint16_t srcPortHigh,
                       uint16_t dstPortLow, uint16_t dstPortHigh,
                       uint8_t protocol, uint8_t priority);
  IpcsClassifierRecord(Tlv tlv);
  Tlv ToTlv() const;
  void AddSrcAddr(Ipv4Address srcAddress, Ipv4Mask srcMask);
  void AddDstAddr(Ipv4Address dstAddress, Ipv4Mask dstMask);
  void AddSrcPortRange(uint16_t srcPortLow, uint16_t srcPortHigh);
  void AddDstPortRange(uint16_t dstPortLow, uint16_t dstPortHigh);
  void AddProtocol(uint8_t proto);
  void SetPriority(uint8_t prio);
  void SetIndex(uint16_t index);
  bool CheckMatch(Ipv4Address srcAddress, Ipv4Address dstAddress,
                  uint16_t srcPort, uint16_t dstPort, uint8_t proto) const;
  uint16_t GetCid() const;
  uint8_t GetPriority() const;
  uint16_t GetIndex() const;
  void SetCid(uint16_t cid);

private:
  bool CheckMatchSrcAddr(Ipv4Address srcAddress) const;
  bool CheckMatchDstAddr(Ipv4Address dstAddress) const;
  bool CheckMatchSrcPort(uint16_t srcPort) const;
  bool CheckMatchDstPort(uint16_t dstPort) const;
  bool CheckMatchProtocol(uint8_t proto) const;

  struct PortRange {
    uint16_t PortLow;
    uint16_t PortHigh;
  };

  struct Ipv4Addr {
    Ipv4Address Address;
    Ipv4Mask Mask;
  };

  uint8_t m_priority;
  uint16_t m_index;
  uint8_t m_tosLow;
  uint8_t m_tosHigh;
  uint8_t m_tosMask;
  std::vector<uint8_t> m_protocol;
  std::vector<Ipv4Addr> m_srcAddr;
  std::vector<Ipv4Addr> m_dstAddr;
  std::vector<PortRange> m_srcPortRange;
  std::vector<PortRange> m_dstPortRange;

  uint16_t m_cid;
};
} // namespace ns3

#endif
