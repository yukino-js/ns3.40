
#ifndef TCP_TX_BUFFER_H
#define TCP_TX_BUFFER_H

#include "tcp-option-sack.h"
#include "tcp-tx-item.h"

#include "ns3/object.h"
#include "ns3/sequence-number.h"
#include "ns3/traced-value.h"

namespace ns3 {
class Packet;

class TcpTxBuffer : public Object {
public:
  static TypeId GetTypeId();
  TcpTxBuffer(uint32_t n = 0);
  ~TcpTxBuffer() override;

  SequenceNumber32 HeadSequence() const;

  SequenceNumber32 TailSequence() const;

  uint32_t Size() const;

  uint32_t MaxBufferSize() const;

  void SetMaxBufferSize(uint32_t n);

  bool IsSackEnabled() const;

  void SetSackEnabled(bool enabled);

  uint32_t Available() const;

  void SetDupAckThresh(uint32_t dupAckThresh);

  void SetSegmentSize(uint32_t segmentSize);

  uint32_t GetRetransmitsCount() const;

  uint32_t GetLost() const;

  uint32_t GetSacked() const;

  bool Add(Ptr<Packet> p);

  uint32_t SizeFromSequence(const SequenceNumber32 &seq) const;

  TcpTxItem *CopyFromSequence(uint32_t numBytes, const SequenceNumber32 &seq);

  void SetHeadSequence(const SequenceNumber32 &seq);

  bool IsRetransmittedDataAcked(const SequenceNumber32 &ack) const;

  void DiscardUpTo(const SequenceNumber32 &seq,
                   const Callback<void, TcpTxItem *> &beforeDelCb = m_nullCb);

  uint32_t Update(const TcpOptionSack::SackList &list,
                  const Callback<void, TcpTxItem *> &sackedCb = m_nullCb);

  bool IsLost(const SequenceNumber32 &seq) const;

  bool NextSeg(SequenceNumber32 *seq, SequenceNumber32 *seqHigh,
               bool isRecovery) const;

  uint32_t BytesInFlight() const;

  void SetSentListLost(bool resetSack = false);

  bool IsHeadRetransmitted() const;

  void DeleteRetransmittedFlagFromHead();

  void ResetSentList();

  void ResetLastSegmentSent();

  void MarkHeadAsLost();

  void AddRenoSack();

  void ResetRenoSack();

  void SetRWndCallback(Callback<uint32_t> rWndCallback);

private:
  friend std::ostream &operator<<(std::ostream &os,
                                  const TcpTxBuffer &tcpTxBuf);

  typedef std::list<TcpTxItem *> PacketList;

  void UpdateLostCount();

  void RemoveFromCounts(TcpTxItem *item, uint32_t size);

  bool IsLostRFC(const SequenceNumber32 &seq,
                 const PacketList::const_iterator &segment) const;

  uint32_t BytesInFlightRFC() const;

  TcpTxItem *GetNewSegment(uint32_t numBytes);

  TcpTxItem *GetTransmittedSegment(uint32_t numBytes,
                                   const SequenceNumber32 &seq);

  TcpTxItem *GetPacketFromList(PacketList &list,
                               const SequenceNumber32 &startingSeq,
                               uint32_t numBytes,
                               const SequenceNumber32 &requestedSeq,
                               bool *listEdited = nullptr) const;

  void MergeItems(TcpTxItem *t1, TcpTxItem *t2) const;

  void SplitItems(TcpTxItem *t1, TcpTxItem *t2, uint32_t size) const;

  void ConsistencyCheck() const;

  std::pair<TcpTxBuffer::PacketList::const_iterator, SequenceNumber32>
  FindHighestSacked() const;

  PacketList m_appList;
  PacketList m_sentList;
  uint32_t m_maxBuffer;
  uint32_t m_size;
  uint32_t m_sentSize;
  Callback<uint32_t> m_rWndCallback;

  TracedValue<SequenceNumber32> m_firstByteSeq;
  std::pair<PacketList::const_iterator, SequenceNumber32> m_highestSack;

  uint32_t m_lostOut{0};
  uint32_t m_sackedOut{0};
  uint32_t m_retrans{0};

  uint32_t m_dupAckThresh{0};
  uint32_t m_segmentSize{0};
  bool m_renoSack{false};
  bool m_sackEnabled{true};

  static Callback<void, TcpTxItem *> m_nullCb;
};

std::ostream &operator<<(std::ostream &os, const TcpTxBuffer &tcpTxBuf);

std::ostream &operator<<(std::ostream &os, const TcpTxItem &item);

} // namespace ns3

#endif
