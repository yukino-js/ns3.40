
#ifndef LTE_FFR_RRC_SAP_H
#define LTE_FFR_RRC_SAP_H

#include "epc-x2-sap.h"
#include "lte-rrc-sap.h"

namespace ns3 {

class LteFfrRrcSapProvider {
public:
  virtual ~LteFfrRrcSapProvider();

  virtual void SetCellId(uint16_t cellId) = 0;

  virtual void SetBandwidth(uint8_t ulBandwidth, uint8_t dlBandwidth) = 0;

  virtual void ReportUeMeas(uint16_t rnti,
                            LteRrcSap::MeasResults measResults) = 0;

  virtual void RecvLoadInformation(EpcX2Sap::LoadInformationParams params) = 0;
};

class LteFfrRrcSapUser {
public:
  virtual ~LteFfrRrcSapUser();

  virtual uint8_t
  AddUeMeasReportConfigForFfr(LteRrcSap::ReportConfigEutra reportConfig) = 0;

  virtual void SetPdschConfigDedicated(
      uint16_t rnti, LteRrcSap::PdschConfigDedicated pdschConfigDedicated) = 0;

  virtual void SendLoadInformation(EpcX2Sap::LoadInformationParams params) = 0;
};

template <class C>
class MemberLteFfrRrcSapProvider : public LteFfrRrcSapProvider {
public:
  MemberLteFfrRrcSapProvider(C *owner);

  MemberLteFfrRrcSapProvider() = delete;

  void SetCellId(uint16_t cellId) override;
  void SetBandwidth(uint8_t ulBandwidth, uint8_t dlBandwidth) override;
  void ReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults) override;
  void RecvLoadInformation(EpcX2Sap::LoadInformationParams params) override;

private:
  C *m_owner;
};

template <class C>
MemberLteFfrRrcSapProvider<C>::MemberLteFfrRrcSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteFfrRrcSapProvider<C>::SetCellId(uint16_t cellId) {
  m_owner->DoSetCellId(cellId);
}

template <class C>
void MemberLteFfrRrcSapProvider<C>::SetBandwidth(uint8_t ulBandwidth,
                                                 uint8_t dlBandwidth) {
  m_owner->DoSetBandwidth(ulBandwidth, dlBandwidth);
}

template <class C>
void MemberLteFfrRrcSapProvider<C>::ReportUeMeas(
    uint16_t rnti, LteRrcSap::MeasResults measResults) {
  m_owner->DoReportUeMeas(rnti, measResults);
}

template <class C>
void MemberLteFfrRrcSapProvider<C>::RecvLoadInformation(
    EpcX2Sap::LoadInformationParams params) {
  m_owner->DoRecvLoadInformation(params);
}

template <class C> class MemberLteFfrRrcSapUser : public LteFfrRrcSapUser {
public:
  MemberLteFfrRrcSapUser(C *owner);

  MemberLteFfrRrcSapUser() = delete;

  uint8_t AddUeMeasReportConfigForFfr(
      LteRrcSap::ReportConfigEutra reportConfig) override;

  void SetPdschConfigDedicated(
      uint16_t rnti,
      LteRrcSap::PdschConfigDedicated pdschConfigDedicated) override;

  void SendLoadInformation(EpcX2Sap::LoadInformationParams params) override;

private:
  C *m_owner;
};

template <class C>
MemberLteFfrRrcSapUser<C>::MemberLteFfrRrcSapUser(C *owner) : m_owner(owner) {}

template <class C>
uint8_t MemberLteFfrRrcSapUser<C>::AddUeMeasReportConfigForFfr(
    LteRrcSap::ReportConfigEutra reportConfig) {
  return m_owner->DoAddUeMeasReportConfigForFfr(reportConfig);
}

template <class C>
void MemberLteFfrRrcSapUser<C>::SetPdschConfigDedicated(
    uint16_t rnti, LteRrcSap::PdschConfigDedicated pdschConfigDedicated) {
  m_owner->DoSetPdschConfigDedicated(rnti, pdschConfigDedicated);
}

template <class C>
void MemberLteFfrRrcSapUser<C>::SendLoadInformation(
    EpcX2Sap::LoadInformationParams params) {
  m_owner->DoSendLoadInformation(params);
}

} // namespace ns3

#endif
