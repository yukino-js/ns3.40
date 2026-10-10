
#ifndef NIST_ERROR_RATE_MODEL_H
#define NIST_ERROR_RATE_MODEL_H

#include "error-rate-model.h"
#include "wifi-mode.h"

namespace ns3 {

class NistErrorRateModel : public ErrorRateModel {
public:
  static TypeId GetTypeId();

  NistErrorRateModel();

private:
  double DoGetChunkSuccessRate(WifiMode mode, const WifiTxVector &txVector,
                               double snr, uint64_t nbits,
                               uint8_t numRxAntennas, WifiPpduField field,
                               uint16_t staId) const override;
  uint8_t GetBValue(WifiCodeRate codeRate) const;
  double CalculatePe(double p, uint8_t bValue) const;
  double GetBpskBer(double snr) const;
  double GetQpskBer(double snr) const;
  double GetQamBer(uint16_t constellationSize, double snr) const;
  double GetFecBpskBer(double snr, uint64_t nbits, uint8_t bValue) const;
  double GetFecQpskBer(double snr, uint64_t nbits, uint8_t bValue) const;
  double GetFecQamBer(uint16_t constellationSize, double snr, uint64_t nbits,
                      uint8_t bValue) const;
};

} // namespace ns3

#endif
