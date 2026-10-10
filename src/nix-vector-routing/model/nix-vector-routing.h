
#ifndef NIX_VECTOR_ROUTING_H
#define NIX_VECTOR_ROUTING_H

#include "ns3/bridge-net-device.h"
#include "ns3/channel.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/net-device-container.h"
#include "ns3/nix-vector.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/nstime.h"

#include <atomic>
#include <map>
#include <unordered_map>

// NOLINTBEGIN(modernize-use-override)

namespace ns3 {

template <typename T>
class NixVectorRouting
    : public std::enable_if_t<std::is_same_v<Ipv4RoutingProtocol, T> ||
                                  std::is_same_v<Ipv6RoutingProtocol, T>,
                              T> {
  static constexpr bool IsIpv4 = std::is_same_v<Ipv4RoutingProtocol, T>;

  using Ip = typename std::conditional_t<IsIpv4, Ipv4, Ipv6>;

  using IpAddress =
      typename std::conditional_t<IsIpv4, Ipv4Address, Ipv6Address>;

  using IpRoute = typename std::conditional_t<IsIpv4, Ipv4Route, Ipv6Route>;

  using IpAddressHash =
      typename std::conditional_t<IsIpv4, Ipv4AddressHash, Ipv6AddressHash>;

  using IpHeader = typename std::conditional_t<IsIpv4, Ipv4Header, Ipv6Header>;

  using IpInterfaceAddress =
      typename std::conditional_t<IsIpv4, Ipv4InterfaceAddress,
                                  Ipv6InterfaceAddress>;

  using IpInterface =
      typename std::conditional_t<IsIpv4, Ipv4Interface, Ipv6Interface>;

  using IpL3Protocol =
      typename std::conditional_t<IsIpv4, Ipv4L3Protocol, Ipv6L3Protocol>;

public:
  NixVectorRouting();
  ~NixVectorRouting();
  static TypeId GetTypeId();
  void SetNode(Ptr<Node> node);

  void FlushGlobalNixRoutingCache() const;

  void PrintRoutingPath(Ptr<Node> source, IpAddress dest,
                        Ptr<OutputStreamWrapper> stream, Time::Unit unit) const;

private:
  void FlushNixCache() const;

  void FlushIpRouteCache() const;

  void ResetTotalNeighbors();

  Ptr<NixVector> GetNixVector(Ptr<Node> source, IpAddress dest,
                              Ptr<NetDevice> oif) const;

  Ptr<NixVector> GetNixVectorInCache(const IpAddress &address,
                                     bool &foundInCache) const;

  Ptr<IpRoute> GetIpRouteInCache(IpAddress address);

  void GetAdjacentNetDevices(Ptr<NetDevice> netDevice, Ptr<Channel> channel,
                             NetDeviceContainer &netDeviceContainer) const;

  Ptr<Node> GetNodeByIp(IpAddress dest) const;

  Ptr<IpInterface> GetInterfaceByNetDevice(Ptr<NetDevice> netDevice) const;

  bool BuildNixVector(const std::vector<Ptr<Node>> &parentVector,
                      uint32_t source, uint32_t dest,
                      Ptr<NixVector> nixVector) const;

  uint32_t FindTotalNeighbors(Ptr<Node> node) const;

  Ptr<BridgeNetDevice> NetDeviceIsBridged(Ptr<NetDevice> nd) const;

  uint32_t FindNetDeviceForNixIndex(Ptr<Node> node, uint32_t nodeIndex,
                                    IpAddress &gatewayIp) const;

  bool BFS(uint32_t numberOfNodes, Ptr<Node> source, Ptr<Node> dest,
           std::vector<Ptr<Node>> &parentVector, Ptr<NetDevice> oif) const;

  void DoInitialize();

  void DoDispose();

  typedef std::map<IpAddress, Ptr<NixVector>> NixMap_t;
  typedef std::map<IpAddress, Ptr<IpRoute>> IpRouteMap_t;

  typedef Callback<void, Ptr<IpRoute>, Ptr<const Packet>, const IpHeader &>
      UnicastForwardCallbackv4;

  typedef Callback<void, Ptr<const NetDevice>, Ptr<IpRoute>, Ptr<const Packet>,
                   const IpHeader &>
      UnicastForwardCallbackv6;

  typedef typename std::conditional_t<IsIpv4, UnicastForwardCallbackv4,
                                      UnicastForwardCallbackv6>
      UnicastForwardCallback;

  typedef Callback<void, Ptr<Ipv4MulticastRoute>, Ptr<const Packet>,
                   const IpHeader &>
      MulticastForwardCallbackv4;

  typedef Callback<void, Ptr<const NetDevice>, Ptr<Ipv6MulticastRoute>,
                   Ptr<const Packet>, const IpHeader &>
      MulticastForwardCallbackv6;

  typedef typename std::conditional_t<IsIpv4, MulticastForwardCallbackv4,
                                      MulticastForwardCallbackv6>
      MulticastForwardCallback;

  typedef Callback<void, Ptr<const Packet>, const IpHeader &, uint32_t>
      LocalDeliverCallback;

  typedef Callback<void, Ptr<const Packet>, const IpHeader &,
                   Socket::SocketErrno>
      ErrorCallback;

  virtual Ptr<IpRoute> RouteOutput(Ptr<Packet> p, const IpHeader &header,
                                   Ptr<NetDevice> oif,
                                   Socket::SocketErrno &sockerr);

  virtual bool RouteInput(Ptr<const Packet> p, const IpHeader &header,
                          Ptr<const NetDevice> idev,
                          const UnicastForwardCallback &ucb,
                          const MulticastForwardCallback &mcb,
                          const LocalDeliverCallback &lcb,
                          const ErrorCallback &ecb);

  virtual void NotifyInterfaceUp(uint32_t interface);

  virtual void NotifyInterfaceDown(uint32_t interface);

  virtual void NotifyAddAddress(uint32_t interface, IpInterfaceAddress address);

  virtual void NotifyRemoveAddress(uint32_t interface,
                                   IpInterfaceAddress address);

  virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                                 Time::Unit unit = Time::S) const;

