
#include "nix-vector-routing.h"

#include "ns3/abort.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/log.h"
#include "ns3/loopback-net-device.h"
#include "ns3/names.h"

#include <iomanip>
#include <queue>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NixVectorRouting");

NS_OBJECT_TEMPLATE_CLASS_DEFINE(NixVectorRouting, Ipv4RoutingProtocol);
NS_OBJECT_TEMPLATE_CLASS_DEFINE(NixVectorRouting, Ipv6RoutingProtocol);

#ifdef NS3_MTP
template <typename T>
std::atomic<bool> NixVectorRouting<T>::g_isCacheDirty(false);

template <typename T>
std::atomic<bool> NixVectorRouting<T>::g_cacheFlushing(false);

template <typename T>
std::atomic<bool> NixVectorRouting<T>::g_isMapBuilt(false);

template <typename T>
std::atomic<bool> NixVectorRouting<T>::g_mapBuilding(false);
#else
template <typename T> bool NixVectorRouting<T>::g_isCacheDirty = false;
#endif

template <typename T> uint32_t NixVectorRouting<T>::g_epoch = 1;

template <typename T>
typename NixVectorRouting<T>::IpAddressToNodeMap
    NixVectorRouting<T>::g_ipAddressToNodeMap;

template <typename T>
typename NixVectorRouting<T>::NetDeviceToIpInterfaceMap
    NixVectorRouting<T>::g_netdeviceToIpInterfaceMap;

template <typename T> TypeId NixVectorRouting<T>::GetTypeId() {
  std::string name;
  if constexpr (std::is_same_v<T, Ipv4RoutingProtocol>) {
    name = "Ipv4";
  } else {
    name = "Ipv6";
  }
  static TypeId tid = TypeId(("ns3::" + name + "NixVectorRouting"))
                          .SetParent<T>()
                          .SetGroupName("NixVectorRouting")
                          .template AddConstructor<NixVectorRouting<T>>();
  return tid;
}

template <typename T>
NixVectorRouting<T>::NixVectorRouting() : m_totalNeighbors(0) {
  NS_LOG_FUNCTION_NOARGS();
}

template <typename T> NixVectorRouting<T>::~NixVectorRouting() {
  NS_LOG_FUNCTION_NOARGS();
}

template <typename T> void NixVectorRouting<T>::SetIpv4(Ptr<Ip> ipv4) {
  NS_ASSERT(ipv4);
  NS_ASSERT(!m_ip);
  NS_LOG_DEBUG("Created Ipv4NixVectorProtocol");

  m_ip = ipv4;
}

template <typename T> void NixVectorRouting<T>::SetIpv6(Ptr<Ip> ipv6) {
  NS_ASSERT(ipv6);
  NS_ASSERT(!m_ip);
  NS_LOG_DEBUG("Created Ipv6NixVectorProtocol");

  m_ip = ipv6;
}

template <typename T> void NixVectorRouting<T>::DoInitialize() {
  NS_LOG_FUNCTION(this);

  for (uint32_t i = 0; i < m_ip->GetNInterfaces(); i++) {
    m_ip->SetForwarding(i, true);
  }

  T::DoInitialize();
}

template <typename T> void NixVectorRouting<T>::DoDispose() {
  NS_LOG_FUNCTION_NOARGS();

  m_node = nullptr;
  m_ip = nullptr;

  T::DoDispose();
}

template <typename T> void NixVectorRouting<T>::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION_NOARGS();

  m_node = node;
}

template <typename T>
void NixVectorRouting<T>::FlushGlobalNixRoutingCache() const {
  NS_LOG_FUNCTION_NOARGS();

  for (auto i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    Ptr<NixVectorRouting<T>> rp = node->GetObject<NixVectorRouting>();
    if (!rp) {
      continue;
    }
    NS_LOG_LOGIC("Flushing Nix caches.");
    rp->FlushNixCache();
    rp->FlushIpRouteCache();
    rp->m_totalNeighbors = 0;
  }

  g_ipAddressToNodeMap.clear();
#ifdef NS3_MTP
  g_isMapBuilt.store(false, std::memory_order_release);
#endif
}

template <typename T> void NixVectorRouting<T>::FlushNixCache() const {
  NS_LOG_FUNCTION_NOARGS();
  m_nixCache.clear();
}

template <typename T> void NixVectorRouting<T>::FlushIpRouteCache() const {
  NS_LOG_FUNCTION_NOARGS();
  m_ipRouteCache.clear();
}

template <typename T>
Ptr<NixVector> NixVectorRouting<T>::GetNixVector(Ptr<Node> source,
                                                 IpAddress dest,
                                                 Ptr<NetDevice> oif) const {
  NS_LOG_FUNCTION(this << source << dest << oif);

  Ptr<NixVector> nixVector = Create<NixVector>();
  nixVector->SetEpoch(g_epoch);

  Ptr<Node> destNode = GetNodeByIp(dest);
  if (!destNode) {
    NS_LOG_ERROR("No routing path exists");
    return nullptr;
  }

  if (source == destNode) {
    NS_LOG_DEBUG("Do not process packets to self");
    return nullptr;
  } else {
    std::vector<Ptr<Node>> parentVector;

    if (BFS(NodeList::GetNNodes(), source, destNode, parentVector, oif)) {
      if (BuildNixVector(parentVector, source->GetId(), destNode->GetId(),
                         nixVector)) {
        return nixVector;
      } else {
        NS_LOG_ERROR("No routing path exists");
        return nullptr;
      }
    } else {
      NS_LOG_ERROR("No routing path exists");
      return nullptr;
    }
  }
}

