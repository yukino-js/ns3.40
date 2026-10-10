
#ifndef WIFI_MPDU_H
#define WIFI_MPDU_H

#include "amsdu-subframe-header.h"
#include "wifi-mac-header.h"
#include "wifi-mac-queue-elem.h"

#include "ns3/packet.h"
#include "ns3/simulator.h"

#include <list>
#include <optional>
#include <set>
#include <variant>

namespace ns3 {

class WmqIteratorTag {
  friend class WifiMacQueue;
  WmqIteratorTag() = default;
};

class WifiMpdu : public SimpleRefCount<WifiMpdu> {
public:
  WifiMpdu(Ptr<const Packet> p, const WifiMacHeader &header,
           Time stamp = Simulator::Now());

  virtual ~WifiMpdu();

  bool IsOriginal() const;

  Ptr<const WifiMpdu> GetOriginal() const;

  Ptr<const Packet> GetPacket() const;

  const WifiMacHeader &GetHeader() const;

  WifiMacHeader &GetHeader();

  Mac48Address GetDestinationAddress() const;

  uint32_t GetSize() const;

  uint32_t GetPacketSize() const;

  bool IsFragment() const;

  void Aggregate(Ptr<const WifiMpdu> msdu);

  typedef std::list<std::pair<Ptr<const Packet>, AmsduSubframeHeader>>
      DeaggregatedMsdus;
  typedef std::list<std::pair<Ptr<const Packet>, AmsduSubframeHeader>>::
      const_iterator DeaggregatedMsdusCI;

  DeaggregatedMsdusCI begin() const;
  DeaggregatedMsdusCI end() const;

  typedef std::list<WifiMacQueueElem>::iterator Iterator;

  void SetQueueIt(std::optional<Iterator> queueIt, WmqIteratorTag tag);
  Iterator GetQueueIt(WmqIteratorTag tag) const;

  bool IsQueued() const;
  AcIndex GetQueueAc() const;
  Time GetTimestamp() const;
  Time GetExpiryTime() const;

  Ptr<Packet> GetProtocolDataUnit() const;

  void SetInFlight(uint8_t linkId) const;
  void ResetInFlight(uint8_t linkId) const;
  std::set<uint8_t> GetInFlightLinkIds() const;
  bool IsInFlight() const;

  void AssignSeqNo(uint16_t seqNo);
  bool HasSeqNoAssigned() const;
  void UnassignSeqNo();

  Ptr<WifiMpdu> CreateAlias(uint8_t linkId) const;

  virtual void Print(std::ostream &os) const;

private:
  void DoAggregate(Ptr<const WifiMpdu> msdu);

  Iterator GetQueueIt() const;

  WifiMpdu() = default;

  WifiMacHeader m_header;

  struct OriginalInfo {
    Ptr<const Packet> m_packet;
    Time m_timestamp;
    DeaggregatedMsdus m_msduList;
    std::optional<Iterator> m_queueIt;
    bool m_seqNoAssigned;
  };

  OriginalInfo &GetOriginalInfo();
  const OriginalInfo &GetOriginalInfo() const;

  using InstanceInfo = std::variant<OriginalInfo, Ptr<WifiMpdu>>;

  InstanceInfo m_instanceInfo;
  static constexpr std::size_t ORIGINAL = 0;
  static constexpr std::size_t ALIAS = 1;
};

std::ostream &operator<<(std::ostream &os, const WifiMpdu &item);

} // namespace ns3

#endif
