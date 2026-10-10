
#ifndef INTERNET_STACK_HELPER_H
#define INTERNET_STACK_HELPER_H

#include "internet-trace-helper.h"

#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

namespace ns3 {

class Node;
class Ipv4RoutingHelper;
class Ipv6RoutingHelper;

class InternetStackHelper : public PcapHelperForIpv4,
                            public PcapHelperForIpv6,
                            public AsciiTraceHelperForIpv4,
                            public AsciiTraceHelperForIpv6 {
public:
  InternetStackHelper();

  ~InternetStackHelper() override;

  InternetStackHelper(const InternetStackHelper &o);

  InternetStackHelper &operator=(const InternetStackHelper &o);

  void Reset();

  void SetRoutingHelper(const Ipv4RoutingHelper &routing);

  void SetRoutingHelper(const Ipv6RoutingHelper &routing);

  void Install(std::string nodeName) const;

  void Install(Ptr<Node> node) const;

  void Install(NodeContainer c) const;

  void InstallAll() const;

  void SetIpv4StackInstall(bool enable);

  void SetIpv6StackInstall(bool enable);

  void SetIpv4ArpJitter(bool enable);

  void SetIpv6NsRsJitter(bool enable);

  int64_t AssignStreams(NodeContainer c, int64_t stream);

private:
  void EnablePcapIpv4Internal(std::string prefix, Ptr<Ipv4> ipv4,
                              uint32_t interface,
                              bool explicitFilename) override;

  void EnableAsciiIpv4Internal(Ptr<OutputStreamWrapper> stream,
                               std::string prefix, Ptr<Ipv4> ipv4,
                               uint32_t interface,
                               bool explicitFilename) override;

  void EnablePcapIpv6Internal(std::string prefix, Ptr<Ipv6> ipv6,
                              uint32_t interface,
                              bool explicitFilename) override;

  void EnableAsciiIpv6Internal(Ptr<OutputStreamWrapper> stream,
                               std::string prefix, Ptr<Ipv6> ipv6,
                               uint32_t interface,
                               bool explicitFilename) override;

  void Initialize();

  const Ipv4RoutingHelper *m_routing;

  const Ipv6RoutingHelper *m_routingv6;

  static void CreateAndAggregateObjectFromTypeId(Ptr<Node> node,
                                                 const std::string typeId);

  bool PcapHooked(Ptr<Ipv4> ipv4);

  bool AsciiHooked(Ptr<Ipv4> ipv4);

  bool PcapHooked(Ptr<Ipv6> ipv6);

  bool AsciiHooked(Ptr<Ipv6> ipv6);

  bool m_ipv4Enabled;

  bool m_ipv6Enabled;

  bool m_ipv4ArpJitterEnabled;

  bool m_ipv6NsRsJitterEnabled;
};

} // namespace ns3

#endif