template <typename T>
Ptr<NixVector>
NixVectorRouting<T>::GetNixVectorInCache(const IpAddress &address,
                                         bool &foundInCache) const {
  NS_LOG_FUNCTION(this << address);

  CheckCacheStateAndFlush();

  auto iter = m_nixCache.find(address);
  if (iter != m_nixCache.end()) {
    NS_LOG_LOGIC("Found Nix-vector in cache.");
    foundInCache = true;
    return iter->second;
  }

  foundInCache = false;
  return nullptr;
}

template <typename T>
Ptr<typename NixVectorRouting<T>::IpRoute>
NixVectorRouting<T>::GetIpRouteInCache(IpAddress address) {
  NS_LOG_FUNCTION(this << address);

  CheckCacheStateAndFlush();

  auto iter = m_ipRouteCache.find(address);
  if (iter != m_ipRouteCache.end()) {
    NS_LOG_LOGIC("Found IpRoute in cache.");
    return iter->second;
  }

  return nullptr;
}

template <typename T>
bool NixVectorRouting<T>::BuildNixVector(
    const std::vector<Ptr<Node>> &parentVector, uint32_t source, uint32_t dest,
    Ptr<NixVector> nixVector) const {
  NS_LOG_FUNCTION(this << parentVector << source << dest << nixVector);

  if (source == dest) {
    return true;
  }

  if (!parentVector.at(dest)) {
    return false;
  }

  Ptr<Node> parentNode = parentVector.at(dest);

  uint32_t numberOfDevices = parentNode->GetNDevices();
  uint32_t destId = 0;
  uint32_t totalNeighbors = 0;

  for (uint32_t i = 0; i < numberOfDevices; i++) {
    Ptr<NetDevice> localNetDevice = parentNode->GetDevice(i);
    if (localNetDevice->IsBridge()) {
      continue;
    }
    Ptr<Channel> channel = localNetDevice->GetChannel();
    if (!channel) {
      continue;
    }

    NetDeviceContainer netDeviceContainer;
    GetAdjacentNetDevices(localNetDevice, channel, netDeviceContainer);

    uint32_t offset = 0;
    for (auto iter = netDeviceContainer.Begin();
         iter != netDeviceContainer.End(); iter++) {
      Ptr<Node> remoteNode = (*iter)->GetNode();

      if (remoteNode->GetId() == dest) {
        destId = totalNeighbors + offset;
      }
      offset += 1;
    }

    totalNeighbors += netDeviceContainer.GetN();
  }
  NS_LOG_LOGIC("Adding Nix: " << destId << " with "
                              << nixVector->BitCount(totalNeighbors)
                              << " bits, for node " << parentNode->GetId());
  nixVector->AddNeighborIndex(destId, nixVector->BitCount(totalNeighbors));

  BuildNixVector(parentVector, source, (parentVector.at(dest))->GetId(),
                 nixVector);
  return true;
}

template <typename T>
void NixVectorRouting<T>::GetAdjacentNetDevices(
    Ptr<NetDevice> netDevice, Ptr<Channel> channel,
    NetDeviceContainer &netDeviceContainer) const {
  NS_LOG_FUNCTION(this << netDevice << channel);

  Ptr<IpInterface> netDeviceInterface = GetInterfaceByNetDevice(netDevice);
  if (!netDeviceInterface || !netDeviceInterface->IsUp()) {
    NS_LOG_LOGIC("IpInterface either doesn't exist or is down");
    return;
  }

  uint32_t netDeviceAddresses = netDeviceInterface->GetNAddresses();

  for (std::size_t i = 0; i < channel->GetNDevices(); i++) {
    Ptr<NetDevice> remoteDevice = channel->GetDevice(i);
    if (remoteDevice != netDevice) {
      Ptr<IpInterface> remoteDeviceInterface =
          GetInterfaceByNetDevice(remoteDevice);
      if (!remoteDeviceInterface || !remoteDeviceInterface->IsUp()) {
        NS_LOG_LOGIC("IpInterface either doesn't exist or is down");
        continue;
      }

      uint32_t remoteDeviceAddresses = remoteDeviceInterface->GetNAddresses();
      bool commonSubnetFound = false;

      for (uint32_t j = 0; j < netDeviceAddresses; ++j) {
        IpInterfaceAddress netDeviceIfAddr = netDeviceInterface->GetAddress(j);
        if constexpr (!IsIpv4) {
          if (netDeviceIfAddr.GetScope() == Ipv6InterfaceAddress::LINKLOCAL) {
            continue;
          }
        }
        for (uint32_t k = 0; k < remoteDeviceAddresses; ++k) {
          IpInterfaceAddress remoteDeviceIfAddr =
              remoteDeviceInterface->GetAddress(k);
          if constexpr (!IsIpv4) {
            if (remoteDeviceIfAddr.GetScope() ==
                Ipv6InterfaceAddress::LINKLOCAL) {
              continue;
            }
          }
          if (netDeviceIfAddr.IsInSameSubnet(remoteDeviceIfAddr.GetAddress())) {
            commonSubnetFound = true;
            break;
          }
        }

        if (commonSubnetFound) {
          break;
        }
      }

      if (!commonSubnetFound) {
        continue;
      }

      Ptr<BridgeNetDevice> bd = NetDeviceIsBridged(remoteDevice);
      if (bd) {
        NS_LOG_LOGIC("Looking through bridge ports of bridge net device "
                     << bd);
        for (uint32_t j = 0; j < bd->GetNBridgePorts(); ++j) {
          Ptr<NetDevice> ndBridged = bd->GetBridgePort(j);
          if (ndBridged == remoteDevice) {
            NS_LOG_LOGIC("That bridge port is me, don't walk backward");
            continue;
          }
          Ptr<Channel> chBridged = ndBridged->GetChannel();
          if (!chBridged) {
            continue;
          }
          GetAdjacentNetDevices(ndBridged, chBridged, netDeviceContainer);
        }
      } else {
        netDeviceContainer.Add(channel->GetDevice(i));
      }
    }
  }
}

