
#ifndef PACKET_LOSS_COUNTER_H
#define PACKET_LOSS_COUNTER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"

namespace ns3 {

class Socket;
class Packet;

class PacketLossCounter {
public:
  PacketLossCounter(uint8_t bitmapSize);
  ~PacketLossCounter();
  void NotifyReceived(uint32_t seq);
  uint32_t GetLost() const;
  uint16_t GetBitMapSize() const;
  void SetBitMapSize(uint16_t size);

private:
  bool GetBit(uint32_t seqNum);
  void SetBit(uint32_t seqNum, bool val);

  uint32_t m_lost;
  uint16_t m_bitMapSize;
  uint32_t m_lastMaxSeqNum;
  uint8_t *m_receiveBitMap;
};
} // namespace ns3

#endif
