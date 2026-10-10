
#ifndef DEVICE_ENERGY_MODEL_CONTAINER_H
#define DEVICE_ENERGY_MODEL_CONTAINER_H

#include "device-energy-model.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class DeviceEnergyModelContainer {
public:
  typedef std::vector<Ptr<DeviceEnergyModel>>::const_iterator Iterator;

public:
  DeviceEnergyModelContainer();

  DeviceEnergyModelContainer(Ptr<DeviceEnergyModel> model);

  DeviceEnergyModelContainer(std::string modelName);

  DeviceEnergyModelContainer(const DeviceEnergyModelContainer &a,
                             const DeviceEnergyModelContainer &b);

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ptr<DeviceEnergyModel> Get(uint32_t i) const;

  void Add(DeviceEnergyModelContainer container);

  void Add(Ptr<DeviceEnergyModel> model);

  void Add(std::string modelName);

  void Clear();

private:
  std::vector<Ptr<DeviceEnergyModel>> m_models;
};

} // namespace ns3

#endif
