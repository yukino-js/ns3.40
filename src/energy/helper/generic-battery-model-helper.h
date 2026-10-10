
#ifndef GENERIC_BATTERY_MODEL_HELPER_H_
#define GENERIC_BATTERY_MODEL_HELPER_H_

#include "energy-model-helper.h"

#include <ns3/generic-battery-model.h>
#include <ns3/node.h>

namespace ns3 {

class GenericBatteryModelHelper : public EnergySourceHelper {
public:
  GenericBatteryModelHelper();
  ~GenericBatteryModelHelper() override;

  void Set(std::string name, const AttributeValue &v) override;

  Ptr<EnergySourceContainer> Install(NodeContainer c) const;

  Ptr<EnergySource> Install(Ptr<Node> node, BatteryModel bm) const;

  EnergySourceContainer Install(NodeContainer c, BatteryModel bm) const;

  void SetCellPack(Ptr<EnergySource> energySource, uint8_t series,
                   uint8_t parallel) const;

  void SetCellPack(EnergySourceContainer energySourceContainer, uint8_t series,
                   uint8_t parallel) const;

private:
  Ptr<EnergySource> DoInstall(Ptr<Node> node) const override;

private:
  ObjectFactory m_batteryModel;
};

} // namespace ns3

#endif
