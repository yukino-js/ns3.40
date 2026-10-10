

#ifndef CSMA_STAR_HELPER_H
#define CSMA_STAR_HELPER_H

#include "ns3/csma-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-interface-container.h"

#include <string>

namespace ns3 {

class CsmaStarHelper {
public:
  CsmaStarHelper(uint32_t numSpokes, CsmaHelper csmaHelper);

  ~CsmaStarHelper();

public:
  Ptr<Node> GetHub() const;

  Ptr<Node> GetSpokeNode(uint32_t i) const;

  NetDeviceContainer GetHubDevices() const;

  NetDeviceContainer GetSpokeDevices() const;

  Ipv4Address GetHubIpv4Address(uint32_t i) const;

  Ipv6Address GetHubIpv6Address(uint32_t i) const;

  Ipv4Address GetSpokeIpv4Address(uint32_t i) const;

  Ipv6Address GetSpokeIpv6Address(uint32_t i) const;

  uint32_t SpokeCount() const;

  void InstallStack(InternetStackHelper stack);

  void AssignIpv4Addresses(Ipv4AddressHelper address);

  void AssignIpv6Addresses(Ipv6Address network, Ipv6Prefix prefix);

private:
  NodeContainer m_hub;
  NetDeviceContainer m_hubDevices;
  NodeContainer m_spokes;
  NetDeviceContainer m_spokeDevices;
  Ipv4InterfaceContainer m_hubInterfaces;
  Ipv4InterfaceContainer m_spokeInterfaces;
  Ipv6InterfaceContainer m_hubInterfaces6;
  Ipv6InterfaceContainer m_spokeInterfaces6;
};

} // namespace ns3

#endif
