
#ifndef PACKET_DATA_CALCULATORS_H
#define PACKET_DATA_CALCULATORS_H

#include "mac48-address.h"

#include "ns3/basic-data-calculators.h"
#include "ns3/data-calculator.h"
#include "ns3/packet.h"

namespace ns3 {

class PacketCounterCalculator : public CounterCalculator<uint32_t> {
public:
  PacketCounterCalculator();
  ~PacketCounterCalculator() override;

  static TypeId GetTypeId();

  void PacketUpdate(std::string path, Ptr<const Packet> packet);

  void FrameUpdate(std::string path, Ptr<const Packet> packet,
                   Mac48Address realto);

protected:
  void DoDispose() override;
};

class PacketSizeMinMaxAvgTotalCalculator
    : public MinMaxAvgTotalCalculator<uint32_t> {
public:
  PacketSizeMinMaxAvgTotalCalculator();
  ~PacketSizeMinMaxAvgTotalCalculator() override;

  static TypeId GetTypeId();

  void PacketUpdate(std::string path, Ptr<const Packet> packet);

  void FrameUpdate(std::string path, Ptr<const Packet> packet,
                   Mac48Address realto);

protected:
  void DoDispose() override;
};

}; // namespace ns3

#endif
