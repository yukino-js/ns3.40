
#ifndef UAN_HELPER_H
#define UAN_HELPER_H

#include "ns3/attribute.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/uan-net-device.h"

#include <string>

namespace ns3 {

class UanChannel;

class UanHelper {
public:
  UanHelper();
  virtual ~UanHelper();

  template <typename... Ts> void SetMac(std::string type, Ts &&...args);

  template <typename... Ts> void SetPhy(std::string phyType, Ts &&...args);

  template <typename... Ts> void SetTransducer(std::string type, Ts &&...args);
  static void EnableAscii(std::ostream &os, uint32_t nodeid, uint32_t deviceid);
  static void EnableAscii(std::ostream &os, NetDeviceContainer d);
  static void EnableAscii(std::ostream &os, NodeContainer n);
  static void EnableAsciiAll(std::ostream &os);

  NetDeviceContainer Install(NodeContainer c) const;

  NetDeviceContainer Install(NodeContainer c, Ptr<UanChannel> channel) const;

  Ptr<UanNetDevice> Install(Ptr<Node> node, Ptr<UanChannel> channel) const;

  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

private:
  ObjectFactory m_device;
  ObjectFactory m_mac;
  ObjectFactory m_phy;
  ObjectFactory m_transducer;
};

template <typename... Ts>
void UanHelper::SetMac(std::string type, Ts &&...args) {
  m_mac = ObjectFactory(type, std::forward<Ts>(args)...);
}

template <typename... Ts>
void UanHelper::SetPhy(std::string phyType, Ts &&...args) {
  m_phy = ObjectFactory(phyType, std::forward<Ts>(args)...);
}

template <typename... Ts>
void UanHelper::SetTransducer(std::string type, Ts &&...args) {
  m_transducer = ObjectFactory(type, std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
