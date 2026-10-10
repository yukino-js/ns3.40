
#ifndef DSR_OPTION_H
#define DSR_OPTION_H

#include "dsr-gratuitous-reply-table.h"
#include "dsr-maintain-buff.h"
#include "dsr-option-header.h"
#include "dsr-rcache.h"
#include "dsr-routing.h"
#include "dsr-rsendbuff.h"

#include "ns3/buffer.h"
#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/timer.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-l4-protocol.h"

#include <list>
#include <map>

namespace ns3 {

class Packet;
class NetDevice;
class Node;
class Ipv4Address;
class Ipv4Interface;
class Ipv4Route;
class Ipv4;
class Time;

namespace dsr {

class DsrOptions : public Object {
public:
  static TypeId GetTypeId();
  DsrOptions();
  ~DsrOptions() override;
  virtual uint8_t GetOptionNumber() const = 0;
  void SetNode(Ptr<Node> node);
  Ptr<Node> GetNode() const;
  bool ContainAddressAfter(Ipv4Address ipv4Address, Ipv4Address destAddress,
                           std::vector<Ipv4Address> &nodeList);
  std::vector<Ipv4Address> CutRoute(Ipv4Address ipv4Address,
                                    std::vector<Ipv4Address> &nodeList);
  virtual Ptr<Ipv4Route> SetRoute(Ipv4Address nextHop, Ipv4Address srcAddress);
  bool ReverseRoutes(std::vector<Ipv4Address> &vec);
  Ipv4Address SearchNextHop(Ipv4Address ipv4Address,
                            std::vector<Ipv4Address> &vec);
  Ipv4Address ReverseSearchNextHop(Ipv4Address ipv4Address,
                                   std::vector<Ipv4Address> &vec);
  Ipv4Address ReverseSearchNextTwoHop(Ipv4Address ipv4Address,
                                      std::vector<Ipv4Address> &vec);
  void PrintVector(std::vector<Ipv4Address> &vec);
  bool IfDuplicates(std::vector<Ipv4Address> &vec,
                    std::vector<Ipv4Address> &vec2);
  bool CheckDuplicates(Ipv4Address ipv4Address, std::vector<Ipv4Address> &vec);
  void RemoveDuplicates(std::vector<Ipv4Address> &vec);
  void ScheduleReply(Ptr<Packet> &packet, std::vector<Ipv4Address> &nodeList,
                     Ipv4Address &source, Ipv4Address &destination);
  uint32_t GetIDfromIP(Ipv4Address address);
  Ptr<Node> GetNodeWithAddress(Ipv4Address ipv4Address);
  virtual uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP,
                          Ipv4Address ipv4Address, Ipv4Address source,
                          const Ipv4Header &ipv4Header, uint8_t protocol,
                          bool &isPromisc, Ipv4Address promiscSource) = 0;

protected:
  TracedCallback<Ptr<const Packet>> m_dropTrace;
  Ipv4Address Broadcast;
  Ptr<dsr::DsrRreqTable> m_rreqTable;
  Ptr<dsr::DsrRouteCache> m_routeCache;
  Ptr<Ipv4Route> m_ipv4Route;
  Ptr<Ipv4> m_ipv4;
  std::vector<Ipv4Address> m_ipv4Address;
  std::vector<Ipv4Address> m_finalRoute;
  Time ActiveRouteTimeout;
  TracedCallback<const DsrOptionSRHeader &> m_rxPacketTrace;

private:
  Ptr<Node> m_node;
};

class DsrOptionPad1 : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 224;

  static TypeId GetTypeId();

  DsrOptionPad1();
  ~DsrOptionPad1() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;
};

class DsrOptionPadn : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 0;

  static TypeId GetTypeId();

  DsrOptionPadn();
  ~DsrOptionPadn() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;
};

class DsrOptionRreq : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 1;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  DsrOptionRreq();
  ~DsrOptionRreq() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;

private:
  Ptr<dsr::DsrRouteCache> m_routeCache;
  Ptr<Ipv4> m_ipv4;
};

class DsrOptionRrep : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 2;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  DsrOptionRrep();
  ~DsrOptionRrep() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;

private:
  Ptr<dsr::DsrRouteCache> m_routeCache;
  Ptr<Ipv4> m_ipv4;
};

class DsrOptionSR : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 96;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  DsrOptionSR();
  ~DsrOptionSR() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;

private:
  Ptr<Ipv4> m_ipv4;
};

class DsrOptionRerr : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 3;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  DsrOptionRerr();
  ~DsrOptionRerr() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;
  uint8_t DoSendError(Ptr<Packet> p, DsrOptionRerrUnreachHeader &rerr,
                      uint32_t rerrSize, Ipv4Address ipv4Address,
                      uint8_t protocol);

private:
  Ptr<dsr::DsrRouteCache> m_routeCache;
  Ptr<Ipv4> m_ipv4;
};

class DsrOptionAckReq : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 160;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  DsrOptionAckReq();
  ~DsrOptionAckReq() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;

private:
  Ptr<dsr::DsrRouteCache> m_routeCache;
  Ptr<Ipv4> m_ipv4;
};

class DsrOptionAck : public DsrOptions {
public:
  static const uint8_t OPT_NUMBER = 32;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  DsrOptionAck();
  ~DsrOptionAck() override;

  uint8_t GetOptionNumber() const override;
  uint8_t Process(Ptr<Packet> packet, Ptr<Packet> dsrP, Ipv4Address ipv4Address,
                  Ipv4Address source, const Ipv4Header &ipv4Header,
                  uint8_t protocol, bool &isPromisc,
                  Ipv4Address promiscSource) override;

private:
  Ptr<dsr::DsrRouteCache> m_routeCache;
  Ptr<Ipv4> m_ipv4;
};
} // namespace dsr
} // namespace ns3

#endif
