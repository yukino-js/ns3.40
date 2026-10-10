

#ifndef POINT_TO_POINT_GRID_HELPER_H
#define POINT_TO_POINT_GRID_HELPER_H

#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-interface-container.h"
#include "ns3/net-device-container.h"
#include "ns3/point-to-point-helper.h"

#include <vector>

namespace ns3 {

class PointToPointGridHelper {
public:
  PointToPointGridHelper(uint32_t nRows, uint32_t nCols,
                         PointToPointHelper pointToPoint);

  ~PointToPointGridHelper();

  Ptr<Node> GetNode(uint32_t row, uint32_t col);

  Ipv4Address GetIpv4Address(uint32_t row, uint32_t col);

  Ipv6Address GetIpv6Address(uint32_t row, uint32_t col);

  void InstallStack(InternetStackHelper stack);

  void AssignIpv4Addresses(Ipv4AddressHelper rowIp, Ipv4AddressHelper colIp);

  void AssignIpv6Addresses(Ipv6Address network, Ipv6Prefix prefix);

  void BoundingBox(double ulx, double uly, double lrx, double lry);

private:
  uint32_t m_xSize;
  uint32_t m_ySize;
  std::vector<NetDeviceContainer> m_rowDevices;
  std::vector<NetDeviceContainer> m_colDevices;
  std::vector<Ipv4InterfaceContainer> m_rowInterfaces;
  std::vector<Ipv4InterfaceContainer> m_colInterfaces;
  std::vector<Ipv6InterfaceContainer> m_rowInterfaces6;
  std::vector<Ipv6InterfaceContainer> m_colInterfaces6;
  std::vector<NodeContainer> m_nodes;
};

} // namespace ns3

#endif
