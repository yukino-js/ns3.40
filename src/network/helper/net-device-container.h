#ifndef NET_DEVICE_CONTAINER_H
#define NET_DEVICE_CONTAINER_H

#include "ns3/net-device.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class NetDeviceContainer {
public:
  typedef std::vector<Ptr<NetDevice>>::const_iterator Iterator;

  NetDeviceContainer();

  NetDeviceContainer(Ptr<NetDevice> dev);

  NetDeviceContainer(std::string devName);

  NetDeviceContainer(const NetDeviceContainer &a, const NetDeviceContainer &b);

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ptr<NetDevice> Get(uint32_t i) const;

  void Add(NetDeviceContainer other);

  void Add(Ptr<NetDevice> device);

  void Add(std::string deviceName);

private:
  std::vector<Ptr<NetDevice>> m_devices;
};

} // namespace ns3

#endif
