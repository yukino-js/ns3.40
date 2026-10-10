
#ifndef MAC_TX_MIDDLE_H
#define MAC_TX_MIDDLE_H

#include "ns3/simple-ref-count.h"

#include <map>

namespace ns3 {

class WifiMacHeader;
class Mac48Address;

class MacTxMiddle : public SimpleRefCount<MacTxMiddle> {
public:
  MacTxMiddle();
  ~MacTxMiddle();

  uint16_t GetNextSequenceNumberFor(const WifiMacHeader *hdr);
  uint16_t PeekNextSequenceNumberFor(const WifiMacHeader *hdr);
  uint16_t GetNextSeqNumberByTidAndAddress(uint8_t tid,
                                           Mac48Address addr) const;
  void SetSequenceNumberFor(const WifiMacHeader *hdr);

private:
  std::map<Mac48Address, uint16_t *> m_qosSequences;
  uint16_t m_sequence;
};

} // namespace ns3

#endif
