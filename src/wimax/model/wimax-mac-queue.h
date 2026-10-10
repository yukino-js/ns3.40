
#ifndef WIMAX_MAC_QUEUE_H
#define WIMAX_MAC_QUEUE_H

#include "wimax-mac-header.h"

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"

#include <queue>
#include <stdint.h>

namespace ns3 {

class WimaxMacQueue : public Object {
public:
  static TypeId GetTypeId();
  WimaxMacQueue();
  WimaxMacQueue(uint32_t maxSize);
  ~WimaxMacQueue() override;
  void SetMaxSize(uint32_t maxSize);
  uint32_t GetMaxSize() const;
  bool Enqueue(Ptr<Packet> packet, const MacHeaderType &hdrType,
               const GenericMacHeader &hdr);
  Ptr<Packet> Dequeue(MacHeaderType::HeaderType packetType);
  Ptr<Packet> Dequeue(MacHeaderType::HeaderType packetType,
                      uint32_t availableByte);

  Ptr<Packet> Peek(GenericMacHeader &hdr) const;
  Ptr<Packet> Peek(GenericMacHeader &hdr, Time &timeStamp) const;

  Ptr<Packet> Peek(MacHeaderType::HeaderType packetType) const;
  Ptr<Packet> Peek(MacHeaderType::HeaderType packetType, Time &timeStamp) const;

  bool IsEmpty() const;

  bool IsEmpty(MacHeaderType::HeaderType packetType) const;

  uint32_t GetSize() const;
  uint32_t GetNBytes() const;

  bool CheckForFragmentation(MacHeaderType::HeaderType packetType);
  uint32_t GetFirstPacketHdrSize(MacHeaderType::HeaderType packetType);
  uint32_t GetFirstPacketPayloadSize(MacHeaderType::HeaderType packetType);
  uint32_t GetFirstPacketRequiredByte(MacHeaderType::HeaderType packetType);
  uint32_t GetQueueLengthWithMACOverhead();
  void SetFragmentation(MacHeaderType::HeaderType packetType);
  void SetFragmentNumber(MacHeaderType::HeaderType packetType);
  void SetFragmentOffset(MacHeaderType::HeaderType packetType, uint32_t offset);

  struct QueueElement {
    QueueElement();
    QueueElement(Ptr<Packet> packet, const MacHeaderType &hdrType,
                 const GenericMacHeader &hdr, Time timeStamp);
    uint32_t GetSize() const;
    Ptr<Packet> m_packet;
    MacHeaderType m_hdrType;
    GenericMacHeader m_hdr;
    Time m_timeStamp;

    bool m_fragmentation;
    uint32_t m_fragmentNumber;
    uint32_t m_fragmentOffset;

    void SetFragmentation();
    void SetFragmentNumber();
    void SetFragmentOffset(uint32_t offset);
  };

private:
  WimaxMacQueue::QueueElement Front(MacHeaderType::HeaderType packetType) const;
  void Pop(MacHeaderType::HeaderType packetType);

  typedef std::deque<QueueElement> PacketQueue;
  PacketQueue m_queue;
  uint32_t m_maxSize;
  uint32_t m_bytes;
  uint32_t m_nrDataPackets;
  uint32_t m_nrRequestPackets;

  TracedCallback<Ptr<const Packet>> m_traceEnqueue;
  TracedCallback<Ptr<const Packet>> m_traceDequeue;
  TracedCallback<Ptr<const Packet>> m_traceDrop;

public:
  const WimaxMacQueue::PacketQueue &GetPacketQueue() const;
};

} // namespace ns3

#endif
