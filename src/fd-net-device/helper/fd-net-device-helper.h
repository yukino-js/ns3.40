
#ifndef FD_NET_DEVICE_HELPER_H
#define FD_NET_DEVICE_HELPER_H

#include "ns3/attribute.h"
#include "ns3/fd-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/trace-helper.h"

#include <string>

namespace ns3 {

class FdNetDeviceHelper : public PcapHelperForDevice,
                          public AsciiTraceHelperForDevice {
public:
  FdNetDeviceHelper();

  ~FdNetDeviceHelper() override {}

  void SetTypeId(std::string type);

  void SetAttribute(std::string n1, const AttributeValue &v1);

  virtual NetDeviceContainer Install(Ptr<Node> node) const;

  virtual NetDeviceContainer Install(std::string name) const;

  virtual NetDeviceContainer Install(const NodeContainer &c) const;

protected:
  virtual Ptr<NetDevice> InstallPriv(Ptr<Node> node) const;

private:
  void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                          bool promiscuous, bool explicitFilename) override;

  void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<NetDevice> nd, bool explicitFilename) override;

  ObjectFactory m_deviceFactory;
};

} // namespace ns3

#endif
