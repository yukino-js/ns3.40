
#ifndef BASIC_ENERGY_SOURCE_HELPER_H
#define BASIC_ENERGY_SOURCE_HELPER_H

#include "energy-model-helper.h"

#include "ns3/node.h"

namespace ns3 {

class BasicEnergySourceHelper : public EnergySourceHelper {
public:
  BasicEnergySourceHelper();
  ~BasicEnergySourceHelper() override;

  void Set(std::string name, const AttributeValue &v) override;

private:
  Ptr<EnergySource> DoInstall(Ptr<Node> node) const override;

private:
  ObjectFactory m_basicEnergySource;
};

} // namespace ns3

#endif
