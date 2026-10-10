
#ifndef DSR_ROUTING_H
#define DSR_ROUTING_H

#include "dsr-errorbuff.h"
#include "dsr-fs-header.h"
#include "dsr-gratuitous-reply-table.h"
#include "dsr-maintain-buff.h"
#include "dsr-network-queue.h"
#include "dsr-option-header.h"
#include "dsr-passive-buff.h"
#include "dsr-rcache.h"
#include "dsr-rreq-table.h"
#include "dsr-rsendbuff.h"

#include "ns3/buffer.h"
#include "ns3/callback.h"
#include "ns3/event-garbage-collector.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/ip-l4-protocol.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/traced-callback.h"
#include "ns3/wifi-mac.h"

#include <cassert>
#include <list>
#include <map>
#include <stdint.h>
#include <sys/types.h>
#include <vector>

namespace ns3 {

class Packet;
class Node;
class Ipv4;
class Ipv4Address;
class Ipv4Header;
class Ipv4Interface;
class Ipv4L3Protocol;
class Time;

namespace dsr {

class DsrOptions;

class DsrRouting : public IpL4Protocol {
public:
  static TypeId GetTypeId();
  static const uint8_t PROT_NUMBER;
  DsrRouting();
  ~DsrRouting() override;
  Ptr<Node> GetNode() const;
  void SetNode(Ptr<Node> node);
  void SetRouteCache(Ptr<dsr::DsrRouteCache> r);
  Ptr<dsr::DsrRouteCache> GetRouteCache() const;
  void SetRequestTable(Ptr<dsr::DsrRreqTable> r);
  Ptr<dsr::DsrRreqTable> GetRequestTable() const;
  void SetPassiveBuffer(Ptr<dsr::DsrPassiveBuffer> r);
  Ptr<dsr::DsrPassiveBuffer> GetPassiveBuffer() const;

  bool IsLinkCache();

  void UseExtends(DsrRouteCacheEntry::IP_VECTOR rt);

  bool LookupRoute(Ipv4Address id, DsrRouteCacheEntry &rt);

  bool AddRoute_Link(DsrRouteCacheEntry::IP_VECTOR nodelist,
                     Ipv4Address source);

  bool AddRoute(DsrRouteCacheEntry &rt);

  void DeleteAllRoutesIncludeLink(Ipv4Address errorSrc, Ipv4Address unreachNode,
                                  Ipv4Address node);

  bool UpdateRouteEntry(Ipv4Address dst);

  bool FindSourceEntry(Ipv4Address src, Ipv4Address dst, uint16_t id);

  Ptr<NetDevice> GetNetDeviceFromContext(std::string context);
  std::vector<std::string> GetElementsFromContext(std::string context);
  uint16_t GetIDfromIP(Ipv4Address address);
  Ipv4Address GetIPfromID(uint16_t id);
  Ipv4Address GetIPfromMAC(Mac48Address address);
  Ptr<Node> GetNodeWithAddress(Ipv4Address ipv4Address);
  void PrintVector(std::vector<Ipv4Address> &vec);
  Ipv4Address SearchNextHop(Ipv4Address ipv4Address,
                            std::vector<Ipv4Address> &vec);
  int GetProtocolNumber() const override;
  void SendBuffTimerExpire();
  void CheckSendBuffer();
  void PacketNewRoute(Ptr<Packet> packet, Ipv4Address source,
                      Ipv4Address destination, uint8_t protocol);
  Ptr<Ipv4Route> SetRoute(Ipv4Address nextHop, Ipv4Address srcAddress);
  uint32_t GetPriority(DsrMessageType messageType);
  void SendUnreachError(Ipv4Address unreachNode, Ipv4Address destination,
                        Ipv4Address originalDst, uint8_t salvage,
                        uint8_t protocol);

  void ForwardErrPacket(DsrOptionRerrUnreachHeader &rerr,
                        DsrOptionSRHeader &sourceRoute, Ipv4Address nextHop,
                        uint8_t protocol, Ptr<Ipv4Route> route);
  void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination,
            uint8_t protocol, Ptr<Ipv4Route> route);
  uint16_t AddAckReqHeader(Ptr<Packet> &packet, Ipv4Address nextHop);
  void SendPacket(Ptr<Packet> packet, Ipv4Address source, Ipv4Address nextHop,
                  uint8_t protocol);
  void Scheduler(uint32_t priority);
  void PriorityScheduler(uint32_t priority, bool continueWithFirst);
  void IncreaseRetransTimer();
  bool SendRealDown(DsrNetworkQueueEntry &newEntry);
  void SendPacketFromBuffer(const DsrOptionSRHeader &sourceRoute,
                            Ipv4Address nextHop, uint8_t protocol);
  bool PassiveEntryCheck(Ptr<Packet> packet, Ipv4Address source,
                         Ipv4Address destination, uint8_t segsLeft,
                         uint16_t fragmentOffset, uint16_t identification,
                         bool saveEntry);

