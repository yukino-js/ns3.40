
#ifndef MESH_HELPER_H
#define MESH_HELPER_H

#include "mesh-stack-installer.h"

#include "ns3/object-factory.h"
#include "ns3/qos-utils.h"
#include "ns3/wifi-standards.h"

namespace ns3 {

class NetDeviceContainer;
class WifiPhyHelper;
class WifiNetDevice;
class NodeContainer;

class MeshHelper {
public:
  MeshHelper();

  ~MeshHelper();

  static MeshHelper Default();

  template <typename... Ts> void SetMacType(Ts &&...args);
  template <typename... Ts>
  void SetRemoteStationManager(std::string type, Ts &&...args);
  void SetStandard(WifiStandard standard);

  enum ChannelPolicy { SPREAD_CHANNELS, ZERO_CHANNEL };

  void SetSpreadInterfaceChannels(ChannelPolicy policy);
  void SetNumberOfInterfaces(uint32_t nInterfaces);

  NetDeviceContainer Install(const WifiPhyHelper &phyHelper,
                             NodeContainer c) const;
  template <typename... Ts>
  void SetStackInstaller(std::string type, Ts &&...args);

  void Report(const ns3::Ptr<ns3::NetDevice> &device, std::ostream &os);

  void ResetStats(const ns3::Ptr<ns3::NetDevice> &device);
  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

  static void EnableLogComponents();

private:
  Ptr<WifiNetDevice> CreateInterface(const WifiPhyHelper &phyHelper,
                                     Ptr<Node> node, uint16_t channelId) const;
  uint32_t m_nInterfaces;
  ChannelPolicy m_spreadChannelPolicy;
  Ptr<MeshStack> m_stack;
  ObjectFactory m_stackFactory;

  ObjectFactory m_mac;
  ObjectFactory m_stationManager;
  ObjectFactory m_ackPolicySelector[4];
  WifiStandard m_standard;
};

template <typename... Ts> void MeshHelper::SetMacType(Ts &&...args) {
  m_mac.SetTypeId("ns3::MeshWifiInterfaceMac");
  m_mac.Set(std::forward<Ts>(args)...);
}

template <typename... Ts>
void MeshHelper::SetRemoteStationManager(std::string type, Ts &&...args) {
  m_stationManager = ObjectFactory(type, std::forward<Ts>(args)...);
}

template <typename... Ts>
void MeshHelper::SetStackInstaller(std::string type, Ts &&...args) {
  m_stackFactory.SetTypeId(type);
  m_stackFactory.Set(std::forward<Ts>(args)...);
  m_stack = m_stackFactory.Create<MeshStack>();
  if (!m_stack) {
    NS_FATAL_ERROR("Stack has not been created: " << type);
  }
}

} // namespace ns3

#endif
