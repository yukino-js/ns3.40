
#include "mac-rx-middle.h"

#include "wifi-mpdu.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/sequence-number.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MacRxMiddle");

class OriginatorRxStatus {
private:
  typedef std::list<Ptr<const Packet>> Fragments;
  typedef std::list<Ptr<const Packet>>::const_iterator FragmentsCI;

  bool m_defragmenting;
  uint16_t m_lastSequenceControl;
  Fragments m_fragments;

public:
  OriginatorRxStatus() {
    m_lastSequenceControl = 0xffff;
    m_defragmenting = false;
  }

  ~OriginatorRxStatus() { m_fragments.clear(); }

  bool IsDeFragmenting() const { return m_defragmenting; }

  void AccumulateFirstFragment(Ptr<const Packet> packet) {
    NS_ASSERT(!m_defragmenting);
    m_defragmenting = true;
    m_fragments.push_back(packet);
  }

  Ptr<Packet> AccumulateLastFragment(Ptr<const Packet> packet) {
    NS_ASSERT(m_defragmenting);
    m_fragments.push_back(packet);
    m_defragmenting = false;
    Ptr<Packet> full = Create<Packet>();
    for (auto i = m_fragments.begin(); i != m_fragments.end(); i++) {
      full->AddAtEnd(*i);
    }
    m_fragments.erase(m_fragments.begin(), m_fragments.end());
    return full;
  }

  void AccumulateFragment(Ptr<const Packet> packet) {
    NS_ASSERT(m_defragmenting);
    m_fragments.push_back(packet);
  }

  bool IsNextFragment(uint16_t sequenceControl) const {
    return (sequenceControl >> 4) == (m_lastSequenceControl >> 4) &&
           (sequenceControl & 0x0f) == ((m_lastSequenceControl & 0x0f) + 1);
  }

  uint16_t GetLastSequenceControl() const { return m_lastSequenceControl; }

  void SetSequenceControl(uint16_t sequenceControl) {
    m_lastSequenceControl = sequenceControl;
  }
};

MacRxMiddle::MacRxMiddle() { NS_LOG_FUNCTION_NOARGS(); }

MacRxMiddle::~MacRxMiddle() {
  NS_LOG_FUNCTION_NOARGS();
  for (auto i = m_originatorStatus.begin(); i != m_originatorStatus.end();
       i++) {
    delete (*i).second;
  }
  m_originatorStatus.erase(m_originatorStatus.begin(),
                           m_originatorStatus.end());
  for (auto i = m_qosOriginatorStatus.begin(); i != m_qosOriginatorStatus.end();
       i++) {
    delete (*i).second;
  }
  m_qosOriginatorStatus.erase(m_qosOriginatorStatus.begin(),
                              m_qosOriginatorStatus.end());
}

void MacRxMiddle::SetForwardCallback(ForwardUpCallback callback) {
  NS_LOG_FUNCTION_NOARGS();
  m_callback = callback;
}

OriginatorRxStatus *MacRxMiddle::Lookup(const WifiMacHeader *hdr) {
  NS_LOG_FUNCTION(hdr);
  OriginatorRxStatus *originator;
  Mac48Address source = hdr->GetAddr2();
  if (hdr->IsQosData() && !hdr->GetAddr2().IsGroup()) {
    originator =
        m_qosOriginatorStatus[std::make_pair(source, hdr->GetQosTid())];
    if (originator == nullptr) {
      originator = new OriginatorRxStatus();
      m_qosOriginatorStatus[std::make_pair(source, hdr->GetQosTid())] =
          originator;
    }
  } else {
    originator = m_originatorStatus[source];
    if (originator == nullptr) {
      originator = new OriginatorRxStatus();
      m_originatorStatus[source] = originator;
    }
  }
  return originator;
}

bool MacRxMiddle::IsDuplicate(const WifiMacHeader *hdr,
                              OriginatorRxStatus *originator) const {
  NS_LOG_FUNCTION(hdr << originator);
  return hdr->IsRetry() &&
         originator->GetLastSequenceControl() == hdr->GetSequenceControl();
}

Ptr<const Packet> MacRxMiddle::HandleFragments(Ptr<const Packet> packet,
                                               const WifiMacHeader *hdr,
                                               OriginatorRxStatus *originator) {
  NS_LOG_FUNCTION(packet << hdr << originator);
  if (originator->IsDeFragmenting()) {
    if (hdr->IsMoreFragments()) {
      if (originator->IsNextFragment(hdr->GetSequenceControl())) {
        NS_LOG_DEBUG("accumulate fragment seq="
                     << hdr->GetSequenceNumber()
                     << ", frag=" << +hdr->GetFragmentNumber()
                     << ", size=" << packet->GetSize());
        originator->AccumulateFragment(packet);
        originator->SetSequenceControl(hdr->GetSequenceControl());
      } else {
        NS_LOG_DEBUG("non-ordered fragment");
      }
      return nullptr;
    } else {
      if (originator->IsNextFragment(hdr->GetSequenceControl())) {
        NS_LOG_DEBUG("accumulate last fragment seq="
                     << hdr->GetSequenceNumber()
                     << ", frag=" << +hdr->GetFragmentNumber()
                     << ", size=" << hdr->GetSize());
        Ptr<Packet> p = originator->AccumulateLastFragment(packet);
        originator->SetSequenceControl(hdr->GetSequenceControl());
        return p;
      } else {
        NS_LOG_DEBUG("non-ordered fragment");
        return nullptr;
      }
    }
  } else {
    if (hdr->IsMoreFragments()) {
      NS_LOG_DEBUG("accumulate first fragment seq="
                   << hdr->GetSequenceNumber()
                   << ", frag=" << +hdr->GetFragmentNumber()
                   << ", size=" << packet->GetSize());
      originator->AccumulateFirstFragment(packet);
      originator->SetSequenceControl(hdr->GetSequenceControl());
      return nullptr;
    } else {
      return packet;
    }
  }
}

void MacRxMiddle::Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  NS_LOG_FUNCTION(*mpdu << +linkId);
  const WifiMacHeader *hdr = &mpdu->GetOriginal()->GetHeader();
  NS_ASSERT(hdr->IsData() || hdr->IsMgt());

  OriginatorRxStatus *originator = Lookup(hdr);
  if (!(SequenceNumber16(originator->GetLastSequenceControl()) <
        SequenceNumber16(hdr->GetSequenceControl()))) {
    NS_LOG_DEBUG("Sequence numbers have looped back. last recorded="
                 << originator->GetLastSequenceControl()
                 << " currently seen=" << hdr->GetSequenceControl());
  }
  if (IsDuplicate(hdr, originator)) {
    NS_LOG_DEBUG("duplicate from=" << hdr->GetAddr2()
                                   << ", seq=" << hdr->GetSequenceNumber()
                                   << ", frag=" << +hdr->GetFragmentNumber());
    return;
  }
  Ptr<const Packet> aggregate =
      HandleFragments(mpdu->GetPacket(), hdr, originator);
  if (!aggregate) {
    return;
  }
  NS_LOG_DEBUG("forwarding data from="
               << hdr->GetAddr2() << ", seq=" << hdr->GetSequenceNumber()
               << ", frag=" << +hdr->GetFragmentNumber());
  if (!hdr->GetAddr1().IsGroup()) {
    originator->SetSequenceControl(hdr->GetSequenceControl());
  }
  if (aggregate == mpdu->GetPacket()) {
    m_callback(mpdu, linkId);
  } else {
    m_callback(Create<WifiMpdu>(aggregate, *hdr), linkId);
  }
}

} // namespace ns3
