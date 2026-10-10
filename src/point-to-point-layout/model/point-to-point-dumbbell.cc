

#include "point-to-point-dumbbell.h"

#include "ns3/constant-position-mobility-model.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/vector.h"

#include <cmath>
#include <iostream>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PointToPointDumbbellHelper");

PointToPointDumbbellHelper::PointToPointDumbbellHelper(
    uint32_t nLeftLeaf, PointToPointHelper leftHelper, uint32_t nRightLeaf,
    PointToPointHelper rightHelper, PointToPointHelper bottleneckHelper) {
  m_routers.Create(2);
  m_leftLeaf.Create(nLeftLeaf);
  m_rightLeaf.Create(nRightLeaf);

  m_routerDevices = bottleneckHelper.Install(m_routers);
  for (uint32_t i = 0; i < nLeftLeaf; ++i) {
    NetDeviceContainer c =
        leftHelper.Install(m_routers.Get(0), m_leftLeaf.Get(i));
    m_leftRouterDevices.Add(c.Get(0));
    m_leftLeafDevices.Add(c.Get(1));
  }
  for (uint32_t i = 0; i < nRightLeaf; ++i) {
    NetDeviceContainer c =
        rightHelper.Install(m_routers.Get(1), m_rightLeaf.Get(i));
    m_rightRouterDevices.Add(c.Get(0));
    m_rightLeafDevices.Add(c.Get(1));
  }
}

PointToPointDumbbellHelper::~PointToPointDumbbellHelper() {}

Ptr<Node> PointToPointDumbbellHelper::GetLeft() const {
  return m_routers.Get(0);
}

Ptr<Node> PointToPointDumbbellHelper::GetLeft(uint32_t i) const {
  return m_leftLeaf.Get(i);
}

Ptr<Node> PointToPointDumbbellHelper::GetRight() const {
  return m_routers.Get(1);
}

Ptr<Node> PointToPointDumbbellHelper::GetRight(uint32_t i) const {
  return m_rightLeaf.Get(i);
}

Ipv4Address PointToPointDumbbellHelper::GetLeftIpv4Address(uint32_t i) const {
  return m_leftLeafInterfaces.GetAddress(i);
}

Ipv4Address PointToPointDumbbellHelper::GetRightIpv4Address(uint32_t i) const {
  return m_rightLeafInterfaces.GetAddress(i);
}

Ipv6Address PointToPointDumbbellHelper::GetLeftIpv6Address(uint32_t i) const {
  return m_leftLeafInterfaces6.GetAddress(i, 1);
}

Ipv6Address PointToPointDumbbellHelper::GetRightIpv6Address(uint32_t i) const {
  return m_rightLeafInterfaces6.GetAddress(i, 1);
}

uint32_t PointToPointDumbbellHelper::LeftCount() const {
  return m_leftLeaf.GetN();
}

uint32_t PointToPointDumbbellHelper::RightCount() const {
  return m_rightLeaf.GetN();
}

void PointToPointDumbbellHelper::InstallStack(InternetStackHelper stack) {
  stack.Install(m_routers);
  stack.Install(m_leftLeaf);
  stack.Install(m_rightLeaf);
}

void PointToPointDumbbellHelper::AssignIpv4Addresses(
    Ipv4AddressHelper leftIp, Ipv4AddressHelper rightIp,
    Ipv4AddressHelper routerIp) {
  m_routerInterfaces = routerIp.Assign(m_routerDevices);
  for (uint32_t i = 0; i < LeftCount(); ++i) {
    NetDeviceContainer ndc;
    ndc.Add(m_leftLeafDevices.Get(i));
    ndc.Add(m_leftRouterDevices.Get(i));
    Ipv4InterfaceContainer ifc = leftIp.Assign(ndc);
    m_leftLeafInterfaces.Add(ifc.Get(0));
    m_leftRouterInterfaces.Add(ifc.Get(1));
    leftIp.NewNetwork();
  }
  for (uint32_t i = 0; i < RightCount(); ++i) {
    NetDeviceContainer ndc;
    ndc.Add(m_rightLeafDevices.Get(i));
    ndc.Add(m_rightRouterDevices.Get(i));
    Ipv4InterfaceContainer ifc = rightIp.Assign(ndc);
    m_rightLeafInterfaces.Add(ifc.Get(0));
    m_rightRouterInterfaces.Add(ifc.Get(1));
    rightIp.NewNetwork();
  }
}

