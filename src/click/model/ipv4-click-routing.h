
#ifndef IPV4_CLICK_ROUTING_H
#define IPV4_CLICK_ROUTING_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/test.h"

#include <map>
#include <string>
#include <sys/time.h>
#include <sys/types.h>

class ClickTrivialTest;
class ClickIfidFromNameTest;
class ClickIpMacAddressFromNameTest;
struct simclick_node;
typedef struct simclick_node simclick_node_t;

namespace ns3 {

class UniformRandomVariable;

class Ipv4ClickRouting : public Ipv4RoutingProtocol {
public:
  friend class ::ClickTrivialTest;
  friend class ::ClickIfidFromNameTest;
  friend class ::ClickIpMacAddressFromNameTest;

  static TypeId GetTypeId();

  Ipv4ClickRouting();
  ~Ipv4ClickRouting() override;

  Ptr<UniformRandomVariable> GetRandomVariable();

protected:
  void DoInitialize() override;

public:
  void DoDispose() override;

  void SetClickFile(std::string clickfile);

  void SetDefines(std::map<std::string, std::string> defines);

  void SetNodeName(std::string name);

  void SetClickRoutingTableElement(std::string name);

  std::string ReadHandler(std::string elementName, std::string handlerName);

  int WriteHandler(std::string elementName, std::string handlerName,
                   std::string writeString);

  void SetPromisc(int ifid);

private:
  simclick_node_t *m_simNode;

  static std::map<simclick_node_t *, Ptr<Ipv4ClickRouting>>
      m_clickInstanceFromSimNode;

public:
  static Ptr<Ipv4ClickRouting>
  GetClickInstanceFromSimNode(simclick_node_t *simnode);

public:
  std::map<std::string, std::string> GetDefines();

  int GetInterfaceId(const char *ifname);

  std::string GetIpAddressFromInterfaceId(int ifid);

  std::string GetIpPrefixFromInterfaceId(int ifid);

  std::string GetMacAddressFromInterfaceId(int ifid);

  std::string GetNodeName();

  bool IsInterfaceReady(int ifid);

  void SetIpv4(Ptr<Ipv4> ipv4) override;

private:
  void AddSimNodeToClickMapping();

  struct timeval GetTimevalFromNow() const;

  void RunClickEvent();

public:
  void HandleScheduleFromClick(const struct timeval *when);

  void HandlePacketFromClick(int ifid, int type, const unsigned char *data,
                             int len);

  void SendPacketToClick(int ifid, int type, const unsigned char *data,
                         int len);

  void Send(Ptr<Packet> p, Ipv4Address src, Ipv4Address dest);

  void Receive(Ptr<Packet> p, Mac48Address receiverAddr, Mac48Address dest);

  Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header,
                             Ptr<NetDevice> oif,
                             Socket::SocketErrno &sockerr) override;
  bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                  Ptr<const NetDevice> idev, const UnicastForwardCallback &ucb,
                  const MulticastForwardCallback &mcb,
                  const LocalDeliverCallback &lcb,
                  const ErrorCallback &ecb) override;
  void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                         Time::Unit unit = Time::S) const override;
  void NotifyInterfaceUp(uint32_t interface) override;
  void NotifyInterfaceDown(uint32_t interface) override;
  void NotifyAddAddress(uint32_t interface,
                        Ipv4InterfaceAddress address) override;
  void NotifyRemoveAddress(uint32_t interface,
                           Ipv4InterfaceAddress address) override;

private:
  std::string m_clickFile;
  std::map<std::string, std::string> m_defines;
  std::string m_nodeName;
  std::string m_clickRoutingTableElement;

  bool m_clickInitialised;
  bool m_nonDefaultName;

  Ptr<Ipv4> m_ipv4;
  Ptr<UniformRandomVariable> m_random;
};

} // namespace ns3

#endif
