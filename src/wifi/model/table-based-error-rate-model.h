
#ifndef TABLE_BASED_ERROR_RATE_MODEL_H
#define TABLE_BASED_ERROR_RATE_MODEL_H

#include "error-rate-model.h"
#include "wifi-mode.h"

#include "ns3/error-rate-tables.h"

#include <optional>

namespace ns3 {

class WifiTxVector;

class TableBasedErrorRateModel : public ErrorRateModel {
public:
  static TypeId GetTypeId();

  TableBasedErrorRateModel();
  ~TableBasedErrorRateModel() override;

  static std::optional<uint8_t> GetMcsForMode(WifiMode mode);

private:
  double DoGetChunkSuccessRate(WifiMode mode, const WifiTxVector &txVector,
                               double snr, uint64_t nbits,
                               uint8_t numRxAntennas, WifiPpduField field,
                               uint16_t staId) const override;

  double RoundSnr(double snr, double precision) const;

  double FetchFsr(WifiMode mode, const WifiTxVector &txVector, double snr,
                  uint64_t nbits) const;

  Ptr<ErrorRateModel> m_fallbackErrorModel;

  uint64_t m_threshold;
};

} // namespace ns3

#endif
