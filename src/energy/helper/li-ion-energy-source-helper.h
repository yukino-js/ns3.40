
#ifndef LI_ION_ENERGY_SOURCE_HELPER_H_
#define LI_ION_ENERGY_SOURCE_HELPER_H_

#include "energy-model-helper.h"

#include "ns3/node.h"

namespace ns3 {

class LiIonEnergySourceHelper : public EnergySourceHelper {
public:
  LiIonEnergySourceHelper();
  ~LiIonEnergySourceHelper() override;

  void Set(std::string name, const AttributeValue &v) override;

private:
  Ptr<EnergySource> DoInstall(Ptr<Node> node) const override;

private:
  ObjectFactory m_liIonEnergySource;
};

} // namespace ns3

#endif
