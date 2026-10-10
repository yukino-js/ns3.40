
#ifndef YANS_ERROR_RATE_MODEL_H
#define YANS_ERROR_RATE_MODEL_H

#include "error-rate-model.h"

namespace ns3 {

class YansErrorRateModel : public ErrorRateModel {
public:
  static TypeId GetTypeId();

  YansErrorRateModel();

private:
  double DoGetChunkSuccessRate(WifiMode mode, const WifiTxVector &txVector,
                               double snr, uint64_t nbits,
                               uint8_t numRxAntennas, WifiPpduField field,
                               uint16_t staId) const override;
  double GetBpskBer(double snr, uint32_t signalSpread, uint64_t phyRate) const;
  double GetQamBer(double snr, unsigned int m, uint32_t signalSpread,
                   uint64_t phyRate) const;
  uint32_t Factorial(uint32_t k) const;
  double Binomial(uint32_t k, double p, uint32_t n) const;
  double CalculatePdOdd(double ber, unsigned int d) const;
  double CalculatePdEven(double ber, unsigned int d) const;
  double CalculatePd(double ber, unsigned int d) const;
  double GetFecBpskBer(double snr, uint64_t nbits, uint32_t signalSpread,
                       uint64_t phyRate, uint32_t dFree, uint32_t adFree) const;
  double GetFecQamBer(double snr, uint64_t nbits, uint32_t signalSpread,
                      uint64_t phyRate, uint32_t m, uint32_t dfree,
                      uint32_t adFree, uint32_t adFreePlusOne) const;
};

} // namespace ns3

#endif
