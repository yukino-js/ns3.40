
#ifndef SEND_PARAMS_H
#define SEND_PARAMS_H

#include <stdint.h>

namespace ns3 {

class WimaxPhy;

class SendParams {
public:
  SendParams();
  virtual ~SendParams();

private:
};

} // namespace ns3

#endif

#ifndef OFDM_SEND_PARAMS_H
#define OFDM_SEND_PARAMS_H

#include "ns3/packet-burst.h"

#include <stdint.h>

namespace ns3 {

class OfdmSendParams : public SendParams {
public:
  OfdmSendParams(Ptr<PacketBurst> burst, uint8_t modulationType,
                 uint8_t direction);
  ~OfdmSendParams() override;

  Ptr<PacketBurst> GetBurst() const { return m_burst; }

  uint8_t GetModulationType() const { return m_modulationType; }

  uint8_t GetDirection() const { return m_direction; }

private:
  Ptr<PacketBurst> m_burst;
  uint8_t m_modulationType;
  uint8_t m_direction;
};

} // namespace ns3

#endif
