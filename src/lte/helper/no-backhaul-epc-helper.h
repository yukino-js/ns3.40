
#ifndef NO_BACKHAUL_EPC_HELPER_H
#define NO_BACKHAUL_EPC_HELPER_H

#include "epc-helper.h"

namespace ns3 {

class EpcSgwApplication;
class EpcPgwApplication;
class EpcMmeApplication;

class NoBackhaulEpcHelper : public EpcHelper {
public:
  NoBackhaulEpcHelper();

  ~NoBackhaulEpcHelper() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void DoDispose() override;

  void AddEnb(Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice,
              std::vector<uint16_t> cellIds) override;
  void AddUe(Ptr<NetDevice> ueLteDevice, uint64_t imsi) override;
  void AddX2Interface(Ptr<Node> enbNode1, Ptr<Node> enbNode2) override;
  void AddS1Interface(Ptr<Node> enb, Ipv4Address enbAddress,
                      Ipv4Address sgwAddress,
                      std::vector<uint16_t> cellIds) override;
  uint8_t ActivateEpsBearer(Ptr<NetDevice> ueLteDevice, uint64_t imsi,
                            Ptr<EpcTft> tft, EpsBearer bearer) override;
  Ptr<Node> GetSgwNode() const override;
  Ptr<Node> GetPgwNode() const override;
  Ipv4InterfaceContainer
  AssignUeIpv4Address(NetDeviceContainer ueDevices) override;
  Ipv6InterfaceContainer
  AssignUeIpv6Address(NetDeviceContainer ueDevices) override;
  Ipv4Address GetUeDefaultGatewayAddress() override;
  Ipv6Address GetUeDefaultGatewayAddress6() override;
  int64_t AssignStreams(int64_t stream) override;

protected:
  virtual void DoAddX2Interface(const Ptr<EpcX2> &enb1X2,
                                const Ptr<NetDevice> &enb1LteDev,
                                const Ipv4Address &enb1X2Address,
                                const Ptr<EpcX2> &enb2X2,
                                const Ptr<NetDevice> &enb2LteDev,
                                const Ipv4Address &enb2X2Address) const;

  virtual void DoActivateEpsBearerForUe(const Ptr<NetDevice> &ueDevice,
                                        const Ptr<EpcTft> &tft,
                                        const EpsBearer &bearer) const;

private:
  Ipv4AddressHelper m_uePgwAddressHelper;
  Ipv6AddressHelper m_uePgwAddressHelper6;

  Ptr<Node> m_pgw;

  Ptr<Node> m_sgw;

  Ptr<Node> m_mme;

  Ptr<EpcSgwApplication> m_sgwApp;

  Ptr<EpcPgwApplication> m_pgwApp;

  Ptr<EpcMmeApplication> m_mmeApp;

  Ptr<VirtualNetDevice> m_tunDevice;

  uint16_t m_gtpuUdpPort;

  Ipv4AddressHelper m_s11Ipv4AddressHelper;

  DataRate m_s11LinkDataRate;

  Time m_s11LinkDelay;

  uint16_t m_s11LinkMtu;

  uint16_t m_gtpcUdpPort;

  Ipv4AddressHelper m_s5Ipv4AddressHelper;

  DataRate m_s5LinkDataRate;

  Time m_s5LinkDelay;

  uint16_t m_s5LinkMtu;

  std::map<uint64_t, Ptr<NetDevice>> m_imsiEnbDeviceMap;

  Ipv4AddressHelper m_x2Ipv4AddressHelper;

  DataRate m_x2LinkDataRate;

  Time m_x2LinkDelay;

  uint16_t m_x2LinkMtu;

  bool m_x2LinkEnablePcap;

  std::string m_x2LinkPcapPrefix;
};

} // namespace ns3

#endif