template <typename T>
void NixVectorRouting<T>::BuildIpAddressToNodeMap() const {
  NS_LOG_FUNCTION_NOARGS();

  for (auto it = NodeList::Begin(); it != NodeList::End(); ++it) {
    Ptr<Node> node = *it;
    Ptr<IpL3Protocol> ip = node->GetObject<IpL3Protocol>();

    if (ip) {
      uint32_t numberOfDevices = node->GetNDevices();

      for (uint32_t deviceId = 0; deviceId < numberOfDevices; deviceId++) {
        Ptr<NetDevice> device = node->GetDevice(deviceId);

        if (!DynamicCast<LoopbackNetDevice>(device)) {
          int32_t interfaceIndex =
              (ip)->GetInterfaceForDevice(node->GetDevice(deviceId));
          if (interfaceIndex != -1) {
            g_netdeviceToIpInterfaceMap[device] =
                (ip)->GetInterface(interfaceIndex);

            uint32_t numberOfAddresses = ip->GetNAddresses(interfaceIndex);
            for (uint32_t addressIndex = 0; addressIndex < numberOfAddresses;
                 addressIndex++) {
              IpInterfaceAddress ifAddr =
                  ip->GetAddress(interfaceIndex, addressIndex);
              IpAddress addr = ifAddr.GetAddress();

              NS_ABORT_MSG_IF(
                  g_ipAddressToNodeMap.count(addr),
                  "Duplicate IP address ("
                      << addr
                      << ") found during NIX Vector map construction for node "
                      << node->GetId());

              NS_LOG_LOGIC("Adding IP address "
                           << addr << " for node " << node->GetId()
                           << " to NIX Vector IP address to node map");
              g_ipAddressToNodeMap[addr] = node;
            }
          }
        }
      }
    }
  }
}

template <typename T>
Ptr<Node> NixVectorRouting<T>::GetNodeByIp(IpAddress dest) const {
  NS_LOG_FUNCTION(this << dest);

#ifdef NS3_MTP
  if (!g_isMapBuilt.load(std::memory_order_acquire)) {
    if (g_mapBuilding.exchange(true, std::memory_order_relaxed)) {
      while (!g_isMapBuilt.load(std::memory_order_acquire))
        ;
    } else {
      BuildIpAddressToNodeMap();
      g_isMapBuilt.store(true, std::memory_order_release);
      g_mapBuilding.store(false, std::memory_order_release);
    }
  }
#else
  if (g_ipAddressToNodeMap.empty()) {
    BuildIpAddressToNodeMap();
  }
#endif

  Ptr<Node> destNode;

  auto iter = g_ipAddressToNodeMap.find(dest);

  if (iter == g_ipAddressToNodeMap.end()) {
    NS_LOG_ERROR("Couldn't find dest node given the IP" << dest);
    destNode = nullptr;
  } else {
    destNode = iter->second;
  }

  return destNode;
}

template <typename T>
Ptr<typename NixVectorRouting<T>::IpInterface>
NixVectorRouting<T>::GetInterfaceByNetDevice(Ptr<NetDevice> netDevice) const {
#ifdef NS3_MTP
  if (!g_isMapBuilt.load(std::memory_order_acquire)) {
    if (g_mapBuilding.exchange(true, std::memory_order_relaxed)) {
      while (!g_isMapBuilt.load(std::memory_order_acquire))
        ;
    } else {
      BuildIpAddressToNodeMap();
      g_isMapBuilt.store(true, std::memory_order_release);
      g_mapBuilding.store(false, std::memory_order_release);
    }
  }
#else
  if (g_netdeviceToIpInterfaceMap.empty()) {
    BuildIpAddressToNodeMap();
  }
#endif

  Ptr<IpInterface> ipInterface;

  auto iter = g_netdeviceToIpInterfaceMap.find(netDevice);

  if (iter == g_netdeviceToIpInterfaceMap.end()) {
    NS_LOG_ERROR("Couldn't find IpInterface node given the NetDevice"
                 << netDevice);
    ipInterface = nullptr;
  } else {
    ipInterface = iter->second;
  }

  return ipInterface;
}

