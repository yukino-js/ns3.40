
#ifndef EMU_EPC_HELPER_H
#define EMU_EPC_HELPER_H

#include "no-backhaul-epc-helper.h"

namespace ns3 {

class EmuEpcHelper : public NoBackhaulEpcHelper {
public:
  EmuEpcHelper();

  ~EmuEpcHelper() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void DoDispose() override;

  void AddEnb(Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice,
              std::vector<uint16_t> cellIds) override;
  void AddX2Interface(Ptr<Node> enbNode1, Ptr<Node> enbNode2) override;

private:
  Ipv4AddressHelper m_epcIpv4AddressHelper;

  Ipv4InterfaceContainer m_sgwIpIfaces;

  std::string m_sgwDeviceName;

  std::string m_enbDeviceName;

  std::string m_sgwMacAddress;

  std::string m_enbMacAddressBase;
};

} // namespace ns3

#endif
