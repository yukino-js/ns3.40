
#ifndef ACOUSTIC_MODEM_ENERGY_MODEL_HELPER_H
#define ACOUSTIC_MODEM_ENERGY_MODEL_HELPER_H

#include "ns3/acoustic-modem-energy-model.h"
#include "ns3/energy-model-helper.h"

namespace ns3 {

class AcousticModemEnergyModelHelper : public DeviceEnergyModelHelper {
public:
  AcousticModemEnergyModelHelper();

  ~AcousticModemEnergyModelHelper() override;

  void Set(std::string name, const AttributeValue &v) override;

  void SetDepletionCallback(
      AcousticModemEnergyModel::AcousticModemEnergyDepletionCallback callback);

private:
  Ptr<DeviceEnergyModel> DoInstall(Ptr<NetDevice> device,
                                   Ptr<EnergySource> source) const override;

private:
  ObjectFactory m_modemEnergy;

  AcousticModemEnergyModel::AcousticModemEnergyDepletionCallback
      m_depletionCallback;
};

} // namespace ns3

#endif
