
#ifndef EPC_HELPER_H
#define EPC_HELPER_H

#include <ns3/data-rate.h>
#include <ns3/epc-tft.h>
#include <ns3/eps-bearer.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv6-address-helper.h>
#include <ns3/object.h>

namespace ns3 {

class Node;
class NetDevice;
class VirtualNetDevice;
class EpcX2;

class EpcHelper : public Object {
public:
  EpcHelper();

  ~EpcHelper() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  virtual void AddEnb(Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice,
                      std::vector<uint16_t> cellIds) = 0;

  virtual void AddUe(Ptr<NetDevice> ueLteDevice, uint64_t imsi) = 0;

  virtual void AddX2Interface(Ptr<Node> enbNode1, Ptr<Node> enbNode2) = 0;

  virtual void AddS1Interface(Ptr<Node> enb, Ipv4Address enbAddress,
                              Ipv4Address sgwAddress,
                              std::vector<uint16_t> cellIds) = 0;

  virtual uint8_t ActivateEpsBearer(Ptr<NetDevice> ueLteDevice, uint64_t imsi,
                                    Ptr<EpcTft> tft, EpsBearer bearer) = 0;

  virtual Ptr<Node> GetSgwNode() const = 0;

  virtual Ptr<Node> GetPgwNode() const = 0;

  virtual Ipv4InterfaceContainer
  AssignUeIpv4Address(NetDeviceContainer ueDevices) = 0;

  virtual Ipv6InterfaceContainer
  AssignUeIpv6Address(NetDeviceContainer ueDevices) = 0;

  virtual Ipv4Address GetUeDefaultGatewayAddress() = 0;

  virtual Ipv6Address GetUeDefaultGatewayAddress6() = 0;

  virtual int64_t AssignStreams(int64_t stream) = 0;
};

} // namespace ns3

#endif
