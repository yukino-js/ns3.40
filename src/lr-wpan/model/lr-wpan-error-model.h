#ifndef LR_WPAN_ERROR_MODEL_H
#define LR_WPAN_ERROR_MODEL_H

#include <ns3/object.h>

namespace ns3 {

class LrWpanErrorModel : public Object {
public:
  static TypeId GetTypeId();

  LrWpanErrorModel();

  double GetChunkSuccessRate(double snr, uint32_t nbits) const;

private:
  double m_binomialCoefficients[17];
};

} // namespace ns3

#endif
