
#ifndef QOS_UTILS_H
#define QOS_UTILS_H

#include "ns3/ptr.h"

#include <map>

namespace ns3 {

class Packet;
class WifiMacHeader;
class QueueItem;
class Mac48Address;

typedef std::pair<Mac48Address, uint8_t> WifiAddressTidPair;

struct WifiAddressTidHash {
  std::size_t operator()(const WifiAddressTidPair &addressTidPair) const;
};

struct WifiAddressHash {
  std::size_t operator()(const Mac48Address &address) const;
};

enum AcIndex : uint8_t {
  AC_BE = 0,
  AC_BK = 1,
  AC_VI = 2,
  AC_VO = 3,
  AC_BE_NQOS = 4,
  AC_BEACON = 5,
  AC_UNDEF
};

class WifiAc {
public:
  WifiAc(uint8_t lowTid, uint8_t highTid);
  uint8_t GetLowTid() const;
  uint8_t GetHighTid() const;
  uint8_t GetOtherTid(uint8_t tid) const;

private:
  uint8_t m_lowTid;
  uint8_t m_highTid;
};

bool operator>(AcIndex left, AcIndex right);

bool operator>=(AcIndex left, AcIndex right);

bool operator<(AcIndex left, AcIndex right);

bool operator<=(AcIndex left, AcIndex right);

extern const std::map<AcIndex, WifiAc> wifiAcList;

AcIndex QosUtilsMapTidToAc(uint8_t tid);

uint8_t QosUtilsGetTidForPacket(Ptr<const Packet> packet);

uint32_t QosUtilsMapSeqControlToUniqueInteger(uint16_t seqControl,
                                              uint16_t endSequence);

bool QosUtilsIsOldPacket(uint16_t startingSeq, uint16_t seqNumber);

uint8_t GetTid(Ptr<const Packet> packet, const WifiMacHeader hdr);

uint8_t SelectQueueByDSField(Ptr<QueueItem> item);

} // namespace ns3

#endif
