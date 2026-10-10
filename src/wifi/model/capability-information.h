
#ifndef CAPABILITY_INFORMATION_H
#define CAPABILITY_INFORMATION_H

#include "ns3/buffer.h"

namespace ns3 {

class CapabilityInformation {
public:
  CapabilityInformation();

  void SetEss();
  void SetIbss();
  void SetShortPreamble(bool shortPreamble);
  void SetShortSlotTime(bool shortSlotTime);
  void SetCfPollable();

  bool IsEss() const;
  bool IsIbss() const;
  bool IsShortPreamble() const;
  bool IsShortSlotTime() const;
  bool IsCfPollable() const;

  uint32_t GetSerializedSize() const;
  Buffer::Iterator Serialize(Buffer::Iterator start) const;
  Buffer::Iterator Deserialize(Buffer::Iterator start);

private:
  bool Is(uint8_t n) const;
  void Set(uint8_t n);
  void Clear(uint8_t n);

  uint16_t m_capability;
};

} // namespace ns3

#endif
