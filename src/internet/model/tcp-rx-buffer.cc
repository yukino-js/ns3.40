
#include "tcp-rx-buffer.h"

#include "ns3/log.h"
#include "ns3/packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpRxBuffer");

NS_OBJECT_ENSURE_REGISTERED(TcpRxBuffer);

TypeId TcpRxBuffer::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TcpRxBuffer")
          .SetParent<Object>()
          .SetGroupName("Internet")
          .AddConstructor<TcpRxBuffer>()
          .AddTraceSource("NextRxSequence",
                          "Next sequence number expected (RCV.NXT)",
                          MakeTraceSourceAccessor(&TcpRxBuffer::m_nextRxSeq),
                          "ns3::SequenceNumber32TracedValueCallback");
  return tid;
}

TcpRxBuffer::TcpRxBuffer(uint32_t n)
    : m_nextRxSeq(n), m_gotFin(false), m_size(0), m_maxBuffer(32768),
      m_availBytes(0) {}

TcpRxBuffer::~TcpRxBuffer() {}

SequenceNumber32 TcpRxBuffer::NextRxSequence() const { return m_nextRxSeq; }

void TcpRxBuffer::SetNextRxSequence(const SequenceNumber32 &s) {
  m_nextRxSeq = s;
}

uint32_t TcpRxBuffer::MaxBufferSize() const { return m_maxBuffer; }

void TcpRxBuffer::SetMaxBufferSize(uint32_t s) { m_maxBuffer = s; }

uint32_t TcpRxBuffer::Size() const { return m_size; }

uint32_t TcpRxBuffer::Available() const { return m_availBytes; }

void TcpRxBuffer::IncNextRxSequence() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_size == 0);
  m_nextRxSeq++;
}

SequenceNumber32 TcpRxBuffer::MaxRxSequence() const {
  if (m_gotFin) {
    return m_finSeq;
  } else if (!m_data.empty() && m_nextRxSeq > m_data.begin()->first) {
    return m_data.begin()->first + SequenceNumber32(m_maxBuffer);
  }
  return m_nextRxSeq + SequenceNumber32(m_maxBuffer);
}

void TcpRxBuffer::SetFinSequence(const SequenceNumber32 &s) {
  NS_LOG_FUNCTION(this);

  m_gotFin = true;
  m_finSeq = s;
  if (m_nextRxSeq == m_finSeq) {
    ++m_nextRxSeq;
  }
}

bool TcpRxBuffer::Finished() { return (m_gotFin && m_finSeq < m_nextRxSeq); }

bool TcpRxBuffer::Add(Ptr<Packet> p, const TcpHeader &tcph) {
  NS_LOG_FUNCTION(this << p << tcph);

  uint32_t pktSize = p->GetSize();
  SequenceNumber32 headSeq = tcph.GetSequenceNumber();
  SequenceNumber32 tailSeq = headSeq + SequenceNumber32(pktSize);
  NS_LOG_LOGIC("Add pkt " << p << " len=" << pktSize << " seq=" << headSeq
                          << ", when NextRxSeq=" << m_nextRxSeq
                          << ", buffsize=" << m_size);

  if (headSeq < m_nextRxSeq) {
    headSeq = m_nextRxSeq;
  }
  if (!m_data.empty()) {
    SequenceNumber32 maxSeq =
        m_data.begin()->first + SequenceNumber32(m_maxBuffer);
    if (maxSeq < tailSeq) {
      tailSeq = maxSeq;
    }
    if (tailSeq < headSeq) {
      headSeq = tailSeq;
    }
  }
  auto i = m_data.begin();
  while (i != m_data.end() && i->first <= tailSeq) {
    SequenceNumber32 lastByteSeq =
        i->first + SequenceNumber32(i->second->GetSize());
    if (lastByteSeq > headSeq) {
      if (i->first > headSeq && lastByteSeq < tailSeq) {
        m_size -= i->second->GetSize();
        m_data.erase(i++);
        continue;
      }
      if (i->first <= headSeq) {
        headSeq = lastByteSeq;
      }
      if (lastByteSeq >= tailSeq) {
        tailSeq = i->first;
      }
    }
    ++i;
  }
  if (headSeq >= tailSeq) {
    NS_LOG_LOGIC("Nothing to buffer");
    return false;
  } else {
    uint32_t start = static_cast<uint32_t>(headSeq - tcph.GetSequenceNumber());
    auto length = static_cast<uint32_t>(tailSeq - headSeq);
    p = p->CreateFragment(start, length);
    NS_ASSERT(length == p->GetSize());
  }
  NS_ASSERT(m_data.find(headSeq) == m_data.end());
  m_data[headSeq] = p;

  if (headSeq > m_nextRxSeq) {
    UpdateSackList(headSeq, tailSeq);
  }

  NS_LOG_LOGIC("Buffered packet of seqno=" << headSeq
                                           << " len=" << p->GetSize());
  m_size += p->GetSize();
  for (i = m_data.begin(); i != m_data.end(); ++i) {
    if (i->first < m_nextRxSeq) {
      continue;
    } else if (i->first > m_nextRxSeq) {
      break;
    };
    m_nextRxSeq = i->first + SequenceNumber32(i->second->GetSize());
    m_availBytes += i->second->GetSize();
    ClearSackList(m_nextRxSeq);
  }
  NS_LOG_LOGIC("Updated buffer occupancy=" << m_size
                                           << " nextRxSeq=" << m_nextRxSeq);
  if (m_gotFin && m_nextRxSeq == m_finSeq) {
    ++m_nextRxSeq;
  };
  return true;
}