  void CancelPacketAllTimer(DsrMaintainBuffEntry &mb);
  bool CancelPassiveTimer(Ptr<Packet> packet, Ipv4Address source,
                          Ipv4Address destination, uint8_t segsLeft);
  void CallCancelPacketTimer(uint16_t ackId, const Ipv4Header &ipv4Header,
                             Ipv4Address realSrc, Ipv4Address realDst);
  void CancelNetworkPacketTimer(DsrMaintainBuffEntry &mb);
  void CancelPassivePacketTimer(DsrMaintainBuffEntry &mb);
  void CancelLinkPacketTimer(DsrMaintainBuffEntry &mb);
  void CancelPacketTimerNextHop(Ipv4Address nextHop, uint8_t protocol);
  void SalvagePacket(Ptr<const Packet> packet, Ipv4Address source,
                     Ipv4Address dst, uint8_t protocol);
  void ScheduleLinkPacketRetry(DsrMaintainBuffEntry &mb, uint8_t protocol);
  void SchedulePassivePacketRetry(DsrMaintainBuffEntry &mb, uint8_t protocol);
  void ScheduleNetworkPacketRetry(DsrMaintainBuffEntry &mb, bool isFirst,
                                  uint8_t protocol);
  void LinkScheduleTimerExpire(DsrMaintainBuffEntry &mb, uint8_t protocol);
  void NetworkScheduleTimerExpire(DsrMaintainBuffEntry &mb, uint8_t protocol);
  void PassiveScheduleTimerExpire(DsrMaintainBuffEntry &mb, uint8_t protocol);
  void ForwardPacket(Ptr<const Packet> packet, DsrOptionSRHeader &sourceRoute,
                     const Ipv4Header &ipv4Header, Ipv4Address source,
                     Ipv4Address destination, Ipv4Address targetAddress,
                     uint8_t protocol, Ptr<Ipv4Route> route);
  void SendInitialRequest(Ipv4Address source, Ipv4Address destination,
                          uint8_t protocol);
  void SendErrorRequest(DsrOptionRerrUnreachHeader &rerr, uint8_t protocol);
  void SendRequest(Ptr<Packet> packet, Ipv4Address source);
  void ScheduleInterRequest(Ptr<Packet> packet);
  void SendGratuitousReply(Ipv4Address replyTo, Ipv4Address replyFrom,
                           std::vector<Ipv4Address> &nodeList,
                           uint8_t protocol);
  void SendReply(Ptr<Packet> packet, Ipv4Address source, Ipv4Address nextHop,
                 Ptr<Ipv4Route> route);
  void ScheduleInitialReply(Ptr<Packet> packet, Ipv4Address source,
                            Ipv4Address nextHop, Ptr<Ipv4Route> route);
  void ScheduleCachedReply(Ptr<Packet> packet, Ipv4Address source,
                           Ipv4Address destination, Ptr<Ipv4Route> route,
                           double hops);
  void SendAck(uint16_t ackId, Ipv4Address destination, Ipv4Address realSrc,
               Ipv4Address realDst, uint8_t protocol, Ptr<Ipv4Route> route);
  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv4Header &header,
                                 Ptr<Ipv4Interface> incomingInterface) override;

  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv6Header &header,
                                 Ptr<Ipv6Interface> incomingInterface) override;

  void SetDownTarget(IpL4Protocol::DownTargetCallback callback) override;
  void SetDownTarget6(IpL4Protocol::DownTargetCallback6 callback) override;
  IpL4Protocol::DownTargetCallback GetDownTarget() const override;
  IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;
  uint8_t Process(Ptr<Packet> &packet, const Ipv4Header &ipv4Header,
                  Ipv4Address dst, uint8_t *nextHeader, uint8_t protocol,
                  bool &isDropped);
  void Insert(Ptr<dsr::DsrOptions> option);
  Ptr<dsr::DsrOptions> GetOption(int optionNumber);
  void CancelRreqTimer(Ipv4Address dst, bool isRemove);
  void ScheduleRreqRetry(Ptr<Packet> packet, std::vector<Ipv4Address> address,
                         bool nonProp, uint32_t requestId, uint8_t protocol);
  void RouteRequestTimerExpire(Ptr<Packet> packet,
                               std::vector<Ipv4Address> address,
                               uint32_t requestId, uint8_t protocol);

  int64_t AssignStreams(int64_t stream);

