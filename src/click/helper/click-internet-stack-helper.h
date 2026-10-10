
#ifndef CLICK_INTERNET_STACK_HELPER_H
#define CLICK_INTERNET_STACK_HELPER_H

#include "ns3/internet-trace-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <map>

namespace ns3 {

class Node;
class Ipv4RoutingHelper;

class ClickInternetStackHelper : public PcapHelperForIpv4,
                                 public AsciiTraceHelperForIpv4 {
public:
  ClickInternetStackHelper();

  ~ClickInternetStackHelper() override;

  ClickInternetStackHelper(const ClickInternetStackHelper &o);

  ClickInternetStackHelper &operator=(const ClickInternetStackHelper &o);

  void Reset();

  void Install(std::string nodeName) const;

  void Install(Ptr<Node> node) const;

  void Install(NodeContainer c) const;

  void InstallAll() const;

  void SetClickFile(NodeContainer c, std::string clickfile);

  void SetClickFile(Ptr<Node> node, std::string clickfile);

  void SetDefines(NodeContainer c, std::map<std::string, std::string> defines);

  void SetDefines(Ptr<Node> node, std::map<std::string, std::string> defines);

  void SetRoutingTableElement(NodeContainer c, std::string rt);

  void SetRoutingTableElement(Ptr<Node> node, std::string rt);

private:
  void EnablePcapIpv4Internal(std::string prefix, Ptr<Ipv4> ipv4,
                              uint32_t interface,
                              bool explicitFilename) override;

  void EnableAsciiIpv4Internal(Ptr<OutputStreamWrapper> stream,
                               std::string prefix, Ptr<Ipv4> ipv4,
                               uint32_t interface,
                               bool explicitFilename) override;

  void Initialize();

  static void CreateAndAggregateObjectFromTypeId(Ptr<Node> node,
                                                 const std::string typeId);

  bool PcapHooked(Ptr<Ipv4> ipv4);

  bool AsciiHooked(Ptr<Ipv4> ipv4);

  bool m_ipv4Enabled;

  std::map<Ptr<Node>, std::string> m_nodeToClickFileMap;

  std::map<Ptr<Node>, std::map<std::string, std::string>> m_nodeToDefinesMap;

  std::map<Ptr<Node>, std::string> m_nodeToRoutingTableElementMap;
};

} // namespace ns3

#endif
