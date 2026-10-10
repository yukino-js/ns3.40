
#ifndef ERROR_RATE_MODEL_H
#define ERROR_RATE_MODEL_H

#include "wifi-mode.h"

#include "ns3/object.h"

namespace ns3 {

class ErrorRateModel : public Object {
public:
  static TypeId GetTypeId();

  double CalculateSnr(const WifiTxVector &txVector, double ber) const;

  virtual bool IsAwgn() const;

  double GetChunkSuccessRate(WifiMode mode, const WifiTxVector &txVector,
                             double snr, uint64_t nbits,
                             uint8_t numRxAntennas = 1,
                             WifiPpduField field = WIFI_PPDU_FIELD_DATA,
                             uint16_t staId = SU_STA_ID) const;

  virtual int64_t AssignStreams(int64_t stream);

private:
  virtual double DoGetChunkSuccessRate(WifiMode mode,
                                       const WifiTxVector &txVector, double snr,
                                       uint64_t nbits, uint8_t numRxAntennas,
                                       WifiPpduField field,
                                       uint16_t staId) const = 0;
};

} // namespace ns3

#endif
