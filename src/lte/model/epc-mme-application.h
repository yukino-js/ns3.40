
#ifndef EPC_MME_APPLICATION_H
#define EPC_MME_APPLICATION_H

#include "epc-gtpc-header.h"
#include "epc-s1ap-sap.h"

#include "ns3/application.h"
#include "ns3/socket.h"

#include <map>

namespace ns3 {

class EpcMmeApplication : public Application {
  friend class MemberEpcS1apSapMme<EpcMmeApplication>;

public:
  static TypeId GetTypeId();
  void DoDispose() override;

  EpcMmeApplication();

  ~EpcMmeApplication() override;

  EpcS1apSapMme *GetS1apSapMme();

  void AddSgw(Ipv4Address sgwS11Addr, Ipv4Address mmeS11Addr,
              Ptr<Socket> mmeS11Socket);

  void AddEnb(uint16_t ecgi, Ipv4Address enbS1UAddr, EpcS1apSapEnb *enbS1apSap);

  void AddUe(uint64_t imsi);

  uint8_t AddBearer(uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer);

private:
  void DoInitialUeMessage(uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t imsi,
                          uint16_t ecgi);

  void DoInitialContextSetupResponse(
      uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
      std::list<EpcS1apSapMme::ErabSetupItem> erabSetupList);

  void DoPathSwitchRequest(uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi,
                           std::list<EpcS1apSapMme::ErabSwitchedInDownlinkItem>
                               erabToBeSwitchedInDownlinkList);

  void
  DoErabReleaseIndication(uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
                          std::list<EpcS1apSapMme::ErabToBeReleasedIndication>
                              erabToBeReleaseIndication);

  void RecvFromS11Socket(Ptr<Socket> socket);

  void DoRecvCreateSessionResponse(GtpcHeader &header, Ptr<Packet> packet);

  void DoRecvModifyBearerResponse(GtpcHeader &header, Ptr<Packet> packet);

  void DoRecvDeleteBearerRequest(GtpcHeader &header, Ptr<Packet> packet);

  struct BearerInfo {
    Ptr<EpcTft> tft;
    EpsBearer bearer;
    uint8_t bearerId;
  };

  struct UeInfo : public SimpleRefCount<UeInfo> {
    uint64_t imsi;
    uint64_t mmeUeS1Id;
    uint16_t enbUeS1Id;
    uint16_t cellId;
    uint16_t bearerCounter;
    std::list<BearerInfo> bearersToBeActivated;
  };

  std::map<uint64_t, Ptr<UeInfo>> m_ueInfoMap;

  void RemoveBearer(Ptr<UeInfo> ueInfo, uint8_t epsBearerId);

  struct EnbInfo : public SimpleRefCount<EnbInfo> {
    uint16_t gci;
    Ipv4Address s1uAddr;
    EpcS1apSapEnb *s1apSapEnb;
  };

  std::map<uint16_t, Ptr<EnbInfo>> m_enbInfoMap;

  EpcS1apSapMme *m_s1apSapMme;

  Ptr<Socket> m_s11Socket;
  Ipv4Address m_mmeS11Addr;
  Ipv4Address m_sgwS11Addr;
  uint16_t m_gtpcUdpPort;
};

} // namespace ns3

#endif