template <typename T>
uint32_t NixVectorRouting<T>::FindTotalNeighbors(Ptr<Node> node) const {
  NS_LOG_FUNCTION(this << node);

  uint32_t numberOfDevices = node->GetNDevices();
  uint32_t totalNeighbors = 0;

  for (uint32_t i = 0; i < numberOfDevices; i++) {
    Ptr<NetDevice> localNetDevice = node->GetDevice(i);
    Ptr<Channel> channel = localNetDevice->GetChannel();
    if (!channel) {
      continue;
    }

    NetDeviceContainer netDeviceContainer;
    GetAdjacentNetDevices(localNetDevice, channel, netDeviceContainer);

    totalNeighbors += netDeviceContainer.GetN();
  }

  return totalNeighbors;
}

template <typename T>
Ptr<BridgeNetDevice>
NixVectorRouting<T>::NetDeviceIsBridged(Ptr<NetDevice> nd) const {
  NS_LOG_FUNCTION(this << nd);

  Ptr<Node> node = nd->GetNode();
  uint32_t nDevices = node->GetNDevices();

  for (uint32_t i = 0; i < nDevices; ++i) {
    Ptr<NetDevice> ndTest = node->GetDevice(i);
    NS_LOG_LOGIC("Examine device " << i << " " << ndTest);

    if (ndTest->IsBridge()) {
      NS_LOG_LOGIC("device " << i << " is a bridge net device");
      Ptr<BridgeNetDevice> bnd = ndTest->GetObject<BridgeNetDevice>();
      NS_ABORT_MSG_UNLESS(bnd, "NixVectorRouting::NetDeviceIsBridged (): "
                               "GetObject for <BridgeNetDevice> failed");

      for (uint32_t j = 0; j < bnd->GetNBridgePorts(); ++j) {
        NS_LOG_LOGIC("Examine bridge port " << j << " "
                                            << bnd->GetBridgePort(j));
        if (bnd->GetBridgePort(j) == nd) {
          NS_LOG_LOGIC("Net device " << nd << " is bridged by " << bnd);
          return bnd;
        }
      }
    }
  }
  NS_LOG_LOGIC("Net device " << nd << " is not bridged");
  return nullptr;
}

template <typename T>
uint32_t NixVectorRouting<T>::FindNetDeviceForNixIndex(
    Ptr<Node> node, uint32_t nodeIndex, IpAddress &gatewayIp) const {
  NS_LOG_FUNCTION(this << node << nodeIndex << gatewayIp);

  uint32_t numberOfDevices = node->GetNDevices();
  uint32_t index = 0;
  uint32_t totalNeighbors = 0;

  for (uint32_t i = 0; i < numberOfDevices; i++) {
    Ptr<NetDevice> localNetDevice = node->GetDevice(i);
    Ptr<Channel> channel = localNetDevice->GetChannel();
    if (!channel) {
      continue;
    }

    NetDeviceContainer netDeviceContainer;
    GetAdjacentNetDevices(localNetDevice, channel, netDeviceContainer);

    if (nodeIndex < (totalNeighbors + netDeviceContainer.GetN())) {
      index = i;
      Ptr<NetDevice> gatewayDevice =
          netDeviceContainer.Get(nodeIndex - totalNeighbors);
      Ptr<IpInterface> gatewayInterface =
          GetInterfaceByNetDevice(gatewayDevice);
      IpInterfaceAddress ifAddr = gatewayInterface->GetAddress(0);
      gatewayIp = ifAddr.GetAddress();
      break;
    }
    totalNeighbors += netDeviceContainer.GetN();
  }

  return index;
}

