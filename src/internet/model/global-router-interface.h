
#ifndef GLOBAL_ROUTER_INTERFACE_H
#define GLOBAL_ROUTER_INTERFACE_H

#include "global-route-manager.h"
#include "ipv4-routing-table-entry.h"

#include "ns3/bridge-net-device.h"
#include "ns3/channel.h"
#include "ns3/ipv4-address.h"
#include "ns3/net-device-container.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class GlobalRouter;
class Ipv4GlobalRouting;

class GlobalRoutingLinkRecord {
public:
  friend class GlobalRoutingLSA;

  enum LinkType {
    Unknown = 0,
    PointToPoint,
    TransitNetwork,
    StubNetwork,
    VirtualLink
  };

  GlobalRoutingLinkRecord();

  GlobalRoutingLinkRecord(LinkType linkType, Ipv4Address linkId,
                          Ipv4Address linkData, uint16_t metric);

  ~GlobalRoutingLinkRecord();

  Ipv4Address GetLinkId() const;

  void SetLinkId(Ipv4Address addr);

  Ipv4Address GetLinkData() const;

  void SetLinkData(Ipv4Address addr);

  LinkType GetLinkType() const;

  void SetLinkType(LinkType linkType);

  uint16_t GetMetric() const;

  void SetMetric(uint16_t metric);

private:
  Ipv4Address m_linkId;

  Ipv4Address m_linkData;

  LinkType m_linkType;

  uint16_t m_metric;
};

class GlobalRoutingLSA {
public:
  enum LSType {
    Unknown = 0,
    RouterLSA,
    NetworkLSA,
    SummaryLSA,
    SummaryLSA_ASBR,
    ASExternalLSAs
  };

  enum SPFStatus {
    LSA_SPF_NOT_EXPLORED = 0,
    LSA_SPF_CANDIDATE,
    LSA_SPF_IN_SPFTREE
  };

  GlobalRoutingLSA();

  GlobalRoutingLSA(SPFStatus status, Ipv4Address linkStateId,
                   Ipv4Address advertisingRtr);

  GlobalRoutingLSA(GlobalRoutingLSA &lsa);

  ~GlobalRoutingLSA();

  GlobalRoutingLSA &operator=(const GlobalRoutingLSA &lsa);

  void CopyLinkRecords(const GlobalRoutingLSA &lsa);

  uint32_t AddLinkRecord(GlobalRoutingLinkRecord *lr);

  uint32_t GetNLinkRecords() const;

  GlobalRoutingLinkRecord *GetLinkRecord(uint32_t n) const;

  void ClearLinkRecords();

  bool IsEmpty() const;

  void Print(std::ostream &os) const;

  LSType GetLSType() const;
  void SetLSType(LSType typ);

  Ipv4Address GetLinkStateId() const;

  void SetLinkStateId(Ipv4Address addr);

  Ipv4Address GetAdvertisingRouter() const;

  void SetAdvertisingRouter(Ipv4Address rtr);

  void SetNetworkLSANetworkMask(Ipv4Mask mask);

  Ipv4Mask GetNetworkLSANetworkMask() const;

  uint32_t AddAttachedRouter(Ipv4Address addr);

  uint32_t GetNAttachedRouters() const;

  Ipv4Address GetAttachedRouter(uint32_t n) const;

  SPFStatus GetStatus() const;

  void SetStatus(SPFStatus status);

  Ptr<Node> GetNode() const;

  void SetNode(Ptr<Node> node);

private:
  LSType m_lsType;
  Ipv4Address m_linkStateId;

  Ipv4Address m_advertisingRtr;

  typedef std::list<GlobalRoutingLinkRecord *> ListOfLinkRecords_t;

  ListOfLinkRecords_t m_linkRecords;

  Ipv4Mask m_networkLSANetworkMask;

  typedef std::list<Ipv4Address> ListOfAttachedRouters_t;

  ListOfAttachedRouters_t m_attachedRouters;

  SPFStatus m_status;
  uint32_t m_node_id;
};

std::ostream &operator<<(std::ostream &os, GlobalRoutingLSA &lsa);

class GlobalRouter : public Object {
public:
  static TypeId GetTypeId();

  GlobalRouter();

  GlobalRouter(const GlobalRouter &) = delete;
  GlobalRouter &operator=(const GlobalRouter &) = delete;

  void SetRoutingProtocol(Ptr<Ipv4GlobalRouting> routing);

  Ptr<Ipv4GlobalRouting> GetRoutingProtocol();

  Ipv4Address GetRouterId() const;

  uint32_t DiscoverLSAs();

  uint32_t GetNumLSAs() const;

  bool GetLSA(uint32_t n, GlobalRoutingLSA &lsa) const;

  void InjectRoute(Ipv4Address network, Ipv4Mask networkMask);

  uint32_t GetNInjectedRoutes();

  Ipv4RoutingTableEntry *GetInjectedRoute(uint32_t i);

  void RemoveInjectedRoute(uint32_t i);

  bool WithdrawRoute(Ipv4Address network, Ipv4Mask networkMask);

private:
  ~GlobalRouter() override;

  void ClearLSAs();

  Ptr<NetDevice> GetAdjacent(Ptr<NetDevice> nd, Ptr<Channel> ch) const;

  Ipv4Address FindDesignatedRouterForLink(Ptr<NetDevice> ndLocal) const;

  bool AnotherRouterOnLink(Ptr<NetDevice> nd) const;

  void ProcessBroadcastLink(Ptr<NetDevice> nd, GlobalRoutingLSA *pLSA,
                            NetDeviceContainer &c);

  void ProcessSingleBroadcastLink(Ptr<NetDevice> nd, GlobalRoutingLSA *pLSA,
                                  NetDeviceContainer &c);

  void ProcessBridgedBroadcastLink(Ptr<NetDevice> nd, GlobalRoutingLSA *pLSA,
                                   NetDeviceContainer &c);

  void ProcessPointToPointLink(Ptr<NetDevice> ndLocal, GlobalRoutingLSA *pLSA);

  void BuildNetworkLSAs(NetDeviceContainer c);

  NetDeviceContainer FindAllNonBridgedDevicesOnLink(Ptr<Channel> ch) const;

  Ptr<BridgeNetDevice> NetDeviceIsBridged(Ptr<NetDevice> nd) const;

  typedef std::list<GlobalRoutingLSA *> ListOfLSAs_t;
  ListOfLSAs_t m_LSAs;

  Ipv4Address m_routerId;
  Ptr<Ipv4GlobalRouting> m_routingProtocol;

  typedef std::list<Ipv4RoutingTableEntry *> InjectedRoutes;
  typedef std::list<Ipv4RoutingTableEntry *>::const_iterator InjectedRoutesCI;
  typedef std::list<Ipv4RoutingTableEntry *>::iterator InjectedRoutesI;
  InjectedRoutes m_injectedRoutes;

  mutable std::vector<Ptr<BridgeNetDevice>> m_bridgesVisited;
  void ClearBridgesVisited() const;
  bool BridgeHasAlreadyBeenVisited(Ptr<BridgeNetDevice> device) const;
  void MarkBridgeAsVisited(Ptr<BridgeNetDevice> device) const;

  void DoDispose() override;
};

} // namespace ns3

#endif
