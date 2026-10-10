
#ifndef DEVICE_ENERGY_MODEL_H
#define DEVICE_ENERGY_MODEL_H

#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"

namespace ns3 {

class EnergySource;

class DeviceEnergyModel : public Object {
public:
  typedef Callback<void, int> ChangeStateCallback;

public:
  static TypeId GetTypeId();
  DeviceEnergyModel();
  ~DeviceEnergyModel() override;

  virtual void SetEnergySource(Ptr<EnergySource> source) = 0;

  virtual double GetTotalEnergyConsumption() const = 0;

  virtual void ChangeState(int newState) = 0;

  double GetCurrentA() const;

  virtual void HandleEnergyDepletion() = 0;

  virtual void HandleEnergyRecharged() = 0;

  virtual void HandleEnergyChanged() = 0;

private:
  virtual double DoGetCurrentA() const;
};

} // namespace ns3

#endif
