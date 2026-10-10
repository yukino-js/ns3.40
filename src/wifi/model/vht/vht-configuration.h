
#ifndef VHT_CONFIGURATION_H
#define VHT_CONFIGURATION_H

#include "ns3/object.h"

#include <map>
#include <tuple>

namespace ns3 {

class VhtConfiguration : public Object {
public:
  VhtConfiguration();
  ~VhtConfiguration() override;

  static TypeId GetTypeId();

  void Set160MHzOperationSupported(bool enable);
  bool Get160MHzOperationSupported() const;

  using SecondaryCcaSensitivityThresholds = std::tuple<double, double, double>;

  void SetSecondaryCcaSensitivityThresholds(
      const SecondaryCcaSensitivityThresholds &thresholds);
  SecondaryCcaSensitivityThresholds
  GetSecondaryCcaSensitivityThresholds() const;

  const std::map<uint16_t, double> &
  GetSecondaryCcaSensitivityThresholdsPerBw() const;

private:
  bool m_160MHzSupported;
  std::map<uint16_t, double> m_secondaryCcaSensitivityThresholds;
};

} // namespace ns3

#endif
