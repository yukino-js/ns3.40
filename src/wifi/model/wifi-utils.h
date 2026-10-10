
#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include "block-ack-type.h"

#include "ns3/fatal-error.h"
#include "ns3/ptr.h"

#include <list>
#include <map>
#include <set>

namespace ns3 {

class WifiMacHeader;
class Packet;

enum class WifiDirection : uint8_t {
  DOWNLINK = 0,
  UPLINK = 1,
  BOTH_DIRECTIONS = 2,
};

inline std::ostream &operator<<(std::ostream &os,
                                const WifiDirection &direction) {
  switch (direction) {
  case WifiDirection::DOWNLINK:
    return (os << "DOWNLINK");
  case WifiDirection::UPLINK:
    return (os << "UPLINK");
  case WifiDirection::BOTH_DIRECTIONS:
    return (os << "BOTH_DIRECTIONS");
  default:
    NS_FATAL_ERROR("Invalid direction");
    return (os << "INVALID");
  }
}

using WifiTidLinkMapping = std::map<uint8_t, std::set<uint8_t>>;

double DbmToW(double dbm);
double DbToRatio(double db);
double WToDbm(double w);
double RatioToDb(double ratio);
uint32_t GetAckSize();
uint32_t GetBlockAckSize(BlockAckType type);
uint32_t GetBlockAckRequestSize(BlockAckReqType type);
uint32_t GetMuBarSize(std::list<BlockAckReqType> types);
uint32_t GetRtsSize();
uint32_t GetCtsSize();
bool IsInWindow(uint16_t seq, uint16_t winstart, uint16_t winsize);
void AddWifiMacTrailer(Ptr<Packet> packet);
uint32_t GetSize(Ptr<const Packet> packet, const WifiMacHeader *hdr,
                 bool isAmpdu);

bool TidToLinkMappingValidForNegType1(const WifiTidLinkMapping &dlLinkMapping,
                                      const WifiTidLinkMapping &ulLinkMapping);

static constexpr uint16_t SEQNO_SPACE_SIZE = 4096;

static constexpr uint16_t SEQNO_SPACE_HALF_SIZE = SEQNO_SPACE_SIZE / 2;

static constexpr uint8_t SINGLE_LINK_OP_ID = 0;

static constexpr uint8_t WIFI_LINKID_UNDEFINED = 0xff;

} // namespace ns3

#endif
