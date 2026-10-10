
#ifndef EPC_ENB_APPLICATION_H
#define EPC_ENB_APPLICATION_H

#include "epc-enb-s1-sap.h"
#include "epc-s1ap-sap.h"

#include <ns3/address.h>
#include <ns3/application.h>
#include <ns3/callback.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/socket.h>
#include <ns3/traced-callback.h>
#include <ns3/virtual-net-device.h>

#include <map>

namespace ns3 {
class EpcEnbS1SapUser;
class EpcEnbS1SapProvider;

class EpcEnbApplication : public Application {
  friend class MemberEpcEnbS1SapProvider<EpcEnbApplication>;
  friend class MemberEpcS1apSapEnb<EpcEnbApplication>;

public:
  static TypeId GetTypeId();

protected:
  void DoDispose() override;

public:
  EpcEnbApplication(Ptr<Socket> lteSocket, Ptr<Socket> lteSocket6,
                    uint16_t cellId);

  void AddS1Interface(Ptr<Socket> s1uSocket, Ipv4Address enbS1uAddress,
                      Ipv4Address sgwS1uAddress);

  ~EpcEnbApplication() override;

  void SetS1SapUser(EpcEnbS1SapUser *s);

  EpcEnbS1SapProvider *GetS1SapProvider();

  void SetS1apSapMme(EpcS1apSapMme *s);

  EpcS1apSapEnb *GetS1apSapEnb();

  void RecvFromLteSocket(Ptr<Socket> socket);

  void RecvFromS1uSocket(Ptr<Socket> socket);

  typedef void (*RxTracedCallback)(Ptr<Packet> packet);

  struct EpsFlowId_t {
    uint16_t m_rnti;
    uint8_t m_bid;

  public:
    EpsFlowId_t();
    EpsFlowId_t(const uint16_t a, const uint8_t b);

    friend bool operator==(const EpsFlowId_t &a, const EpsFlowId_t &b);
    friend bool operator<(const EpsFlowId_t &a, const EpsFlowId_t &b);
  };

private:
  void DoInitialUeMessage(uint64_t imsi, uint16_t rnti);
  void
  DoPathSwitchRequest(EpcEnbS1SapProvider::PathSwitchRequestParameters params);
  void DoUeContextRelease(uint16_t rnti);

  void DoInitialContextSetupRequest(
      uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
      std::list<EpcS1apSapEnb::ErabToBeSetupItem> erabToBeSetupList);
  void DoPathSwitchRequestAcknowledge(
      uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi,
      std::list<EpcS1apSapEnb::ErabSwitchedInUplinkItem>
          erabToBeSwitchedInUplinkList);

  void DoReleaseIndication(uint64_t imsi, uint16_t rnti, uint8_t bearerId);

  void SendToLteSocket(Ptr<Packet> packet, uint16_t rnti, uint8_t bid);

  void SendToS1uSocket(Ptr<Packet> packet, uint32_t teid);

  void SetupS1Bearer(uint32_t teid, uint16_t rnti, uint8_t bid);

  Ptr<Socket> m_lteSocket;

  Ptr<Socket> m_lteSocket6;

  Ptr<Socket> m_s1uSocket;

  Ipv4Address m_enbS1uAddress;

  Ipv4Address m_sgwS1uAddress;

  std::map<uint16_t, std::map<uint8_t, uint32_t>> m_rbidTeidMap;

  std::map<uint32_t, EpsFlowId_t> m_teidRbidMap;

  uint16_t m_gtpuUdpPort;

  EpcEnbS1SapProvider *m_s1SapProvider;

  EpcEnbS1SapUser *m_s1SapUser;

  EpcS1apSapMme *m_s1apSapMme;

  EpcS1apSapEnb *m_s1apSapEnb;

  std::map<uint64_t, uint16_t> m_imsiRntiMap;

  uint16_t m_cellId;

  TracedCallback<Ptr<Packet>> m_rxLteSocketPktTrace;

  TracedCallback<Ptr<Packet>> m_rxS1uSocketPktTrace;
};

} // namespace ns3

#endif
