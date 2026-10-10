
#ifndef WIFI_RADIO_ENERGY_MODEL_HELPER_H
#define WIFI_RADIO_ENERGY_MODEL_HELPER_H

#include "ns3/energy-model-helper.h"
#include "ns3/wifi-radio-energy-model.h"

namespace ns3 {

class WifiRadioEnergyModelHelper : public DeviceEnergyModelHelper {
public:
  WifiRadioEnergyModelHelper();

  ~WifiRadioEnergyModelHelper() override;

  void Set(std::string name, const AttributeValue &v) override;

  void SetDepletionCallback(
      WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback);

  void SetRechargedCallback(
      WifiRadioEnergyModel::WifiRadioEnergyRechargedCallback callback);

  template <typename... Ts>
  void SetTxCurrentModel(std::string name, Ts &&...args);

private:
  Ptr<DeviceEnergyModel> DoInstall(Ptr<NetDevice> device,
                                   Ptr<EnergySource> source) const override;

private:
  ObjectFactory m_radioEnergy;
  WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback m_depletionCallback;
  WifiRadioEnergyModel::WifiRadioEnergyRechargedCallback m_rechargedCallback;
  ObjectFactory m_txCurrentModel;
};

template <typename... Ts>
void WifiRadioEnergyModelHelper::SetTxCurrentModel(std::string name,
                                                   Ts &&...args) {
  m_txCurrentModel = ObjectFactory(name, std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
