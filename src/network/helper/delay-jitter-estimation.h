#ifndef DELAY_JITTER_ESTIMATION_H
#define DELAY_JITTER_ESTIMATION_H

#include "ns3/nstime.h"
#include "ns3/packet.h"

namespace ns3 {

class DelayJitterEstimation {
public:
  DelayJitterEstimation();

  static void PrepareTx(Ptr<const Packet> packet);

  void RecordRx(Ptr<const Packet> packet);

  Time GetLastDelay() const;

  uint64_t GetLastJitter() const;

private:
  Time m_jitter{0};
  Time m_transit{0};
};

} // namespace ns3

#endif
