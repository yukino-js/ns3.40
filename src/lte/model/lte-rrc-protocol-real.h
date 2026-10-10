
#ifndef LTE_RRC_PROTOCOL_REAL_H
#define LTE_RRC_PROTOCOL_REAL_H

#include "lte-pdcp-sap.h"
#include "lte-rlc-sap.h"
#include "lte-rrc-sap.h"

#include <ns3/object.h>
#include <ns3/ptr.h>

#include <map>
#include <stdint.h>

namespace ns3 {

class LteUeRrcSapProvider;
class LteUeRrcSapUser;
class LteEnbRrcSapProvider;
class LteUeRrc;

class LteUeRrcProtocolReal : public Object {
  friend class MemberLteUeRrcSapUser<LteUeRrcProtocolReal>;
  friend class LteRlcSpecificLteRlcSapUser<LteUeRrcProtocolReal>;
  friend class LtePdcpSpecificLtePdcpSapUser<LteUeRrcProtocolReal>;

public:
  LteUeRrcProtocolReal();
  ~LteUeRrcProtocolReal() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  void SetLteUeRrcSapProvider(LteUeRrcSapProvider *p);
  LteUeRrcSapUser *GetLteUeRrcSapUser();

  void SetUeRrc(Ptr<LteUeRrc> rrc);

private:
  void DoSetup(LteUeRrcSapUser::SetupParameters params);
  void DoSendRrcConnectionRequest(LteRrcSap::RrcConnectionRequest msg);
  void DoSendRrcConnectionSetupCompleted(
      LteRrcSap::RrcConnectionSetupCompleted msg) const;
  void DoSendRrcConnectionReconfigurationCompleted(
      LteRrcSap::RrcConnectionReconfigurationCompleted msg);
  void DoSendRrcConnectionReestablishmentRequest(
      LteRrcSap::RrcConnectionReestablishmentRequest msg) const;
  void DoSendRrcConnectionReestablishmentComplete(
      LteRrcSap::RrcConnectionReestablishmentComplete msg) const;
  void DoSendMeasurementReport(LteRrcSap::MeasurementReport msg);
  void DoSendIdealUeContextRemoveRequest(uint16_t rnti);

  void SetEnbRrcSapProvider();
  void DoReceivePdcpPdu(Ptr<Packet> p);
  void DoReceivePdcpSdu(LtePdcpSapUser::ReceivePdcpSduParameters params);

  Ptr<LteUeRrc> m_rrc;
  uint16_t m_rnti;
  LteUeRrcSapProvider *m_ueRrcSapProvider;
  LteUeRrcSapUser *m_ueRrcSapUser;
  LteEnbRrcSapProvider *m_enbRrcSapProvider;

  LteUeRrcSapUser::SetupParameters m_setupParameters;
  LteUeRrcSapProvider::CompleteSetupParameters m_completeSetupParameters;
};

class LteEnbRrcProtocolReal : public Object {
  friend class MemberLteEnbRrcSapUser<LteEnbRrcProtocolReal>;
  friend class LtePdcpSpecificLtePdcpSapUser<LteEnbRrcProtocolReal>;
  friend class LteRlcSpecificLteRlcSapUser<LteEnbRrcProtocolReal>;
  friend class RealProtocolRlcSapUser;

public:
  LteEnbRrcProtocolReal();
  ~LteEnbRrcProtocolReal() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  void SetLteEnbRrcSapProvider(LteEnbRrcSapProvider *p);
  LteEnbRrcSapUser *GetLteEnbRrcSapUser();

  void SetCellId(uint16_t cellId);

  LteUeRrcSapProvider *GetUeRrcSapProvider(uint16_t rnti);
  void SetUeRrcSapProvider(uint16_t rnti, LteUeRrcSapProvider *p);

private:
  void DoSetupUe(uint16_t rnti, LteEnbRrcSapUser::SetupUeParameters params);
  void DoRemoveUe(uint16_t rnti);
  void DoSendSystemInformation(uint16_t cellId,
                               LteRrcSap::SystemInformation msg);
  void SendSystemInformation(uint16_t cellId, LteRrcSap::SystemInformation msg);
  void DoSendRrcConnectionSetup(uint16_t rnti,
                                LteRrcSap::RrcConnectionSetup msg);
  void DoSendRrcConnectionReconfiguration(
      uint16_t rnti, LteRrcSap::RrcConnectionReconfiguration msg);
  void DoSendRrcConnectionReestablishment(
      uint16_t rnti, LteRrcSap::RrcConnectionReestablishment msg);
  void DoSendRrcConnectionReestablishmentReject(
      uint16_t rnti, LteRrcSap::RrcConnectionReestablishmentReject msg);
  void DoSendRrcConnectionRelease(uint16_t rnti,
                                  LteRrcSap::RrcConnectionRelease msg);
  void DoSendRrcConnectionReject(uint16_t rnti,
                                 LteRrcSap::RrcConnectionReject msg);
  Ptr<Packet> DoEncodeHandoverPreparationInformation(
      LteRrcSap::HandoverPreparationInfo msg);
  LteRrcSap::HandoverPreparationInfo
  DoDecodeHandoverPreparationInformation(Ptr<Packet> p);
  Ptr<Packet>
  DoEncodeHandoverCommand(LteRrcSap::RrcConnectionReconfiguration msg);
  LteRrcSap::RrcConnectionReconfiguration
  DoDecodeHandoverCommand(Ptr<Packet> p);

  void DoReceivePdcpSdu(LtePdcpSapUser::ReceivePdcpSduParameters params);
  void DoReceivePdcpPdu(uint16_t rnti, Ptr<Packet> p);

  uint16_t m_rnti;
  uint16_t m_cellId;
  LteEnbRrcSapProvider *m_enbRrcSapProvider;
  LteEnbRrcSapUser *m_enbRrcSapUser;
  std::map<uint16_t, LteUeRrcSapProvider *> m_enbRrcSapProviderMap;
  std::map<uint16_t, LteEnbRrcSapUser::SetupUeParameters>
      m_setupUeParametersMap;
  std::map<uint16_t, LteEnbRrcSapProvider::CompleteSetupUeParameters>
      m_completeSetupUeParametersMap;
};

class RealProtocolRlcSapUser : public LteRlcSapUser {
public:
  RealProtocolRlcSapUser(LteEnbRrcProtocolReal *pdcp, uint16_t rnti);

  void ReceivePdcpPdu(Ptr<Packet> p) override;

private:
  RealProtocolRlcSapUser();
  LteEnbRrcProtocolReal *m_pdcp;
  uint16_t m_rnti;
};

} // namespace ns3

#endif
