
#ifndef THRESHOLD_PREAMBLE_DETECTION_MODEL_H
#define THRESHOLD_PREAMBLE_DETECTION_MODEL_H

#include "preamble-detection-model.h"

namespace ns3 {
class ThresholdPreambleDetectionModel : public PreambleDetectionModel {
public:
  static TypeId GetTypeId();

  ThresholdPreambleDetectionModel();
  ~ThresholdPreambleDetectionModel() override;

  bool IsPreambleDetected(double rssi, double snr,
                          double channelWidth) const override;

private:
  double m_threshold;
  double m_rssiMin;
};

} // namespace ns3

#endif