void PointToPointDumbbellHelper::AssignIpv6Addresses(Ipv6Address addrBase,
                                                     Ipv6Prefix prefix) {
  Ipv6AddressGenerator::Init(addrBase, prefix);
  Ipv6Address v6network;
  Ipv6AddressHelper addressHelper;

  v6network = Ipv6AddressGenerator::GetNetwork(prefix);
  addressHelper.SetBase(v6network, prefix);
  m_routerInterfaces6 = addressHelper.Assign(m_routerDevices);
  Ipv6AddressGenerator::NextNetwork(prefix);

  for (uint32_t i = 0; i < LeftCount(); ++i) {
    v6network = Ipv6AddressGenerator::GetNetwork(prefix);
    addressHelper.SetBase(v6network, prefix);

    NetDeviceContainer ndc;
    ndc.Add(m_leftLeafDevices.Get(i));
    ndc.Add(m_leftRouterDevices.Get(i));
    Ipv6InterfaceContainer ifc = addressHelper.Assign(ndc);
    auto it = ifc.Begin();
    m_leftLeafInterfaces6.Add((*it).first, (*it).second);
    it++;
    m_leftRouterInterfaces6.Add((*it).first, (*it).second);
    Ipv6AddressGenerator::NextNetwork(prefix);
  }
  for (uint32_t i = 0; i < RightCount(); ++i) {
    v6network = Ipv6AddressGenerator::GetNetwork(prefix);
    addressHelper.SetBase(v6network, prefix);

    NetDeviceContainer ndc;
    ndc.Add(m_rightLeafDevices.Get(i));
    ndc.Add(m_rightRouterDevices.Get(i));
    Ipv6InterfaceContainer ifc = addressHelper.Assign(ndc);
    auto it = ifc.Begin();
    m_rightLeafInterfaces6.Add((*it).first, (*it).second);
    it++;
    m_rightRouterInterfaces6.Add((*it).first, (*it).second);
    Ipv6AddressGenerator::NextNetwork(prefix);
  }
}

void PointToPointDumbbellHelper::BoundingBox(double ulx, double uly, double lrx,
                                             double lry) const {
  double xDist;
  double yDist;
  if (lrx > ulx) {
    xDist = lrx - ulx;
  } else {
    xDist = ulx - lrx;
  }
  if (lry > uly) {
    yDist = lry - uly;
  } else {
    yDist = uly - lry;
  }

  double xAdder = xDist / 3.0;
  double thetaL = M_PI / (LeftCount() + 1.0);
  double thetaR = M_PI / (RightCount() + 1.0);

  Ptr<Node> lr = GetLeft();
  Ptr<ConstantPositionMobilityModel> loc =
      lr->GetObject<ConstantPositionMobilityModel>();
  if (!loc) {
    loc = CreateObject<ConstantPositionMobilityModel>();
    lr->AggregateObject(loc);
  }
  Vector lrl(ulx + xAdder, uly + yDist / 2.0, 0);
  loc->SetPosition(lrl);

  Ptr<Node> rr = GetRight();
  loc = rr->GetObject<ConstantPositionMobilityModel>();
  if (!loc) {
    loc = CreateObject<ConstantPositionMobilityModel>();
    rr->AggregateObject(loc);
  }
  Vector rrl(ulx + xAdder * 2, uly + yDist / 2.0, 0);
  loc->SetPosition(rrl);

  double theta = -M_PI_2 + thetaL;
  for (uint32_t l = 0; l < LeftCount(); ++l) {
    if ((LeftCount() % 2) == 1) {
      if (l == (LeftCount() / 2)) {
        theta = 0.0;
      }
    }
    Ptr<Node> ln = GetLeft(l);
    loc = ln->GetObject<ConstantPositionMobilityModel>();
    if (!loc) {
      loc = CreateObject<ConstantPositionMobilityModel>();
      ln->AggregateObject(loc);
    }
    Vector lnl(lrl.x - std::cos(theta) * xAdder,
               lrl.y + std::sin(theta) * xAdder, 0);
    if (lnl.y < uly) {
      lnl.y = uly;
    }
    if (lnl.y > lry) {
      lnl.y = lry;
    }
    loc->SetPosition(lnl);
    theta += thetaL;
  }
  theta = -M_PI_2 + thetaR;
  for (uint32_t r = 0; r < RightCount(); ++r) {
    if ((RightCount() % 2) == 1) {
      if (r == (RightCount() / 2)) {
        theta = 0.0;
      }
    }
    Ptr<Node> rn = GetRight(r);
    loc = rn->GetObject<ConstantPositionMobilityModel>();
    if (!loc) {
      loc = CreateObject<ConstantPositionMobilityModel>();
      rn->AggregateObject(loc);
    }
    Vector rnl(rrl.x + std::cos(theta) * xAdder,
               rrl.y + std::sin(theta) * xAdder, 0);
    if (rnl.y < uly) {
      rnl.y = uly;
    }
    if (rnl.y > lry) {
      rnl.y = lry;
    }
    loc->SetPosition(rnl);
    theta += thetaR;
  }
}

} // namespace ns3
