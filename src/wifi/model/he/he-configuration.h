
#ifndef HE_CONFIGURATION_H
#define HE_CONFIGURATION_H

#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3 {

class HeConfiguration : public Object {
public:
  HeConfiguration();

  static TypeId GetTypeId();

  void SetGuardInterval(Time guardInterval);
  Time GetGuardInterval() const;
  void SetBssColor(uint8_t bssColor);
  uint8_t GetBssColor() const;
  void SetMaxTbPpduDelay(Time maxTbPpduDelay);
  Time GetMaxTbPpduDelay() const;
  void SetMpduBufferSize(uint16_t size);
  uint16_t GetMpduBufferSize() const;

private:
  Time m_guardInterval;
  uint8_t m_bssColor;
  Time m_maxTbPpduDelay;
  uint16_t m_mpduBufferSize;
  uint8_t m_muBeAifsn;
  uint8_t m_muBkAifsn;
  uint8_t m_muViAifsn;
  uint8_t m_muVoAifsn;
  uint16_t m_muBeCwMin;
  uint16_t m_muBkCwMin;
  uint16_t m_muViCwMin;
  uint16_t m_muVoCwMin;
  uint16_t m_muBeCwMax;
  uint16_t m_muBkCwMax;
  uint16_t m_muViCwMax;
  uint16_t m_muVoCwMax;
  Time m_beMuEdcaTimer;
  Time m_bkMuEdcaTimer;
  Time m_viMuEdcaTimer;
  Time m_voMuEdcaTimer;
};

} // namespace ns3

#endif
