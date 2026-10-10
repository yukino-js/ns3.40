
#ifndef LR_WPAN_CONSTANTS_H
#define LR_WPAN_CONSTANTS_H

#include <cstdint>

namespace ns3 {
namespace lrwpan {

constexpr uint32_t aMaxPhyPacketSize{127};

constexpr uint32_t aTurnaroundTime{12};

constexpr uint32_t aMinMPDUOverhead{9};

constexpr uint32_t aBaseSlotDuration{60};

constexpr uint32_t aNumSuperframeSlots{16};

constexpr uint32_t aBaseSuperframeDuration{aBaseSlotDuration *
                                           aNumSuperframeSlots};

constexpr uint32_t aMaxLostBeacons{4};

constexpr uint32_t aMaxSIFSFrameSize{18};

constexpr uint32_t aUnitBackoffPeriod{20};

constexpr uint32_t aMaxBeaconOverhead{75};

constexpr uint32_t aMaxBeaconPayloadLength{aMaxPhyPacketSize -
                                           aMaxBeaconOverhead};

} // namespace lrwpan
} // namespace ns3

#endif
