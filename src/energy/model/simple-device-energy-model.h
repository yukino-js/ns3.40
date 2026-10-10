
#ifndef SIMPLE_DEVICE_ENERGY_MODEL_H
#define SIMPLE_DEVICE_ENERGY_MODEL_H

#include "device-energy-model.h"

#include "ns3/nstime.h"
#include "ns3/traced-value.h"

namespace ns3 {

class SimpleDeviceEnergyModel : public DeviceEnergyModel {
public:
  static TypeId GetTypeId();
  SimpleDeviceEnergyModel();
  ~SimpleDeviceEnergyModel() override;

  virtual void SetNode(Ptr<Node> node);

  virtual Ptr<Node> GetNode() const;

  void SetEnergySource(Ptr<EnergySource> source) override;

  double GetTotalEnergyConsumption() const override;

  void ChangeState(int newState) override {}

  void HandleEnergyDepletion() override {}

  void HandleEnergyRecharged() override {}

  void HandleEnergyChanged() override {}

  void SetCurrentA(double current);

private:
  void DoDispose() override;

  double DoGetCurrentA() const override;

  Time m_lastUpdateTime;
  double m_actualCurrentA;
  Ptr<EnergySource> m_source;
  Ptr<Node> m_node;
  TracedValue<double> m_totalEnergyConsumption;
};

} // namespace ns3

#endif
