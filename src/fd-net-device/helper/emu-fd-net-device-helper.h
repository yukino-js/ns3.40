
#ifndef EMU_FD_NET_DEVICE_HELPER_H
#define EMU_FD_NET_DEVICE_HELPER_H

#include "fd-net-device-helper.h"

#include "ns3/attribute.h"
#include "ns3/fd-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <string>

namespace ns3 {

class EmuFdNetDeviceHelper : public FdNetDeviceHelper {
public:
  EmuFdNetDeviceHelper();

  ~EmuFdNetDeviceHelper() override {}

  std::string GetDeviceName();

  void SetDeviceName(std::string deviceName);

  void HostQdiscBypass(bool hostQdiscBypass);

protected:
  Ptr<NetDevice> InstallPriv(Ptr<Node> node) const override;

  virtual void SetFileDescriptor(Ptr<FdNetDevice> device) const;

  virtual int CreateFileDescriptor() const;

  std::string m_deviceName;
  bool m_hostQdiscBypass;
};

} // namespace ns3

#endif