uint32_t TcpRxBuffer::GetSackListSize() const {
  NS_LOG_FUNCTION(this);

  return static_cast<uint32_t>(m_sackList.size());
}

void TcpRxBuffer::UpdateSackList(const SequenceNumber32 &head,
                                 const SequenceNumber32 &tail) {
  NS_LOG_FUNCTION(this << head << tail);
  NS_ASSERT(head > m_nextRxSeq);

  TcpOptionSack::SackBlock current;
  current.first = head;
  current.second = tail;

  m_sackList.push_front(current);

  bool updated = false;
  auto it = m_sackList.begin();
  TcpOptionSack::SackBlock begin = *it;
  TcpOptionSack::SackBlock merged;
  ++it;

  while (it != m_sackList.end()) {
    current = *it;

    if (begin.first == current.second) {
      NS_ASSERT(current.first < begin.second);
      merged = TcpOptionSack::SackBlock(current.first, begin.second);
      updated = true;
    } else if (begin.second == current.first) {
      NS_ASSERT(begin.first < current.second);
      merged = TcpOptionSack::SackBlock(begin.first, current.second);
      updated = true;
    }

    if (updated) {
      m_sackList.erase(it);
      m_sackList.pop_front();
      m_sackList.push_front(merged);
      it = m_sackList.begin();
      begin = *it;
      updated = false;
    }

    ++it;
  }

  if (m_sackList.size() > 4) {
    m_sackList.pop_back();
  }
}

void TcpRxBuffer::ClearSackList(const SequenceNumber32 &seq) {
  NS_LOG_FUNCTION(this << seq);

  for (auto it = m_sackList.begin(); it != m_sackList.end();) {
    TcpOptionSack::SackBlock block = *it;
    NS_ASSERT(block.first < block.second);

    if (block.second <= seq) {
      it = m_sackList.erase(it);
    } else {
      it++;
    }
  }
}

TcpOptionSack::SackList TcpRxBuffer::GetSackList() const { return m_sackList; }

Ptr<Packet> TcpRxBuffer::Extract(uint32_t maxSize) {
  NS_LOG_FUNCTION(this << maxSize);

  uint32_t extractSize = std::min(maxSize, m_availBytes);
  NS_LOG_LOGIC("Requested to extract "
               << extractSize << " bytes from TcpRxBuffer of size=" << m_size);
  if (extractSize == 0) {
    return nullptr;
  }
  NS_ASSERT(!m_data.empty());
  Ptr<Packet> outPkt = Create<Packet>();
  BufIterator i;
  while (extractSize) {
    i = m_data.begin();
    NS_ASSERT(i->first <= m_nextRxSeq);
    uint32_t pktSize = i->second->GetSize();
    if (pktSize <= extractSize) {
      outPkt->AddAtEnd(i->second);
      m_data.erase(i);
      m_size -= pktSize;
      m_availBytes -= pktSize;
      extractSize -= pktSize;
    } else {
      outPkt->AddAtEnd(i->second->CreateFragment(0, extractSize));
      m_data[i->first + SequenceNumber32(extractSize)] =
          i->second->CreateFragment(extractSize, pktSize - extractSize);
      m_data.erase(i);
      m_size -= extractSize;
      m_availBytes -= extractSize;
      extractSize = 0;
    }
  }
  if (outPkt->GetSize() == 0) {
    NS_LOG_LOGIC("Nothing extracted.");
    return nullptr;
  }
  NS_LOG_LOGIC("Extracted " << outPkt->GetSize() << " bytes, bufsize=" << m_size
                            << ", num pkts in buffer=" << m_data.size());
  return outPkt;
}

} // namespace ns3
