
#ifndef LTE_TEST_ENTITIES_H
#define LTE_TEST_ENTITIES_H

#include "ns3/lte-mac-sap.h"
#include "ns3/lte-pdcp-sap.h"
#include "ns3/lte-rlc-sap.h"
#include "ns3/net-device.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include <ns3/epc-enb-s1-sap.h>

namespace ns3 {

class LteTestRrc : public Object {
  friend class LtePdcpSpecificLtePdcpSapUser<LteTestRrc>;

public:
  static TypeId GetTypeId();

  LteTestRrc();
  ~LteTestRrc() override;
  void DoDispose() override;

  void SetLtePdcpSapProvider(LtePdcpSapProvider *s);
  LtePdcpSapUser *GetLtePdcpSapUser();

  void Start();
  void Stop();

  void SendData(Time at, std::string dataToSend);
  std::string GetDataReceived();

  uint32_t GetTxPdus();
  uint32_t GetTxBytes();
  uint32_t GetRxPdus();
  uint32_t GetRxBytes();

  Time GetTxLastTime();
  Time GetRxLastTime();

  void SetArrivalTime(Time arrivalTime);
  void SetPduSize(uint32_t pduSize);

  void SetDevice(Ptr<NetDevice> device);

private:
  virtual void
  DoReceivePdcpSdu(LtePdcpSapUser::ReceivePdcpSduParameters params);

  LtePdcpSapUser *m_pdcpSapUser;
  LtePdcpSapProvider *m_pdcpSapProvider;

  std::string m_receivedData;

  uint32_t m_txPdus;
  uint32_t m_txBytes;
  uint32_t m_rxPdus;
  uint32_t m_rxBytes;
  Time m_txLastTime;
  Time m_rxLastTime;

  EventId m_nextPdu;
  Time m_arrivalTime;
  uint32_t m_pduSize;

  Ptr<NetDevice> m_device;
};

class LteTestPdcp : public Object {
  friend class LteRlcSpecificLteRlcSapUser<LteTestPdcp>;

public:
  static TypeId GetTypeId();

  LteTestPdcp();
  ~LteTestPdcp() override;
  void DoDispose() override;

  void SetLteRlcSapProvider(LteRlcSapProvider *s);
  LteRlcSapUser *GetLteRlcSapUser();

  void Start();

  void SendData(Time time, std::string dataToSend);
  std::string GetDataReceived();

private:
  virtual void DoReceivePdcpPdu(Ptr<Packet> p);

  LteRlcSapUser *m_rlcSapUser;
  LteRlcSapProvider *m_rlcSapProvider;

  std::string m_receivedData;
};

class LteTestMac : public Object {
  friend class EnbMacMemberLteMacSapProvider<LteTestMac>;

public:
  static TypeId GetTypeId();

  LteTestMac();
  ~LteTestMac() override;
  void DoDispose() override;

  void SetDevice(Ptr<NetDevice> device);

  void SendTxOpportunity(Time time, uint32_t bytes);
  std::string GetDataReceived();

  bool Receive(Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t protocol,
               const Address &addr);

  void SetLteMacSapUser(LteMacSapUser *s);
  LteMacSapProvider *GetLteMacSapProvider();

  void SetLteMacLoopback(Ptr<LteTestMac> s);

  void SetPdcpHeaderPresent(bool present);

  void SetRlcHeaderType(uint8_t rlcHeaderType);

  enum RlcHeaderType_t {
    UM_RLC_HEADER = 0,
    AM_RLC_HEADER = 1,
  };

  void SetTxOpportunityMode(uint8_t mode);

  enum TxOpportunityMode_t {
    MANUAL_MODE = 0,
    AUTOMATIC_MODE = 1,
    RANDOM_MODE = 2
  };

  void SetTxOppTime(Time txOppTime);
  void SetTxOppSize(uint32_t txOppSize);

  uint32_t GetTxPdus();
  uint32_t GetTxBytes();
  uint32_t GetRxPdus();
  uint32_t GetRxBytes();

private:
  void DoTransmitPdu(LteMacSapProvider::TransmitPduParameters params);
  void
  DoReportBufferStatus(LteMacSapProvider::ReportBufferStatusParameters params);

  LteMacSapProvider *m_macSapProvider;
  LteMacSapUser *m_macSapUser;
  Ptr<LteTestMac> m_macLoopback;

  std::string m_receivedData;

  uint8_t m_rlcHeaderType;
  bool m_pdcpHeaderPresent;
  uint8_t m_txOpportunityMode;

  Ptr<NetDevice> m_device;

  EventId m_nextTxOpp;
  Time m_txOppTime;
  uint32_t m_txOppSize;
  std::list<EventId> m_nextTxOppList;

  uint32_t m_txPdus;
  uint32_t m_txBytes;
  uint32_t m_rxPdus;
  uint32_t m_rxBytes;
};

class EpcTestRrc : public Object {
  friend class MemberEpcEnbS1SapUser<EpcTestRrc>;

public:
  EpcTestRrc();
  ~EpcTestRrc() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  void SetS1SapProvider(EpcEnbS1SapProvider *s);

  EpcEnbS1SapUser *GetS1SapUser();

private:
  void DoInitialContextSetupRequest(
      EpcEnbS1SapUser::InitialContextSetupRequestParameters params);
  void DoDataRadioBearerSetupRequest(
      EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params);
  void DoPathSwitchRequestAcknowledge(
      EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters params);

  EpcEnbS1SapProvider *m_s1SapProvider;
  EpcEnbS1SapUser *m_s1SapUser;
};

} // namespace ns3

#endif
