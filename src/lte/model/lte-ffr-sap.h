
#ifndef LTE_FFR_SAP_H
#define LTE_FFR_SAP_H

#include "ff-mac-sched-sap.h"

#include <map>

namespace ns3 {

class LteFfrSapProvider {
public:
  virtual ~LteFfrSapProvider();

  virtual std::vector<bool> GetAvailableDlRbg() = 0;

  virtual bool IsDlRbgAvailableForUe(int i, uint16_t rnti) = 0;

  virtual std::vector<bool> GetAvailableUlRbg() = 0;

  virtual bool IsUlRbgAvailableForUe(int i, uint16_t rnti) = 0;

  virtual void ReportDlCqiInfo(
      const FfMacSchedSapProvider::SchedDlCqiInfoReqParameters &params) = 0;

  virtual void ReportUlCqiInfo(
      const FfMacSchedSapProvider::SchedUlCqiInfoReqParameters &params) = 0;

  virtual void
  ReportUlCqiInfo(std::map<uint16_t, std::vector<double>> ulCqiMap) = 0;

  virtual uint8_t GetTpc(uint16_t rnti) = 0;

  virtual uint16_t GetMinContinuousUlBandwidth() = 0;
};

class LteFfrSapUser {
public:
  virtual ~LteFfrSapUser();
};

template <class C> class MemberLteFfrSapProvider : public LteFfrSapProvider {
public:
  MemberLteFfrSapProvider(C *owner);

  MemberLteFfrSapProvider() = delete;

  std::vector<bool> GetAvailableDlRbg() override;
  bool IsDlRbgAvailableForUe(int i, uint16_t rnti) override;
  std::vector<bool> GetAvailableUlRbg() override;
  bool IsUlRbgAvailableForUe(int i, uint16_t rnti) override;
  void ReportDlCqiInfo(const FfMacSchedSapProvider::SchedDlCqiInfoReqParameters
                           &params) override;
  void ReportUlCqiInfo(const FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
                           &params) override;
  void
  ReportUlCqiInfo(std::map<uint16_t, std::vector<double>> ulCqiMap) override;
  uint8_t GetTpc(uint16_t rnti) override;
  uint16_t GetMinContinuousUlBandwidth() override;

private:
  C *m_owner;
};

template <class C>
MemberLteFfrSapProvider<C>::MemberLteFfrSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
std::vector<bool> MemberLteFfrSapProvider<C>::GetAvailableDlRbg() {
  return m_owner->DoGetAvailableDlRbg();
}

template <class C>
bool MemberLteFfrSapProvider<C>::IsDlRbgAvailableForUe(int i, uint16_t rnti) {
  return m_owner->DoIsDlRbgAvailableForUe(i, rnti);
}

template <class C>
std::vector<bool> MemberLteFfrSapProvider<C>::GetAvailableUlRbg() {
  return m_owner->DoGetAvailableUlRbg();
}

template <class C>
bool MemberLteFfrSapProvider<C>::IsUlRbgAvailableForUe(int i, uint16_t rnti) {
  return m_owner->DoIsUlRbgAvailableForUe(i, rnti);
}

template <class C>
void MemberLteFfrSapProvider<C>::ReportDlCqiInfo(
    const FfMacSchedSapProvider::SchedDlCqiInfoReqParameters &params) {
  m_owner->DoReportDlCqiInfo(params);
}

template <class C>
void MemberLteFfrSapProvider<C>::ReportUlCqiInfo(
    const FfMacSchedSapProvider::SchedUlCqiInfoReqParameters &params) {
  m_owner->DoReportUlCqiInfo(params);
}

template <class C>
void MemberLteFfrSapProvider<C>::ReportUlCqiInfo(
    std::map<uint16_t, std::vector<double>> ulCqiMap) {
  m_owner->DoReportUlCqiInfo(ulCqiMap);
}

template <class C> uint8_t MemberLteFfrSapProvider<C>::GetTpc(uint16_t rnti) {
  return m_owner->DoGetTpc(rnti);
}

template <class C>
uint16_t MemberLteFfrSapProvider<C>::GetMinContinuousUlBandwidth() {
  return m_owner->DoGetMinContinuousUlBandwidth();
}

template <class C> class MemberLteFfrSapUser : public LteFfrSapUser {
public:
  MemberLteFfrSapUser(C *owner);

  MemberLteFfrSapUser() = delete;

private:
  C *m_owner;
};

template <class C>
MemberLteFfrSapUser<C>::MemberLteFfrSapUser(C *owner) : m_owner(owner) {}

} // namespace ns3

#endif
