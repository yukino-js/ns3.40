
#ifndef WIFI_PSDU_H
#define WIFI_PSDU_H

#include "wifi-mac-header.h"
#include "wifi-mpdu.h"

#include "ns3/nstime.h"

#include <set>
#include <vector>

namespace ns3 {

class Packet;

class WifiPsdu : public SimpleRefCount<WifiPsdu> {
public:
  WifiPsdu(Ptr<const Packet> p, const WifiMacHeader &header);

  WifiPsdu(Ptr<WifiMpdu> mpdu, bool isSingle);

  WifiPsdu(Ptr<const WifiMpdu> mpdu, bool isSingle);

  WifiPsdu(std::vector<Ptr<WifiMpdu>> mpduList);

  virtual ~WifiPsdu();

  bool IsSingle() const;

  bool IsAggregate() const;

  Ptr<const Packet> GetPacket() const;

  const WifiMacHeader &GetHeader(std::size_t i) const;

  WifiMacHeader &GetHeader(std::size_t i);

  Ptr<const Packet> GetPayload(std::size_t i) const;

  Ptr<Packet> GetAmpduSubframe(std::size_t i) const;

  std::size_t GetAmpduSubframeSize(std::size_t i) const;

  Mac48Address GetAddr1() const;

  Mac48Address GetAddr2() const;

  bool HasNav() const;

  Time GetDuration() const;

  void SetDuration(Time duration);

  std::set<uint8_t> GetTids() const;

  WifiMacHeader::QosAckPolicy GetAckPolicyForTid(uint8_t tid) const;

  void SetAckPolicyForTid(uint8_t tid, WifiMacHeader::QosAckPolicy policy);

  uint16_t GetMaxDistFromStartingSeq(uint16_t startingSeq) const;

  uint32_t GetSize() const;

  std::size_t GetNMpdus() const;

  std::vector<Ptr<WifiMpdu>>::const_iterator begin() const;

  std::vector<Ptr<WifiMpdu>>::iterator begin();

  std::vector<Ptr<WifiMpdu>>::const_iterator end() const;

  std::vector<Ptr<WifiMpdu>>::iterator end();

  void Print(std::ostream &os) const;

private:
  bool m_isSingle;
  std::vector<Ptr<WifiMpdu>> m_mpduList;
  uint32_t m_size;
};

std::ostream &operator<<(std::ostream &os, const WifiPsdu &psdu);

} // namespace ns3

#endif
