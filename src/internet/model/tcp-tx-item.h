#ifndef TCP_TX_ITEM_H
#define TCP_TX_ITEM_H

#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/sequence-number.h"

namespace ns3 {
class TcpTxItem {
public:
  void Print(std::ostream &os, Time::Unit unit = Time::S) const;

  uint32_t GetSeqSize() const;

  bool IsSacked() const;

  bool IsRetrans() const;

  Ptr<Packet> GetPacketCopy() const;

  Ptr<const Packet> GetPacket() const;

  const Time &GetLastSent() const;

  struct RateInformation {
    uint64_t m_delivered{0};
    Time m_deliveredTime{Time::Max()};
    Time m_firstSent{Time::Max()};
    bool m_isAppLimited{false};
  };

  RateInformation &GetRateInformation();

  bool m_retrans{false};

private:
  friend class TcpTxBuffer;

  SequenceNumber32 m_startSeq{0};
  Ptr<Packet> m_packet{nullptr};
  bool m_lost{false};
  Time m_lastSent{Time::Max()};
  bool m_sacked{false};

  RateInformation m_rateInfo;
};

} // namespace ns3

#endif
