
#ifndef NETMAP_NET_DEVICE_HELPER_H
#define NETMAP_NET_DEVICE_HELPER_H

#include "fd-net-device-helper.h"

#include "ns3/attribute.h"
#include "ns3/fd-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/netmap-net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <string>

namespace ns3 {

class NetmapNetDevice;

class NetmapNetDeviceHelper : public FdNetDeviceHelper {
public:
  NetmapNetDeviceHelper();

  virtual ~NetmapNetDeviceHelper() {}

  std::string GetDeviceName();

  void SetDeviceName(std::string deviceName);

protected:
  Ptr<NetDevice> InstallPriv(Ptr<Node> node) const;

  virtual void SetDeviceAttributes(Ptr<FdNetDevice> device) const;

  virtual int CreateFileDescriptor() const;

  void SwitchInNetmapMode(int fd, Ptr<NetmapNetDevice> device) const;

  std::string m_deviceName;
};

} // namespace ns3

#endif