template <typename T>
Ptr<typename NixVectorRouting<T>::IpRoute>
NixVectorRouting<T>::RouteOutput(Ptr<Packet> p, const IpHeader &header,
                                 Ptr<NetDevice> oif,
                                 Socket::SocketErrno &sockerr) {
  NS_LOG_FUNCTION(this << header << oif);

  Ptr<IpRoute> rtentry;
  Ptr<NixVector> nixVectorInCache;
  Ptr<NixVector> nixVectorForPacket;

  CheckCacheStateAndFlush();

  IpAddress destAddress = header.GetDestination();

  NS_LOG_DEBUG("Dest IP from header: " << destAddress);

  if (destAddress.IsLocalhost()) {
    rtentry = Create<IpRoute>();
    rtentry->SetSource(IpAddress::GetLoopback());
    rtentry->SetDestination(destAddress);
    rtentry->SetGateway(IpAddress::GetZero());
    for (uint32_t i = 0; i < m_ip->GetNInterfaces(); i++) {
      Ptr<LoopbackNetDevice> loNetDevice =
          DynamicCast<LoopbackNetDevice>(m_ip->GetNetDevice(i));
      if (loNetDevice) {
        rtentry->SetOutputDevice(loNetDevice);
        break;
      }
    }
    return rtentry;
  }

  if constexpr (!IsIpv4) {
    if (destAddress.IsLinkLocalMulticast()) {
      NS_ASSERT_MSG(oif, "Try to send on link-local multicast address, and no "
                         "interface index is given!");
      rtentry = Create<IpRoute>();
      rtentry->SetSource(m_ip->SourceAddressSelection(
          m_ip->GetInterfaceForDevice(oif), destAddress));
      rtentry->SetDestination(destAddress);
      rtentry->SetGateway(Ipv6Address::GetZero());
      rtentry->SetOutputDevice(oif);
      return rtentry;
    }
  }
  bool foundInCache = false;
  nixVectorInCache = GetNixVectorInCache(destAddress, foundInCache);

  if (!foundInCache) {
    NS_LOG_LOGIC("Nix-vector not in cache, build: ");
    nixVectorInCache = GetNixVector(m_node, destAddress, oif);
    if (nixVectorInCache) {
      m_nixCache.insert(
          typename NixMap_t::value_type(destAddress, nixVectorInCache));
    }
  }

  if (nixVectorInCache) {
    NS_LOG_LOGIC("Nix-vector contents: " << *nixVectorInCache);

    nixVectorForPacket = nixVectorInCache->Copy();

    if (m_totalNeighbors == 0) {
      m_totalNeighbors = FindTotalNeighbors(m_node);
    }

    uint32_t numberOfBits = nixVectorForPacket->BitCount(m_totalNeighbors);
    uint32_t nodeIndex = nixVectorForPacket->ExtractNeighborIndex(numberOfBits);

    rtentry = GetIpRouteInCache(destAddress);

    if (!rtentry || !(rtentry->GetOutputDevice() == oif)) {

      if (rtentry) {
        m_ipRouteCache.erase(destAddress);
      }

      NS_LOG_LOGIC("IpRoute not in cache, build: ");
      IpAddress gatewayIp;
      uint32_t index = FindNetDeviceForNixIndex(m_node, nodeIndex, gatewayIp);
      int32_t interfaceIndex = 0;

      if (!oif) {
        interfaceIndex =
            (m_ip)->GetInterfaceForDevice(m_node->GetDevice(index));
      } else {
        interfaceIndex = (m_ip)->GetInterfaceForDevice(oif);
      }

      NS_ASSERT_MSG(interfaceIndex != -1,
                    "Interface index not found for device");

      IpAddress sourceIPAddr =
          m_ip->SourceAddressSelection(interfaceIndex, destAddress);

      rtentry = Create<IpRoute>();
      rtentry->SetSource(sourceIPAddr);

      rtentry->SetGateway(gatewayIp);
      rtentry->SetDestination(destAddress);

      if (!oif) {
        rtentry->SetOutputDevice(m_ip->GetNetDevice(interfaceIndex));
      } else {
        rtentry->SetOutputDevice(oif);
      }

      sockerr = Socket::ERROR_NOTERROR;

      m_ipRouteCache.insert(
          typename IpRouteMap_t::value_type(destAddress, rtentry));
    }

    NS_LOG_LOGIC("Nix-vector contents: "
                 << *nixVectorInCache << " : Remaining bits: "
                 << nixVectorForPacket->GetRemainingBits());

    if (p) {
      NS_LOG_LOGIC("Adding Nix-vector to packet: " << *nixVectorForPacket);
      p->SetNixVector(nixVectorForPacket);
    }
  } else {
    NS_LOG_ERROR("No path to the dest: " << destAddress);
    sockerr = Socket::ERROR_NOROUTETOHOST;
  }

  return rtentry;
}

