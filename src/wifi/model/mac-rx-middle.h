
#ifndef MAC_RX_MIDDLE_H
#define MAC_RX_MIDDLE_H

#include "ns3/callback.h"
#include "ns3/simple-ref-count.h"

#include <map>

namespace ns3 {

class WifiMacHeader;
class OriginatorRxStatus;
class Packet;
class Mac48Address;
class WifiMpdu;

class MacRxMiddle : public SimpleRefCount<MacRxMiddle> {
public:
  typedef Callback<void, Ptr<const WifiMpdu>, uint8_t> ForwardUpCallback;

  MacRxMiddle();
  ~MacRxMiddle();

  void SetForwardCallback(ForwardUpCallback callback);

  void Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId);

private:
  friend class MacRxMiddleTest;
  OriginatorRxStatus *Lookup(const WifiMacHeader *hdr);
  bool IsDuplicate(const WifiMacHeader *hdr,
                   OriginatorRxStatus *originator) const;
  Ptr<const Packet> HandleFragments(Ptr<const Packet> packet,
                                    const WifiMacHeader *hdr,
                                    OriginatorRxStatus *originator);

  typedef std::map<Mac48Address, OriginatorRxStatus *, std::less<>> Originators;
  typedef std::map<std::pair<Mac48Address, uint8_t>, OriginatorRxStatus *,
                   std::less<>>
      QosOriginators;
  typedef std::map<Mac48Address, OriginatorRxStatus *, std::less<>>::iterator
      OriginatorsI;
  typedef std::map<std::pair<Mac48Address, uint8_t>, OriginatorRxStatus *,
                   std::less<>>::iterator QosOriginatorsI;

  Originators m_originatorStatus;
  QosOriginators m_qosOriginatorStatus;
  ForwardUpCallback m_callback;
};

} // namespace ns3

#endif
