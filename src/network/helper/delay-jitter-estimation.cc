
#include "delay-jitter-estimation.h"

#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/timestamp-tag.h"

namespace ns3 {

DelayJitterEstimation::DelayJitterEstimation() {}

void DelayJitterEstimation::PrepareTx(Ptr<const Packet> packet) {
  TimestampTag tag(Simulator::Now());
  packet->AddByteTag(tag);
}

void DelayJitterEstimation::RecordRx(Ptr<const Packet> packet) {
  TimestampTag tag;

  if (!packet->FindFirstMatchingByteTag(tag)) {
    return;
  }

  Time r_ts = tag.GetTimestamp();
  Time arrival = Simulator::Now();
  Time transit = arrival - r_ts;
  Time delta = transit - m_transit;
  m_transit = transit;

  m_jitter += Abs(delta) - ((m_jitter + TimeStep(8)) / 16);
}

Time DelayJitterEstimation::GetLastDelay() const { return m_transit; }

uint64_t DelayJitterEstimation::GetLastJitter() const {

  return (m_jitter / 16).GetTimeStep();
}

} // namespace ns3
