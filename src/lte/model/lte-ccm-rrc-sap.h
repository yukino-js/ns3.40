
#ifndef LTE_CCM_RRC_SAP_H
#define LTE_CCM_RRC_SAP_H

#include "eps-bearer.h"
#include "lte-enb-cmac-sap.h"
#include "lte-mac-sap.h"
#include "lte-rrc-sap.h"

#include <map>

namespace ns3 {
class LteUeCmacSapProvider;
class UeManager;
class LteEnbCmacSapProvider;
class LteMacSapUser;
class LteRrcSap;

class LteCcmRrcSapProvider {
  friend class UeManager;
  friend class LteMacSapUser;

public:
  virtual ~LteCcmRrcSapProvider();

  struct LcsConfig {
    uint16_t componentCarrierId;
    LteEnbCmacSapProvider::LcInfo lc;
    LteMacSapUser *msu;
  };

  virtual void ReportUeMeas(uint16_t rnti,
                            LteRrcSap::MeasResults measResults) = 0;

  virtual void AddUe(uint16_t rnti, uint8_t state) = 0;

  virtual void AddLc(LteEnbCmacSapProvider::LcInfo lcInfo,
                     LteMacSapUser *msu) = 0;

  virtual void RemoveUe(uint16_t rnti) = 0;

  virtual std::vector<LteCcmRrcSapProvider::LcsConfig>
  SetupDataRadioBearer(EpsBearer bearer, uint8_t bearerId, uint16_t rnti,
                       uint8_t lcid, uint8_t lcGroup, LteMacSapUser *msu) = 0;

  virtual std::vector<uint8_t> ReleaseDataRadioBearer(uint16_t rnti,
                                                      uint8_t lcid) = 0;

  virtual LteMacSapUser *
  ConfigureSignalBearer(LteEnbCmacSapProvider::LcInfo lcInfo,
                        LteMacSapUser *rlcMacSapUser) = 0;
};

class LteCcmRrcSapUser {
  friend class LteEnbRrc;

public:
  virtual ~LteCcmRrcSapUser();

  virtual uint8_t AddUeMeasReportConfigForComponentCarrier(
      LteRrcSap::ReportConfigEutra reportConfig) = 0;

  virtual void TriggerComponentCarrier(uint16_t rnti,
                                       uint16_t targetCellId) = 0;

  virtual void
  AddLcs(std::vector<LteEnbRrcSapProvider::LogicalChannelConfig> lcConfig) = 0;

  virtual void ReleaseLcs(uint16_t rnti, uint8_t lcid) = 0;

  virtual Ptr<UeManager> GetUeManager(uint16_t rnti) = 0;

  virtual void SetNumberOfComponentCarriers(uint16_t noOfComponentCarriers) = 0;
};

template <class C>
class MemberLteCcmRrcSapProvider : public LteCcmRrcSapProvider {
public:
  MemberLteCcmRrcSapProvider(C *owner);

  void ReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults) override;
  void AddUe(uint16_t rnti, uint8_t state) override;
  void AddLc(LteEnbCmacSapProvider::LcInfo lcInfo, LteMacSapUser *msu) override;
  void RemoveUe(uint16_t rnti) override;
  std::vector<LteCcmRrcSapProvider::LcsConfig>
  SetupDataRadioBearer(EpsBearer bearer, uint8_t bearerId, uint16_t rnti,
                       uint8_t lcid, uint8_t lcGroup,
                       LteMacSapUser *msu) override;
  std::vector<uint8_t> ReleaseDataRadioBearer(uint16_t rnti,
                                              uint8_t lcid) override;
  LteMacSapUser *ConfigureSignalBearer(LteEnbCmacSapProvider::LcInfo lcInfo,
                                       LteMacSapUser *rlcMacSapUser) override;

private:
  C *m_owner;
};

template <class C>
MemberLteCcmRrcSapProvider<C>::MemberLteCcmRrcSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteCcmRrcSapProvider<C>::ReportUeMeas(
    uint16_t rnti, LteRrcSap::MeasResults measResults) {
  m_owner->DoReportUeMeas(rnti, measResults);
}

