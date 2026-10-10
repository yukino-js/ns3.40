

#ifndef POINT_TO_POINT_DUMBBELL_HELPER_H
#define POINT_TO_POINT_DUMBBELL_HELPER_H

#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-interface-container.h"
#include "ns3/point-to-point-helper.h"

#include <string>

namespace ns3 {

class PointToPointDumbbellHelper {
public:
  PointToPointDumbbellHelper(uint32_t nLeftLeaf, PointToPointHelper leftHelper,
                             uint32_t nRightLeaf,
                             PointToPointHelper rightHelper,
                             PointToPointHelper bottleneckHelper);

  ~PointToPointDumbbellHelper();

public:
  Ptr<Node> GetLeft() const;

  Ptr<Node> GetLeft(uint32_t i) const;

  Ptr<Node> GetRight() const;

  Ptr<Node> GetRight(uint32_t i) const;

  Ipv4Address GetLeftIpv4Address(uint32_t i) const;

  Ipv4Address GetRightIpv4Address(uint32_t i) const;

  Ipv6Address GetLeftIpv6Address(uint32_t i) const;

  Ipv6Address GetRightIpv6Address(uint32_t i) const;

  uint32_t LeftCount() const;

  uint32_t RightCount() const;

  void InstallStack(InternetStackHelper stack);

  void AssignIpv4Addresses(Ipv4AddressHelper leftIp, Ipv4AddressHelper rightIp,
                           Ipv4AddressHelper routerIp);

  void AssignIpv6Addresses(Ipv6Address network, Ipv6Prefix prefix);

  void BoundingBox(double ulx, double uly, double lrx, double lry) const;

private:
  NodeContainer m_leftLeaf;
  NetDeviceContainer m_leftLeafDevices;
  NodeContainer m_rightLeaf;
  NetDeviceContainer m_rightLeafDevices;
  NodeContainer m_routers;
  NetDeviceContainer m_routerDevices;
  NetDeviceContainer m_leftRouterDevices;
  NetDeviceContainer m_rightRouterDevices;
  Ipv4InterfaceContainer m_leftLeafInterfaces;
  Ipv4InterfaceContainer m_leftRouterInterfaces;
  Ipv4InterfaceContainer m_rightLeafInterfaces;
  Ipv4InterfaceContainer m_rightRouterInterfaces;
  Ipv4InterfaceContainer m_routerInterfaces;
  Ipv6InterfaceContainer m_leftLeafInterfaces6;
  Ipv6InterfaceContainer m_leftRouterInterfaces6;
  Ipv6InterfaceContainer m_rightLeafInterfaces6;
  Ipv6InterfaceContainer m_rightRouterInterfaces6;
  Ipv6InterfaceContainer m_routerInterfaces6;
};

} // namespace ns3

#endif
