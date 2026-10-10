
#ifndef BLOCK_ACK_TYPE_H
#define BLOCK_ACK_TYPE_H

#include <cstdint>
#include <ostream>
#include <vector>

namespace ns3 {

struct BlockAckType {
  enum Variant { BASIC, COMPRESSED, EXTENDED_COMPRESSED, MULTI_TID, MULTI_STA };

  Variant m_variant;
  std::vector<uint8_t> m_bitmapLen;

  BlockAckType();
  BlockAckType(Variant v);
  BlockAckType(Variant v, std::vector<uint8_t> l);
};

struct BlockAckReqType {
  enum Variant { BASIC, COMPRESSED, EXTENDED_COMPRESSED, MULTI_TID };

  Variant m_variant;
  uint8_t m_nSeqControls;

  BlockAckReqType();
  BlockAckReqType(Variant v);
  BlockAckReqType(Variant v, uint8_t nSeqControls);
};

std::ostream &operator<<(std::ostream &os, const BlockAckType &type);

std::ostream &operator<<(std::ostream &os, const BlockAckReqType &type);

} // namespace ns3

#endif