template <class C>
void MemberLteCcmRrcSapProvider<C>::AddUe(uint16_t rnti, uint8_t state) {
  m_owner->DoAddUe(rnti, state);
}

template <class C>
void MemberLteCcmRrcSapProvider<C>::AddLc(LteEnbCmacSapProvider::LcInfo lcInfo,
                                          LteMacSapUser *msu) {
  m_owner->DoAddLc(lcInfo, msu);
}

template <class C> void MemberLteCcmRrcSapProvider<C>::RemoveUe(uint16_t rnti) {
  m_owner->DoRemoveUe(rnti);
}

template <class C>
std::vector<LteCcmRrcSapProvider::LcsConfig>
MemberLteCcmRrcSapProvider<C>::SetupDataRadioBearer(EpsBearer bearer,
                                                    uint8_t bearerId,
                                                    uint16_t rnti, uint8_t lcid,
                                                    uint8_t lcGroup,
                                                    LteMacSapUser *msu) {
  return m_owner->DoSetupDataRadioBearer(bearer, bearerId, rnti, lcid, lcGroup,
                                         msu);
}

template <class C>
std::vector<uint8_t>
MemberLteCcmRrcSapProvider<C>::ReleaseDataRadioBearer(uint16_t rnti,
                                                      uint8_t lcid) {
  return m_owner->DoReleaseDataRadioBearer(rnti, lcid);
}

template <class C>
LteMacSapUser *MemberLteCcmRrcSapProvider<C>::ConfigureSignalBearer(
    LteEnbCmacSapProvider::LcInfo lcInfo, LteMacSapUser *rlcMacSapUser) {
  return m_owner->DoConfigureSignalBearer(lcInfo, rlcMacSapUser);
}

template <class C> class MemberLteCcmRrcSapUser : public LteCcmRrcSapUser {
public:
  MemberLteCcmRrcSapUser(C *owner);

  void AddLcs(std::vector<LteEnbRrcSapProvider::LogicalChannelConfig> lcConfig)
      override;
  void ReleaseLcs(uint16_t rnti, uint8_t lcid) override;
  uint8_t AddUeMeasReportConfigForComponentCarrier(
      LteRrcSap::ReportConfigEutra reportConfig) override;
  void TriggerComponentCarrier(uint16_t rnti, uint16_t targetCellId) override;
  Ptr<UeManager> GetUeManager(uint16_t rnti) override;
  void SetNumberOfComponentCarriers(uint16_t noOfComponentCarriers) override;

private:
  C *m_owner;
};

template <class C>
MemberLteCcmRrcSapUser<C>::MemberLteCcmRrcSapUser(C *owner) : m_owner(owner) {}

template <class C>
void MemberLteCcmRrcSapUser<C>::AddLcs(
    std::vector<LteEnbRrcSapProvider::LogicalChannelConfig> lcConfig) {
  NS_FATAL_ERROR(
      "Function should not be called because it is not implemented.");
}

template <class C>
void MemberLteCcmRrcSapUser<C>::ReleaseLcs(uint16_t rnti, uint8_t lcid) {
  NS_FATAL_ERROR(
      "Function should not be called because it is not implemented.");
}

template <class C>
uint8_t MemberLteCcmRrcSapUser<C>::AddUeMeasReportConfigForComponentCarrier(
    LteRrcSap::ReportConfigEutra reportConfig) {
  return m_owner->DoAddUeMeasReportConfigForComponentCarrier(reportConfig);
}

template <class C>
void MemberLteCcmRrcSapUser<C>::TriggerComponentCarrier(uint16_t rnti,
                                                        uint16_t targetCellId) {
  NS_FATAL_ERROR(
      "Function should not be called because it is not implemented.");
}

template <class C>
Ptr<UeManager> MemberLteCcmRrcSapUser<C>::GetUeManager(uint16_t rnti) {
  return m_owner->GetUeManager(rnti);
}

template <class C>
void MemberLteCcmRrcSapUser<C>::SetNumberOfComponentCarriers(
    uint16_t noOfComponentCarriers) {
  return m_owner->DoSetNumberOfComponentCarriers(noOfComponentCarriers);
}

} // namespace ns3

#endif
