
#ifndef EPC_PGW_APPLICATION_H
#define EPC_PGW_APPLICATION_H

#include "epc-gtpc-header.h"
#include "epc-tft-classifier.h"

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/virtual-net-device.h"

namespace ns3 {

class EpcPgwApplication : public Application {
public:
  static TypeId GetTypeId();
  void DoDispose() override;

  EpcPgwApplication(const Ptr<VirtualNetDevice> tunDevice, Ipv4Address s5Addr,
                    const Ptr<Socket> s5uSocket, const Ptr<Socket> s5cSocket);

  ~EpcPgwApplication() override;

  bool RecvFromTunDevice(Ptr<Packet> packet, const Address &source,
                         const Address &dest, uint16_t protocolNumber);

  void RecvFromS5uSocket(Ptr<Socket> socket);

  void RecvFromS5cSocket(Ptr<Socket> socket);

  void SendToTunDevice(Ptr<Packet> packet, uint32_t teid);

  void SendToS5uSocket(Ptr<Packet> packet, Ipv4Address sgwS5uAddress,
                       uint32_t teid);

  void AddSgw(Ipv4Address sgwS5Addr);

  void AddUe(uint64_t imsi);

  void SetUeAddress(uint64_t imsi, Ipv4Address ueAddr);

  void SetUeAddress6(uint64_t imsi, Ipv6Address ueAddr);

  typedef void (*RxTracedCallback)(Ptr<Packet> packet);

private:
  void DoRecvCreateSessionRequest(Ptr<Packet> packet);

  void DoRecvModifyBearerRequest(Ptr<Packet> packet);

  void DoRecvDeleteBearerCommand(Ptr<Packet> packet);

  void DoRecvDeleteBearerResponse(Ptr<Packet> packet);

  class UeInfo : public SimpleRefCount<UeInfo> {
  public:
    UeInfo();

    void AddBearer(uint8_t bearerId, uint32_t teid, Ptr<EpcTft> tft);

    void RemoveBearer(uint8_t bearerId);

    uint32_t Classify(Ptr<Packet> p, uint16_t protocolNumber);

    Ipv4Address GetSgwAddr();

    void SetSgwAddr(Ipv4Address addr);

    Ipv4Address GetUeAddr();

    void SetUeAddr(Ipv4Address addr);

    Ipv6Address GetUeAddr6();

    void SetUeAddr6(Ipv6Address addr);

  private:
    Ipv4Address m_ueAddr;
    Ipv6Address m_ueAddr6;
    Ipv4Address m_sgwAddr;
    EpcTftClassifier m_tftClassifier;
    std::map<uint8_t, uint32_t> m_teidByBearerIdMap;
  };

  Ipv4Address m_pgwS5Addr;

  Ptr<Socket> m_s5uSocket;

  Ptr<Socket> m_s5cSocket;

  Ptr<VirtualNetDevice> m_tunDevice;

  std::map<Ipv4Address, Ptr<UeInfo>> m_ueInfoByAddrMap;

  std::map<Ipv6Address, Ptr<UeInfo>> m_ueInfoByAddrMap6;

  std::map<uint64_t, Ptr<UeInfo>> m_ueInfoByImsiMap;

  uint16_t m_gtpuUdpPort;

  uint16_t m_gtpcUdpPort;

  Ipv4Address m_sgwS5Addr;

  TracedCallback<Ptr<Packet>> m_rxTunPktTrace;

  TracedCallback<Ptr<Packet>> m_rxS5PktTrace;
};

} // namespace ns3

#endif
