
#ifndef LTE_ANR_SAP_H
#define LTE_ANR_SAP_H

#include "lte-rrc-sap.h"

namespace ns3 {

class LteAnrSapProvider {
public:
  virtual ~LteAnrSapProvider();

  virtual void ReportUeMeas(LteRrcSap::MeasResults measResults) = 0;

  virtual void AddNeighbourRelation(uint16_t cellId) = 0;

  virtual bool GetNoRemove(uint16_t cellId) const = 0;

  virtual bool GetNoHo(uint16_t cellId) const = 0;

  virtual bool GetNoX2(uint16_t cellId) const = 0;
};

class LteAnrSapUser {
public:
  virtual ~LteAnrSapUser();

  virtual uint8_t
  AddUeMeasReportConfigForAnr(LteRrcSap::ReportConfigEutra reportConfig) = 0;
};

template <class C> class MemberLteAnrSapProvider : public LteAnrSapProvider {
public:
  MemberLteAnrSapProvider(C *owner);

  MemberLteAnrSapProvider() = delete;

  void ReportUeMeas(LteRrcSap::MeasResults measResults) override;
  void AddNeighbourRelation(uint16_t cellId) override;
  bool GetNoRemove(uint16_t cellId) const override;
  bool GetNoHo(uint16_t cellId) const override;
  bool GetNoX2(uint16_t cellId) const override;

private:
  C *m_owner;
};

template <class C>
MemberLteAnrSapProvider<C>::MemberLteAnrSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteAnrSapProvider<C>::ReportUeMeas(
    LteRrcSap::MeasResults measResults) {
  m_owner->DoReportUeMeas(measResults);
}

template <class C>
void MemberLteAnrSapProvider<C>::AddNeighbourRelation(uint16_t cellId) {
  m_owner->DoAddNeighbourRelation(cellId);
}

template <class C>
bool MemberLteAnrSapProvider<C>::GetNoRemove(uint16_t cellId) const {
  return m_owner->DoGetNoRemove(cellId);
}

template <class C>
bool MemberLteAnrSapProvider<C>::GetNoHo(uint16_t cellId) const {
  return m_owner->DoGetNoHo(cellId);
}

template <class C>
bool MemberLteAnrSapProvider<C>::GetNoX2(uint16_t cellId) const {
  return m_owner->DoGetNoX2(cellId);
}

template <class C> class MemberLteAnrSapUser : public LteAnrSapUser {
public:
  MemberLteAnrSapUser(C *owner);

  MemberLteAnrSapUser() = delete;

  uint8_t AddUeMeasReportConfigForAnr(
      LteRrcSap::ReportConfigEutra reportConfig) override;

private:
  C *m_owner;
};

template <class C>
MemberLteAnrSapUser<C>::MemberLteAnrSapUser(C *owner) : m_owner(owner) {}

template <class C>
uint8_t MemberLteAnrSapUser<C>::AddUeMeasReportConfigForAnr(
    LteRrcSap::ReportConfigEutra reportConfig) {
  return m_owner->DoAddUeMeasReportConfigForAnr(reportConfig);
}

} // namespace ns3

#endif