template <typename T>
bool NixVectorRouting<T>::RouteInput(
    Ptr<const Packet> p, const IpHeader &header, Ptr<const NetDevice> idev,
    const UnicastForwardCallback &ucb, const MulticastForwardCallback &mcb,
    const LocalDeliverCallback &lcb, const ErrorCallback &ecb) {
  NS_LOG_FUNCTION(this << p << header << header.GetSource()
                       << header.GetDestination() << idev);

  CheckCacheStateAndFlush();

  NS_ASSERT(m_ip);
  NS_ASSERT(m_ip->GetInterfaceForDevice(idev) >= 0);
  uint32_t iif = m_ip->GetInterfaceForDevice(idev);
  NS_ASSERT(iif >= 0);

  IpAddress destAddress = header.GetDestination();

  if constexpr (IsIpv4) {
    if (m_ip->IsDestinationAddress(destAddress, iif)) {
      if (!lcb.IsNull()) {
        NS_LOG_LOGIC("Local delivery to " << destAddress);
        p->SetNixVector(nullptr);
        lcb(p, header, iif);
        return true;
      } else {
        return false;
      }
    }
  } else {
    if (destAddress.IsMulticast()) {
      NS_LOG_LOGIC("Multicast route not supported by Nix-Vector routing "
                   << destAddress);
      return false;
    }

    if (m_ip->IsForwarding(iif) == false) {
      NS_LOG_LOGIC("Forwarding disabled for this interface");
      if (!ecb.IsNull()) {
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
      }
      return true;
    }
  }

  Ptr<IpRoute> rtentry;

  Ptr<NixVector> nixVector = p->GetNixVector();

  NS_ASSERT(nixVector);

  if (nixVector->GetEpoch() != g_epoch) {
    NS_LOG_LOGIC("NixVector epoch mismatch (" << nixVector->GetEpoch() << " Vs "
                                              << g_epoch
                                              << ") - rebuilding it");
    nixVector = GetNixVector(m_node, destAddress, nullptr);
    p->SetNixVector(nixVector);
  }

  if (m_totalNeighbors == 0) {
    m_totalNeighbors = FindTotalNeighbors(m_node);
  }
  uint32_t numberOfBits = nixVector->BitCount(m_totalNeighbors);
  uint32_t nodeIndex = nixVector->ExtractNeighborIndex(numberOfBits);

  rtentry = GetIpRouteInCache(destAddress);
  if (!rtentry) {
    NS_LOG_LOGIC("IpRoute not in cache, build: ");
    IpAddress gatewayIp;
    uint32_t index = FindNetDeviceForNixIndex(m_node, nodeIndex, gatewayIp);
    uint32_t interfaceIndex =
        (m_ip)->GetInterfaceForDevice(m_node->GetDevice(index));
    IpInterfaceAddress ifAddr = m_ip->GetAddress(interfaceIndex, 0);

    rtentry = Create<IpRoute>();
    rtentry->SetSource(ifAddr.GetAddress());

    rtentry->SetGateway(gatewayIp);
    rtentry->SetDestination(destAddress);
    rtentry->SetOutputDevice(m_ip->GetNetDevice(interfaceIndex));

    m_ipRouteCache.insert(
        typename IpRouteMap_t::value_type(destAddress, rtentry));
  }

  NS_LOG_LOGIC("At Node " << m_node->GetId() << ", Extracting " << numberOfBits
                          << " bits from Nix-vector: " << nixVector << " : "
                          << *nixVector);

  if constexpr (IsIpv4) {
    ucb(rtentry, p, header);
  } else {
    ucb(idev, rtentry, p, header);
  }

  return true;
}

template <typename T>
void NixVectorRouting<T>::PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                                            Time::Unit unit) const {
  NS_LOG_FUNCTION_NOARGS();

  CheckCacheStateAndFlush();

  std::ostream *os = stream->GetStream();
  std::ios oldState(nullptr);
  oldState.copyfmt(*os);

  *os << std::resetiosflags(std::ios::adjustfield)
      << std::setiosflags(std::ios::left);

  *os << "Node: " << m_ip->template GetObject<Node>()->GetId()
      << ", Time: " << Now().As(unit) << ", Local time: "
      << m_ip->template GetObject<Node>()->GetLocalTime().As(unit)
      << ", Nix Routing" << std::endl;

  *os << "NixCache:" << std::endl;
  if (m_nixCache.size() > 0) {
    *os << std::setw(30) << "Destination";
    *os << "NixVector" << std::endl;
    for (auto it = m_nixCache.begin(); it != m_nixCache.end(); it++) {
      std::ostringstream dest;
      dest << it->first;
      *os << std::setw(30) << dest.str();
      if (it->second) {
        *os << *(it->second) << std::endl;
      } else {
        *os << "-" << std::endl;
      }
    }
  }

  *os << "IpRouteCache:" << std::endl;
  if (m_ipRouteCache.size() > 0) {
    *os << std::setw(30) << "Destination";
    *os << std::setw(30) << "Gateway";
    *os << std::setw(30) << "Source";
    *os << "OutputDevice" << std::endl;
    for (auto it = m_ipRouteCache.begin(); it != m_ipRouteCache.end(); it++) {
      std::ostringstream dest;
      std::ostringstream gw;
      std::ostringstream src;
      dest << it->second->GetDestination();
      *os << std::setw(30) << dest.str();
      gw << it->second->GetGateway();
      *os << std::setw(30) << gw.str();
      src << it->second->GetSource();
      *os << std::setw(30) << src.str();
      *os << "  ";
      if (Names::FindName(it->second->GetOutputDevice()) != "") {
        *os << Names::FindName(it->second->GetOutputDevice());
      } else {
        *os << it->second->GetOutputDevice()->GetIfIndex();
      }
      *os << std::endl;
    }
  }
  *os << std::endl;
  (*os).copyfmt(oldState);
}

template <typename T> void NixVectorRouting<T>::NotifyInterfaceUp(uint32_t i) {
#ifdef NS3_MTP
  g_isCacheDirty.store(true, std::memory_order_release);
#else
  g_isCacheDirty = true;
#endif
}

template <typename T>
void NixVectorRouting<T>::NotifyInterfaceDown(uint32_t i) {
#ifdef NS3_MTP
  g_isCacheDirty.store(true, std::memory_order_release);
#else
  g_isCacheDirty = true;
#endif
}

template <typename T>
void NixVectorRouting<T>::NotifyAddAddress(uint32_t interface,
                                           IpInterfaceAddress address) {
#ifdef NS3_MTP
  g_isCacheDirty.store(true, std::memory_order_release);
#else
  g_isCacheDirty = true;
#endif
}

