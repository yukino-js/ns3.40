
#include "rv-battery-model-helper.h"

#include "ns3/energy-source.h"

namespace ns3 {

RvBatteryModelHelper::RvBatteryModelHelper() {
  m_rvBatteryModel.SetTypeId("ns3::RvBatteryModel");
}

RvBatteryModelHelper::~RvBatteryModelHelper() {}

void RvBatteryModelHelper::Set(std::string name, const AttributeValue &v) {
  m_rvBatteryModel.Set(name, v);
}

Ptr<EnergySource> RvBatteryModelHelper::DoInstall(Ptr<Node> node) const {
  NS_ASSERT(node);
  Ptr<EnergySource> source = m_rvBatteryModel.Create<EnergySource>();
  NS_ASSERT(source);
  source->SetNode(node);
  return source;
}

} // namespace ns3
