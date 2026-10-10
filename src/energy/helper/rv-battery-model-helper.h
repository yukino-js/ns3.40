
#ifndef RV_BATTERY_MODEL_HELPER_H
#define RV_BATTERY_MODEL_HELPER_H

#include "energy-model-helper.h"

#include "ns3/node.h"

namespace ns3 {

class RvBatteryModelHelper : public EnergySourceHelper {
public:
  RvBatteryModelHelper();
  ~RvBatteryModelHelper() override;

  void Set(std::string name, const AttributeValue &v) override;

private:
  Ptr<EnergySource> DoInstall(Ptr<Node> node) const override;

private:
  ObjectFactory m_rvBatteryModel;
};

} // namespace ns3

#endif