protected:
  void NotifyNewAggregate() override;
  void DoDispose() override;
  TracedCallback<Ptr<const Packet>> m_dropTrace;
  TracedCallback<const DsrOptionSRHeader &> m_txPacketTrace;

private:
  void Start();
  void SendRerrWhenBreaksLinkToNextHop(Ipv4Address nextHop, uint8_t protocol);
  bool PromiscReceive(Ptr<NetDevice> device, Ptr<const Packet> packet,
                      uint16_t protocol, const Address &from, const Address &to,
                      NetDevice::PacketType packetType);
  typedef std::list<Ptr<DsrOptions>> DsrOptionList_t;
  DsrOptionList_t m_options;

  Ptr<Ipv4L3Protocol> m_ipv4;

  Ptr<Ipv4Route> m_ipv4Route;

  Ptr<Ipv4> m_ip;

  Ptr<Node> m_node;

  Ipv4Address m_mainAddress;

  uint8_t segsLeft;

  IpL4Protocol::DownTargetCallback m_downTarget;

  uint32_t m_maxNetworkSize;

  Time m_maxNetworkDelay;

  uint32_t m_discoveryHopLimit;

  uint8_t m_maxSalvageCount;

  Time m_requestPeriod;

  Time m_nonpropRequestTimeout;

  uint32_t m_sendRetries;

  uint32_t m_passiveRetries;

  uint32_t m_linkRetries;

  uint32_t m_rreqRetries;

  uint32_t m_maxMaintRexmt;

  Time m_nodeTraversalTime;

  uint32_t m_maxSendBuffLen;

  Time m_sendBufferTimeout;

  DsrSendBuffer m_sendBuffer;

  DsrErrorBuffer m_errorBuffer;

  uint32_t m_maxMaintainLen;

  Time m_maxMaintainTime;

  uint32_t m_maxCacheLen;

  Time m_maxCacheTime;

  Time m_maxRreqTime;

  uint32_t m_maxEntriesEachDst;

  DsrMaintainBuffer m_maintainBuffer;

  uint32_t m_requestId;

  uint16_t m_ackId;

  uint32_t m_requestTableSize;

  uint32_t m_requestTableIds;

  uint32_t m_maxRreqId;

  Time m_blacklistTimeout;

  Ipv4Address m_broadcast;

  uint32_t m_broadcastJitter;

  Time m_passiveAckTimeout;

  uint32_t m_tryPassiveAcks;

  Time m_linkAckTimeout;

  uint32_t m_tryLinkAcks;

  Timer m_sendBuffTimer;

  Time m_sendBuffInterval;

  Time m_gratReplyHoldoff;

  Time m_maxRequestPeriod;

  uint32_t m_graReplyTableSize;

  std::string m_cacheType;

  std::string m_routeSortType;

  uint32_t m_stabilityDecrFactor;

  uint32_t m_stabilityIncrFactor;

  Time m_initStability;

  Time m_minLifeTime;

  Time m_useExtends;

  bool m_subRoute;

  Time m_retransIncr;

  std::vector<Ipv4Address> m_finalRoute;

  std::map<Ipv4Address, Timer> m_addressReqTimer;

  std::map<Ipv4Address, Timer> m_nonPropReqTimer;

  std::map<NetworkKey, Timer> m_addressForwardTimer;

  std::map<NetworkKey, uint32_t> m_addressForwardCnt;

  std::map<PassiveKey, uint32_t> m_passiveCnt;

  std::map<PassiveKey, Timer> m_passiveAckTimer;

  std::map<LinkKey, uint32_t> m_linkCnt;

  std::map<LinkKey, Timer> m_linkAckTimer;

  Ptr<dsr::DsrRouteCache> m_routeCache;

  Ptr<dsr::DsrRreqTable> m_rreqTable;

  Ptr<dsr::DsrPassiveBuffer> m_passiveBuffer;

  uint32_t m_numPriorityQueues;

  bool m_linkAck;

  std::map<uint32_t, Ptr<dsr::DsrNetworkQueue>> m_priorityQueue;

  DsrGraReply m_graReply;

  DsrNetworkQueue m_networkQueue;

  std::vector<Ipv4Address> m_clearList;

  std::vector<Ipv4Address> m_addresses;

  std::map<std::string, uint32_t> m_macToNodeIdMap;

  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};
} // namespace dsr
} // namespace ns3

#endif
