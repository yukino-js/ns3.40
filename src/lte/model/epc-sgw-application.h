
#ifndef EPC_SGW_APPLICATION_H
#define EPC_SGW_APPLICATION_H

#include "epc-gtpc-header.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/socket.h"

#include <map>

namespace ns3 {

class EpcSgwApplication : public Application {
public:
  static TypeId GetTypeId();
  void DoDispose() override;

  EpcSgwApplication(const Ptr<Socket> s1uSocket, Ipv4Address s5Addr,
                    const Ptr<Socket> s5uSocket, const Ptr<Socket> s5cSocket);

  ~EpcSgwApplication() override;

  void AddMme(Ipv4Address mmeS11Addr, Ptr<Socket> s11Socket);

  void AddPgw(Ipv4Address pgwAddr);

  void AddEnb(uint16_t cellId, Ipv4Address enbAddr, Ipv4Address sgwAddr);

private:
  void RecvFromS11Socket(Ptr<Socket> socket);

  void RecvFromS5uSocket(Ptr<Socket> socket);

  void RecvFromS5cSocket(Ptr<Socket> socket);

  void RecvFromS1uSocket(Ptr<Socket> socket);

  void SendToS5uSocket(Ptr<Packet> packet, Ipv4Address pgwAddr, uint32_t teid);

  void SendToS1uSocket(Ptr<Packet> packet, Ipv4Address enbS1uAddress,
                       uint32_t teid);

  void DoRecvCreateSessionRequest(Ptr<Packet> packet);

  void DoRecvModifyBearerRequest(Ptr<Packet> packet);

  void DoRecvDeleteBearerCommand(Ptr<Packet> packet);

  void DoRecvDeleteBearerResponse(Ptr<Packet> packet);

  void DoRecvCreateSessionResponse(Ptr<Packet> packet);

  void DoRecvModifyBearerResponse(Ptr<Packet> packet);

  void DoRecvDeleteBearerRequest(Ptr<Packet> packet);

  Ipv4Address m_s5Addr;

  Ipv4Address m_mmeS11Addr;

  Ptr<Socket> m_s11Socket;

  Ipv4Address m_pgwAddr;

  Ptr<Socket> m_s5uSocket;

  Ptr<Socket> m_s5cSocket;

  Ptr<Socket> m_s1uSocket;

  uint16_t m_gtpuUdpPort;

  uint16_t m_gtpcUdpPort;

  uint32_t m_teidCount;

  struct EnbInfo {
    Ipv4Address enbAddr;
    Ipv4Address sgwAddr;
  };

  std::map<uint16_t, EnbInfo> m_enbInfoByCellId;

  std::map<uint32_t, Ipv4Address> m_enbByTeidMap;

  std::map<uint32_t, GtpcHeader::Fteid_t> m_mmeS11FteidBySgwS5cTeid;
};

} // namespace ns3

#endif
