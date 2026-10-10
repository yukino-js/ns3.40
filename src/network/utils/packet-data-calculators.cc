
#include "packet-data-calculators.h"

#include "mac48-address.h"

#include "ns3/basic-data-calculators.h"
#include "ns3/log.h"
#include "ns3/packet.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PacketDataCalculators");

PacketCounterCalculator::PacketCounterCalculator() { NS_LOG_FUNCTION_NOARGS(); }

PacketCounterCalculator::~PacketCounterCalculator() {
  NS_LOG_FUNCTION_NOARGS();
}

TypeId PacketCounterCalculator::GetTypeId() {
  static TypeId tid = TypeId("ns3::PacketCounterCalculator")
                          .SetParent<CounterCalculator<uint32_t>>()
                          .SetGroupName("Network")
                          .AddConstructor<PacketCounterCalculator>();
  return tid;
}

void PacketCounterCalculator::DoDispose() {
  NS_LOG_FUNCTION_NOARGS();

  CounterCalculator<uint32_t>::DoDispose();
}

void PacketCounterCalculator::PacketUpdate(std::string path,
                                           Ptr<const Packet> packet) {
  NS_LOG_FUNCTION_NOARGS();

  CounterCalculator<uint32_t>::Update();
}

void PacketCounterCalculator::FrameUpdate(std::string path,
                                          Ptr<const Packet> packet,
                                          Mac48Address realto) {
  NS_LOG_FUNCTION_NOARGS();

  CounterCalculator<uint32_t>::Update();
}

PacketSizeMinMaxAvgTotalCalculator::PacketSizeMinMaxAvgTotalCalculator() {
  NS_LOG_FUNCTION_NOARGS();
}

PacketSizeMinMaxAvgTotalCalculator::~PacketSizeMinMaxAvgTotalCalculator() {
  NS_LOG_FUNCTION_NOARGS();
}

TypeId PacketSizeMinMaxAvgTotalCalculator::GetTypeId() {
  static TypeId tid = TypeId("ns3::PacketSizeMinMaxAvgTotalCalculator")
                          .SetParent<MinMaxAvgTotalCalculator<uint32_t>>()
                          .SetGroupName("Network")
                          .AddConstructor<PacketSizeMinMaxAvgTotalCalculator>();
  return tid;
}

void PacketSizeMinMaxAvgTotalCalculator::DoDispose() {
  NS_LOG_FUNCTION_NOARGS();

  MinMaxAvgTotalCalculator<uint32_t>::DoDispose();
}

void PacketSizeMinMaxAvgTotalCalculator::PacketUpdate(
    std::string path, Ptr<const Packet> packet) {
  NS_LOG_FUNCTION_NOARGS();

  MinMaxAvgTotalCalculator<uint32_t>::Update(packet->GetSize());
}

void PacketSizeMinMaxAvgTotalCalculator::FrameUpdate(std::string path,
                                                     Ptr<const Packet> packet,
                                                     Mac48Address realto) {
  NS_LOG_FUNCTION_NOARGS();

  MinMaxAvgTotalCalculator<uint32_t>::Update(packet->GetSize());
}