  virtual void SetIpv4(Ptr<Ip> ipv4);

  virtual void SetIpv6(Ptr<Ip> ipv6);

  virtual void NotifyAddRoute(IpAddress dst, Ipv6Prefix mask, IpAddress nextHop,
                              uint32_t interface,
                              IpAddress prefixToUse = IpAddress::GetZero());

  virtual void NotifyRemoveRoute(IpAddress dst, Ipv6Prefix mask,
                                 IpAddress nextHop, uint32_t interface,
                                 IpAddress prefixToUse = IpAddress::GetZero());

  void CheckCacheStateAndFlush() const;

  void BuildIpAddressToNodeMap() const;

#ifdef NS3_MTP
  static std::atomic<bool> g_isCacheDirty;
  static std::atomic<bool> g_cacheFlushing;
  static std::atomic<bool> g_isMapBuilt;
  static std::atomic<bool> g_mapBuilding;
#else
  static bool g_isCacheDirty;
#endif

  static uint32_t g_epoch;

  mutable NixMap_t m_nixCache;

  mutable IpRouteMap_t m_ipRouteCache;

  Ptr<Ip> m_ip;
  Ptr<Node> m_node;

  uint32_t m_totalNeighbors;

  typedef std::unordered_map<IpAddress, ns3::Ptr<ns3::Node>, IpAddressHash>
      IpAddressToNodeMap;
  static IpAddressToNodeMap g_ipAddressToNodeMap;

  typedef std::unordered_map<Ptr<NetDevice>, Ptr<IpInterface>>
      NetDeviceToIpInterfaceMap;
  static NetDeviceToIpInterfaceMap g_netdeviceToIpInterfaceMap;
};

typedef NixVectorRouting<Ipv4RoutingProtocol> Ipv4NixVectorRouting;

typedef NixVectorRouting<Ipv6RoutingProtocol> Ipv6NixVectorRouting;
} // namespace ns3

// NOLINTEND(modernize-use-override)

#endif
