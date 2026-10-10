
#ifndef TCP_RX_BUFFER_H
#define TCP_RX_BUFFER_H

#include "tcp-header.h"
#include "tcp-option-sack.h"

#include "ns3/ptr.h"
#include "ns3/sequence-number.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

#include <map>

namespace ns3 {
class Packet;

class TcpRxBuffer : public Object {
public:
  static TypeId GetTypeId();
  TcpRxBuffer(uint32_t n = 0);
  ~TcpRxBuffer() override;

  SequenceNumber32 NextRxSequence() const;
  SequenceNumber32 MaxRxSequence() const;
  void IncNextRxSequence();
  void SetNextRxSequence(const SequenceNumber32 &s);
  void SetFinSequence(const SequenceNumber32 &s);
  uint32_t MaxBufferSize() const;
  void SetMaxBufferSize(uint32_t s);
  uint32_t Size() const;
  uint32_t Available() const;
  bool Finished();

  bool Add(Ptr<Packet> p, const TcpHeader &tcph);

  Ptr<Packet> Extract(uint32_t maxSize);

  TcpOptionSack::SackList GetSackList() const;

  uint32_t GetSackListSize() const;

  bool GotFin() const { return m_gotFin; }

private:
  void UpdateSackList(const SequenceNumber32 &head,
                      const SequenceNumber32 &tail);

  void ClearSackList(const SequenceNumber32 &seq);

  TcpOptionSack::SackList m_sackList;

  typedef std::map<SequenceNumber32, Ptr<Packet>>::iterator BufIterator;
  TracedValue<SequenceNumber32> m_nextRxSeq;
  SequenceNumber32 m_finSeq;
  bool m_gotFin;
  uint32_t m_size;
  uint32_t m_maxBuffer;
  uint32_t m_availBytes;
  std::map<SequenceNumber32, Ptr<Packet>> m_data;
};

} // namespace ns3

#endif
