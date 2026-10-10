
#ifndef SNR_TO_BLOCK_ERROR_RATE_MANAGER_H
#define SNR_TO_BLOCK_ERROR_RATE_MANAGER_H

#include "snr-to-block-error-rate-record.h"

#include "ns3/ptr.h"

#include <vector>

namespace ns3 {

class SNRToBlockErrorRateManager {
public:
  SNRToBlockErrorRateManager();
  ~SNRToBlockErrorRateManager();
  void SetTraceFilePath(char *traceFilePath);
  std::string GetTraceFilePath();
  double GetBlockErrorRate(double SNR, uint8_t modulation);
  SNRToBlockErrorRateRecord *GetSNRToBlockErrorRateRecord(double SNR,
                                                          uint8_t modulation);

  void LoadTraces();
  void LoadDefaultTraces();
  void ReLoadTraces();
  void ActivateLoss(bool loss);

private:
  void ClearRecords();
  bool m_activateLoss;
  std::string m_traceFilePath;

  std::vector<SNRToBlockErrorRateRecord *> *m_recordModulation[7];
};
} // namespace ns3

#endif
