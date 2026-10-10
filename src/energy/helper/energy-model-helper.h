
#ifndef ENERGY_MODEL_HELPER_H
#define ENERGY_MODEL_HELPER_H

#include "energy-source-container.h"

#include "ns3/attribute.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/device-energy-model.h"
#include "ns3/energy-source.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ptr.h"

namespace ns3 {

class EnergySourceHelper {
public:
  virtual ~EnergySourceHelper();

  virtual void Set(std::string name, const AttributeValue &v) = 0;

  EnergySourceContainer Install(Ptr<Node> node) const;

  EnergySourceContainer Install(NodeContainer c) const;

  EnergySourceContainer Install(std::string nodeName) const;

  EnergySourceContainer InstallAll() const;

private:
  virtual Ptr<EnergySource> DoInstall(Ptr<Node> node) const = 0;
};

class DeviceEnergyModelHelper {
public:
  virtual ~DeviceEnergyModelHelper();

  virtual void Set(std::string name, const AttributeValue &v) = 0;

  DeviceEnergyModelContainer Install(Ptr<NetDevice> device,
                                     Ptr<EnergySource> source) const;

  DeviceEnergyModelContainer
  Install(NetDeviceContainer deviceContainer,
          EnergySourceContainer sourceContainer) const;

private:
  virtual Ptr<DeviceEnergyModel> DoInstall(Ptr<NetDevice> device,
                                           Ptr<EnergySource> source) const = 0;
};

} // namespace ns3

#endif
