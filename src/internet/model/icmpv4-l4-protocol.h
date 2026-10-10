
#ifndef ICMPV4_L4_PROTOCOL_H
#define ICMPV4_L4_PROTOCOL_H

#include "icmpv4.h"
#include "ip-l4-protocol.h"

#include "ns3/ipv4-address.h"

namespace ns3 {

class Node;
class Ipv4Interface;
class Ipv4Route;

class Icmpv4L4Protocol : public IpL4Protocol {
public:
  static TypeId GetTypeId();
  static const uint8_t PROT_NUMBER;

  Icmpv4L4Protocol();
  ~Icmpv4L4Protocol() override;

  void SetNode(Ptr<Node> node);

  static uint16_t GetStaticProtocolNumber();

  int GetProtocolNumber() const override;

  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv4Header &header,
                                 Ptr<Ipv4Interface> incomingInterface) override;

  IpL4Protocol::RxStatus Receive(Ptr<Packet> p, const Ipv6Header &header,
                                 Ptr<Ipv6Interface> incomingInterface) override;

  void SendDestUnreachFragNeeded(Ipv4Header header, Ptr<const Packet> orgData,
                                 uint16_t nextHopMtu);

  void SendTimeExceededTtl(Ipv4Header header, Ptr<const Packet> orgData,
                           bool isFragment);

  void SendDestUnreachPort(Ipv4Header header, Ptr<const Packet> orgData);

  void SetDownTarget(IpL4Protocol::DownTargetCallback cb) override;
  void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb) override;
  IpL4Protocol::DownTargetCallback GetDownTarget() const override;
  IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;

protected:
  void NotifyNewAggregate() override;

private:
  void HandleEcho(Ptr<Packet> p, Icmpv4Header header, Ipv4Address source,
                  Ipv4Address destination);
  void HandleDestUnreach(Ptr<Packet> p, Icmpv4Header header, Ipv4Address source,
                         Ipv4Address destination);
  void HandleTimeExceeded(Ptr<Packet> p, Icmpv4Header icmp, Ipv4Address source,
                          Ipv4Address destination);
  void SendDestUnreach(Ipv4Header header, Ptr<const Packet> orgData,
                       uint8_t code, uint16_t nextHopMtu);
  void SendMessage(Ptr<Packet> packet, Ipv4Address dest, uint8_t type,
                   uint8_t code);
  void SendMessage(Ptr<Packet> packet, Ipv4Address source, Ipv4Address dest,
                   uint8_t type, uint8_t code, Ptr<Ipv4Route> route);
  void Forward(Ipv4Address source, Icmpv4Header icmp, uint32_t info,
               Ipv4Header ipHeader, const uint8_t payload[8]);

  void DoDispose() override;

  Ptr<Node> m_node;
  IpL4Protocol::DownTargetCallback m_downTarget;
};

} // namespace ns3

#endif