template <typename T>
void NixVectorRouting<T>::NotifyRemoveAddress(uint32_t interface,
                                              IpInterfaceAddress address) {
#ifdef NS3_MTP
  g_isCacheDirty.store(true, std::memory_order_release);
#else
  g_isCacheDirty = true;
#endif
}

template <typename T>
void NixVectorRouting<T>::NotifyAddRoute(IpAddress dst, Ipv6Prefix mask,
                                         IpAddress nextHop, uint32_t interface,
                                         IpAddress prefixToUse) {
#ifdef NS3_MTP
  g_isCacheDirty.store(true, std::memory_order_release);
#else
  g_isCacheDirty = true;
#endif
}

template <typename T>
void NixVectorRouting<T>::NotifyRemoveRoute(IpAddress dst, Ipv6Prefix mask,
                                            IpAddress nextHop,
                                            uint32_t interface,
                                            IpAddress prefixToUse) {
#ifdef NS3_MTP
  g_isCacheDirty.store(true, std::memory_order_release);
#else
  g_isCacheDirty = true;
#endif
}

template <typename T>
bool NixVectorRouting<T>::BFS(uint32_t numberOfNodes, Ptr<Node> source,
                              Ptr<Node> dest,
                              std::vector<Ptr<Node>> &parentVector,
                              Ptr<NetDevice> oif) const {
  NS_LOG_FUNCTION(this << numberOfNodes << source << dest << parentVector
                       << oif);

  NS_LOG_LOGIC("Going from Node " << source->GetId() << " to Node "
                                  << dest->GetId());
  std::queue<Ptr<Node>> greyNodeList;

  parentVector.assign(numberOfNodes, nullptr);

  greyNodeList.push(source);
  parentVector.at(source->GetId()) = source;

  while (!greyNodeList.empty()) {
    Ptr<Node> currNode = greyNodeList.front();
    Ptr<IpL3Protocol> ip = currNode->GetObject<IpL3Protocol>();

    if (currNode == dest) {
      NS_LOG_LOGIC("Made it to Node " << currNode->GetId());
      return true;
    }

    if (currNode == source && oif) {
      if (ip) {
        uint32_t interfaceIndex = (ip)->GetInterfaceForDevice(oif);
        if (!(ip->IsUp(interfaceIndex))) {
          NS_LOG_LOGIC("IpInterface is down");
          return false;
        }
      }
      if (!(oif->IsLinkUp())) {
        NS_LOG_LOGIC("Link is down.");
        return false;
      }
      Ptr<Channel> channel = oif->GetChannel();
      if (!channel) {
        return false;
      }

      NetDeviceContainer netDeviceContainer;
      GetAdjacentNetDevices(oif, channel, netDeviceContainer);

      for (auto iter = netDeviceContainer.Begin();
           iter != netDeviceContainer.End(); iter++) {
        Ptr<Node> remoteNode = (*iter)->GetNode();
        Ptr<IpInterface> remoteIpInterface = GetInterfaceByNetDevice(*iter);
        if (!remoteIpInterface || !(remoteIpInterface->IsUp())) {
          NS_LOG_LOGIC("IpInterface either doesn't exist or is down");
          continue;
        }

        if (!parentVector.at(remoteNode->GetId())) {
          parentVector.at(remoteNode->GetId()) = currNode;
          greyNodeList.push(remoteNode);
        }
      }
    } else {
      for (uint32_t i = 0; i < (currNode->GetNDevices()); i++) {
        Ptr<NetDevice> localNetDevice = currNode->GetDevice(i);

        if (ip) {
          uint32_t interfaceIndex =
              (ip)->GetInterfaceForDevice(currNode->GetDevice(i));
          if (!(ip->IsUp(interfaceIndex))) {
            NS_LOG_LOGIC("IpInterface is down");
            continue;
          }
        }
        if (!(localNetDevice->IsLinkUp())) {
          NS_LOG_LOGIC("Link is down.");
          continue;
        }
        Ptr<Channel> channel = localNetDevice->GetChannel();
        if (!channel) {
          continue;
        }

        NetDeviceContainer netDeviceContainer;
        GetAdjacentNetDevices(localNetDevice, channel, netDeviceContainer);

        for (auto iter = netDeviceContainer.Begin();
             iter != netDeviceContainer.End(); iter++) {
          Ptr<Node> remoteNode = (*iter)->GetNode();
          Ptr<IpInterface> remoteIpInterface = GetInterfaceByNetDevice(*iter);
          if (!remoteIpInterface || !(remoteIpInterface->IsUp())) {
            NS_LOG_LOGIC("IpInterface either doesn't exist or is down");
            continue;
          }

          if (!parentVector.at(remoteNode->GetId())) {
            parentVector.at(remoteNode->GetId()) = currNode;
            greyNodeList.push(remoteNode);
          }
        }
      }
    }

    greyNodeList.pop();
  }

  return false;
}

