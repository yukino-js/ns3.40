
#ifndef POINT_TO_POINT_EPC_HELPER_H
#define POINT_TO_POINT_EPC_HELPER_H

#include "no-backhaul-epc-helper.h"

namespace ns3 {

class PointToPointEpcHelper : public NoBackhaulEpcHelper {
public:
  PointToPointEpcHelper();

  ~PointToPointEpcHelper() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void DoDispose() override;

  void AddEnb(Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice,
              std::vector<uint16_t> cellIds) override;

private:
  Ipv4AddressHelper m_s1uIpv4AddressHelper;

  DataRate m_s1uLinkDataRate;

  Time m_s1uLinkDelay;

  uint16_t m_s1uLinkMtu;

  Ipv4AddressHelper m_s1apIpv4AddressHelper;

  bool m_s1uLinkEnablePcap;

  std::string m_s1uLinkPcapPrefix;
};

} // namespace ns3

#endif
