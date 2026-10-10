
#ifndef LTE_HANDOVER_MANAGEMENT_SAP_H
#define LTE_HANDOVER_MANAGEMENT_SAP_H

#include "lte-rrc-sap.h"

namespace ns3 {

class LteHandoverManagementSapProvider {
public:
  virtual ~LteHandoverManagementSapProvider();

  virtual void ReportUeMeas(uint16_t rnti,
                            LteRrcSap::MeasResults measResults) = 0;
};

class LteHandoverManagementSapUser {
public:
  virtual ~LteHandoverManagementSapUser();

  virtual std::vector<uint8_t> AddUeMeasReportConfigForHandover(
      LteRrcSap::ReportConfigEutra reportConfig) = 0;

  virtual void TriggerHandover(uint16_t rnti, uint16_t targetCellId) = 0;
};

template <class C>
class MemberLteHandoverManagementSapProvider
    : public LteHandoverManagementSapProvider {
public:
  MemberLteHandoverManagementSapProvider(C *owner);

  MemberLteHandoverManagementSapProvider() = delete;

  void ReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults) override;

private:
  C *m_owner;
};

template <class C>
MemberLteHandoverManagementSapProvider<
    C>::MemberLteHandoverManagementSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteHandoverManagementSapProvider<C>::ReportUeMeas(
    uint16_t rnti, LteRrcSap::MeasResults measResults) {
  m_owner->DoReportUeMeas(rnti, measResults);
}

template <class C>
class MemberLteHandoverManagementSapUser : public LteHandoverManagementSapUser {
public:
  MemberLteHandoverManagementSapUser(C *owner);

  MemberLteHandoverManagementSapUser() = delete;

  std::vector<uint8_t> AddUeMeasReportConfigForHandover(
      LteRrcSap::ReportConfigEutra reportConfig) override;
  void TriggerHandover(uint16_t rnti, uint16_t targetCellId) override;

private:
  C *m_owner;
};

template <class C>
MemberLteHandoverManagementSapUser<C>::MemberLteHandoverManagementSapUser(
    C *owner)
    : m_owner(owner) {}

template <class C>
std::vector<uint8_t>
MemberLteHandoverManagementSapUser<C>::AddUeMeasReportConfigForHandover(
    LteRrcSap::ReportConfigEutra reportConfig) {
  return m_owner->DoAddUeMeasReportConfigForHandover(reportConfig);
}

template <class C>
void MemberLteHandoverManagementSapUser<C>::TriggerHandover(
    uint16_t rnti, uint16_t targetCellId) {
  return m_owner->DoTriggerHandover(rnti, targetCellId);
}

} // namespace ns3

#endif