template <typename T>
void NixVectorRouting<T>::PrintRoutingPath(Ptr<Node> source, IpAddress dest,
                                           Ptr<OutputStreamWrapper> stream,
                                           Time::Unit unit) const {
  NS_LOG_FUNCTION(this << source << dest);

  Ptr<NixVector> nixVectorInCache;
  Ptr<NixVector> nixVector;
  Ptr<IpRoute> rtentry;

  CheckCacheStateAndFlush();

  Ptr<Node> destNode = GetNodeByIp(dest);
  if (!destNode) {
    NS_LOG_ERROR("No routing path exists");
    return;
  }

  std::ostream *os = stream->GetStream();
  std::ios oldState(nullptr);
  oldState.copyfmt(*os);

  *os << std::resetiosflags(std::ios::adjustfield)
      << std::setiosflags(std::ios::left);
  *os << "Time: " << Now().As(unit) << ", Nix Routing" << std::endl;
  *os << "Route path from ";
  *os << "Node " << source->GetId() << " to Node " << destNode->GetId() << ", ";
  *os << "Nix Vector: ";

  bool foundInCache = true;
  nixVectorInCache = GetNixVectorInCache(dest, foundInCache);

  if (!foundInCache) {
    NS_LOG_LOGIC("Nix-vector not in cache, build: ");
    nixVectorInCache = GetNixVector(source, dest, nullptr);
  }

  if (nixVectorInCache || (!nixVectorInCache && source == destNode)) {
    Ptr<Node> curr = source;
    uint32_t totalNeighbors = 0;

    if (nixVectorInCache) {
      nixVector = nixVectorInCache->Copy();

      *os << *nixVector;
    }
    *os << std::endl;

    if (source == destNode) {
      std::ostringstream addr;
      std::ostringstream node;
      addr << dest;
      node << "(Node " << destNode->GetId() << ")";
      *os << std::setw(25) << addr.str();
      *os << std::setw(10) << node.str();
      *os << "---->   ";
      *os << std::setw(25) << addr.str();
      *os << node.str() << std::endl;
    }

    while (curr != destNode) {
      totalNeighbors = FindTotalNeighbors(curr);
      uint32_t numberOfBits = nixVector->BitCount(totalNeighbors);
      uint32_t nixIndex = nixVector->ExtractNeighborIndex(numberOfBits);
      IpAddress gatewayIp;
      uint32_t netDeviceIndex =
          FindNetDeviceForNixIndex(curr, nixIndex, gatewayIp);
      Ptr<IpL3Protocol> ip = curr->GetObject<IpL3Protocol>();
      Ptr<NetDevice> outDevice = curr->GetDevice(netDeviceIndex);
      uint32_t interfaceIndex = ip->GetInterfaceForDevice(outDevice);
      IpAddress sourceIPAddr;
      if (curr == source) {
        sourceIPAddr = ip->SourceAddressSelection(interfaceIndex, dest);
      } else {
        sourceIPAddr = ip->GetAddress(interfaceIndex, 0).GetAddress();
      }

      std::ostringstream currAddr;
      std::ostringstream currNode;
      std::ostringstream nextAddr;
      std::ostringstream nextNode;
      currAddr << sourceIPAddr;
      currNode << "(Node " << curr->GetId() << ")";
      *os << std::setw(25) << currAddr.str();
      *os << std::setw(10) << currNode.str();
      curr = GetNodeByIp(gatewayIp);
      nextAddr << ((curr == destNode) ? dest : gatewayIp);
      nextNode << "(Node " << curr->GetId() << ")";
      *os << "---->   ";
      *os << std::setw(25) << nextAddr.str();
      *os << nextNode.str() << std::endl;
    }
    *os << std::endl;
  } else {
    *os << ")" << std::endl;
    *os << "There does not exist a path from Node " << source->GetId()
        << " to Node " << destNode->GetId() << "." << std::endl;
  }
  (*os).copyfmt(oldState);
}

template <typename T>
void NixVectorRouting<T>::CheckCacheStateAndFlush() const {
#ifdef NS3_MTP
  if (g_isCacheDirty.load(std::memory_order_acquire)) {
    if (g_cacheFlushing.exchange(true, std::memory_order_relaxed)) {
      while (g_isCacheDirty.load(std::memory_order_acquire))
        ;
    } else {
      FlushGlobalNixRoutingCache();
      g_isCacheDirty.store(false, std::memory_order_release);
      g_cacheFlushing.store(false, std::memory_order_release);
    }
  }
#else
  if (g_isCacheDirty) {
    FlushGlobalNixRoutingCache();
    g_epoch++;
    g_isCacheDirty = false;
  }
#endif
}

template void NixVectorRouting<Ipv4RoutingProtocol>::SetNode(Ptr<Node> node);
template void NixVectorRouting<Ipv6RoutingProtocol>::SetNode(Ptr<Node> node);
template void
NixVectorRouting<Ipv4RoutingProtocol>::FlushGlobalNixRoutingCache() const;
template void
NixVectorRouting<Ipv6RoutingProtocol>::FlushGlobalNixRoutingCache() const;
template void NixVectorRouting<Ipv4RoutingProtocol>::PrintRoutingPath(
    Ptr<Node> source, IpAddress dest, Ptr<OutputStreamWrapper> stream,
    Time::Unit unit) const;
template void NixVectorRouting<Ipv6RoutingProtocol>::PrintRoutingPath(
    Ptr<Node> source, IpAddress dest, Ptr<OutputStreamWrapper> stream,
    Time::Unit unit) const;

} // namespace ns3
