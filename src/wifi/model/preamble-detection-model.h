
#ifndef PREAMBLE_DETECTION_MODEL_H
#define PREAMBLE_DETECTION_MODEL_H

#include "ns3/object.h"

namespace ns3 {

class PreambleDetectionModel : public Object {
public:
  static TypeId GetTypeId();

  virtual bool IsPreambleDetected(double rssi, double snr,
                                  double channelWidth) const = 0;
};

} // namespace ns3

#endif
