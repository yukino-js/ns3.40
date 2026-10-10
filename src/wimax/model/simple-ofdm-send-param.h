
#ifndef SIMPLE_OFDM_SEND_PARAM_H
#define SIMPLE_OFDM_SEND_PARAM_H

#include "bvec.h"
#include "wimax-channel.h"
#include "wimax-phy.h"

#include "ns3/propagation-loss-model.h"

#include <list>

namespace ns3 {

class SimpleOfdmSendParam {
public:
  SimpleOfdmSendParam();
  SimpleOfdmSendParam(const Bvec &fecBlock, uint32_t burstSize,
                      bool isFirstBlock, uint64_t Frequency,
                      WimaxPhy::ModulationType modulationType,
                      uint8_t direction, double rxPowerDbm);
  SimpleOfdmSendParam(uint32_t burstSize, bool isFirstBlock, uint64_t Frequency,
                      WimaxPhy::ModulationType modulationType,
                      uint8_t direction, double rxPowerDbm,
                      Ptr<PacketBurst> burst);
  ~SimpleOfdmSendParam();
  void SetFecBlock(const Bvec &fecBlock);
  void SetBurstSize(uint32_t burstSize);
  void SetIsFirstBlock(bool isFirstBlock);
  void SetFrequency(uint64_t Frequency);
  void SetModulationType(WimaxPhy::ModulationType modulationType);
  void SetDirection(uint8_t direction);
  void SetRxPowerDbm(double rxPowerDbm);
  Bvec GetFecBlock();
  uint32_t GetBurstSize() const;
  bool GetIsFirstBlock() const;
  uint64_t GetFrequency() const;
  WimaxPhy::ModulationType GetModulationType();
  uint8_t GetDirection() const;
  double GetRxPowerDbm() const;
  Ptr<PacketBurst> GetBurst();

private:
  Bvec m_fecBlock;
  uint32_t m_burstSize;
  bool m_isFirstBlock;
  uint64_t m_frequency;
  WimaxPhy::ModulationType m_modulationType;
  uint8_t m_direction;
  double m_rxPowerDbm;
  Ptr<PacketBurst> m_burst;
};
} // namespace ns3

#endif
