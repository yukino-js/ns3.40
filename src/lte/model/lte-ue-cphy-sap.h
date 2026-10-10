
#ifndef LTE_UE_CPHY_SAP_H
#define LTE_UE_CPHY_SAP_H

#include "lte-rrc-sap.h"

#include <ns3/ptr.h>

#include <stdint.h>

namespace ns3 {

class LteEnbNetDevice;

class LteUeCphySapProvider {
public:
  virtual ~LteUeCphySapProvider();

  virtual void Reset() = 0;

  virtual void StartCellSearch(uint32_t dlEarfcn) = 0;

  virtual void SynchronizeWithEnb(uint16_t cellId) = 0;

  virtual void SynchronizeWithEnb(uint16_t cellId, uint32_t dlEarfcn) = 0;

  virtual uint16_t GetCellId() = 0;

  virtual uint32_t GetDlEarfcn() = 0;

  virtual void SetDlBandwidth(uint16_t dlBandwidth) = 0;

  virtual void ConfigureUplink(uint32_t ulEarfcn, uint16_t ulBandwidth) = 0;

  virtual void ConfigureReferenceSignalPower(int8_t referenceSignalPower) = 0;

  virtual void SetRnti(uint16_t rnti) = 0;

  virtual void SetTransmissionMode(uint8_t txMode) = 0;

  virtual void SetSrsConfigurationIndex(uint16_t srcCi) = 0;

  virtual void SetPa(double pa) = 0;

  virtual void SetRsrpFilterCoefficient(uint8_t rsrpFilterCoefficient) = 0;

  virtual void ResetPhyAfterRlf() = 0;

  virtual void ResetRlfParams() = 0;

  virtual void StartInSyncDetection() = 0;

  virtual void SetImsi(uint64_t imsi) = 0;
};

class LteUeCphySapUser {
public:
  virtual ~LteUeCphySapUser();

  struct UeMeasurementsElement {
    uint16_t m_cellId;
    double m_rsrp;
    double m_rsrq;
  };

  struct UeMeasurementsParameters {
    std::vector<UeMeasurementsElement> m_ueMeasurementsList;
    uint8_t m_componentCarrierId;
  };

  virtual void
  RecvMasterInformationBlock(uint16_t cellId,
                             LteRrcSap::MasterInformationBlock mib) = 0;

  virtual void RecvSystemInformationBlockType1(
      uint16_t cellId, LteRrcSap::SystemInformationBlockType1 sib1) = 0;

  virtual void ReportUeMeasurements(UeMeasurementsParameters params) = 0;

  virtual void NotifyOutOfSync() = 0;

  virtual void NotifyInSync() = 0;

  virtual void ResetSyncIndicationCounter() = 0;
};

template <class C>
class MemberLteUeCphySapProvider : public LteUeCphySapProvider {
public:
  MemberLteUeCphySapProvider(C *owner);

  MemberLteUeCphySapProvider() = delete;

  void Reset() override;
  void StartCellSearch(uint32_t dlEarfcn) override;
  void SynchronizeWithEnb(uint16_t cellId) override;
  void SynchronizeWithEnb(uint16_t cellId, uint32_t dlEarfcn) override;
  uint16_t GetCellId() override;
  uint32_t GetDlEarfcn() override;
  void SetDlBandwidth(uint16_t dlBandwidth) override;
  void ConfigureUplink(uint32_t ulEarfcn, uint16_t ulBandwidth) override;
  void ConfigureReferenceSignalPower(int8_t referenceSignalPower) override;
  void SetRnti(uint16_t rnti) override;
  void SetTransmissionMode(uint8_t txMode) override;
  void SetSrsConfigurationIndex(uint16_t srcCi) override;
  void SetPa(double pa) override;
  void SetRsrpFilterCoefficient(uint8_t rsrpFilterCoefficient) override;
  void ResetPhyAfterRlf() override;
  void ResetRlfParams() override;
  void StartInSyncDetection() override;
  void SetImsi(uint64_t imsi) override;

private:
  C *m_owner;
};

template <class C>
MemberLteUeCphySapProvider<C>::MemberLteUeCphySapProvider(C *owner)
    : m_owner(owner) {}

template <class C> void MemberLteUeCphySapProvider<C>::Reset() {
  m_owner->DoReset();
}

template <class C>
void MemberLteUeCphySapProvider<C>::StartCellSearch(uint32_t dlEarfcn) {
  m_owner->DoStartCellSearch(dlEarfcn);
}

template <class C>
void MemberLteUeCphySapProvider<C>::SynchronizeWithEnb(uint16_t cellId) {
  m_owner->DoSynchronizeWithEnb(cellId);
}

template <class C>
void MemberLteUeCphySapProvider<C>::SynchronizeWithEnb(uint16_t cellId,
                                                       uint32_t dlEarfcn) {
  m_owner->DoSynchronizeWithEnb(cellId, dlEarfcn);
}

template <class C> uint16_t MemberLteUeCphySapProvider<C>::GetCellId() {
  return m_owner->DoGetCellId();
}

template <class C> uint32_t MemberLteUeCphySapProvider<C>::GetDlEarfcn() {
  return m_owner->DoGetDlEarfcn();
}

template <class C>
void MemberLteUeCphySapProvider<C>::SetDlBandwidth(uint16_t dlBandwidth) {
  m_owner->DoSetDlBandwidth(dlBandwidth);
}

template <class C>
void MemberLteUeCphySapProvider<C>::ConfigureUplink(uint32_t ulEarfcn,
                                                    uint16_t ulBandwidth) {
  m_owner->DoConfigureUplink(ulEarfcn, ulBandwidth);
}

template <class C>
void MemberLteUeCphySapProvider<C>::ConfigureReferenceSignalPower(
    int8_t referenceSignalPower) {
  m_owner->DoConfigureReferenceSignalPower(referenceSignalPower);
}

template <class C> void MemberLteUeCphySapProvider<C>::SetRnti(uint16_t rnti) {
  m_owner->DoSetRnti(rnti);
}

template <class C>
void MemberLteUeCphySapProvider<C>::SetTransmissionMode(uint8_t txMode) {
  m_owner->DoSetTransmissionMode(txMode);
}

template <class C>
void MemberLteUeCphySapProvider<C>::SetSrsConfigurationIndex(uint16_t srcCi) {
  m_owner->DoSetSrsConfigurationIndex(srcCi);
}

template <class C> void MemberLteUeCphySapProvider<C>::SetPa(double pa) {
  m_owner->DoSetPa(pa);
}

template <class C>
void MemberLteUeCphySapProvider<C>::SetRsrpFilterCoefficient(
    uint8_t rsrpFilterCoefficient) {
  m_owner->DoSetRsrpFilterCoefficient(rsrpFilterCoefficient);
}

template <class C> void MemberLteUeCphySapProvider<C>::ResetPhyAfterRlf() {
  m_owner->DoResetPhyAfterRlf();
}

template <class C> void MemberLteUeCphySapProvider<C>::ResetRlfParams() {
  m_owner->DoResetRlfParams();
}

template <class C> void MemberLteUeCphySapProvider<C>::StartInSyncDetection() {
  m_owner->DoStartInSyncDetection();
}

template <class C> void MemberLteUeCphySapProvider<C>::SetImsi(uint64_t imsi) {
  m_owner->DoSetImsi(imsi);
}

template <class C> class MemberLteUeCphySapUser : public LteUeCphySapUser {
public:
  MemberLteUeCphySapUser(C *owner);

  MemberLteUeCphySapUser() = delete;

  void
  RecvMasterInformationBlock(uint16_t cellId,
                             LteRrcSap::MasterInformationBlock mib) override;
  void RecvSystemInformationBlockType1(
      uint16_t cellId, LteRrcSap::SystemInformationBlockType1 sib1) override;
  void ReportUeMeasurements(
      LteUeCphySapUser::UeMeasurementsParameters params) override;
  void NotifyOutOfSync() override;
  void NotifyInSync() override;
  void ResetSyncIndicationCounter() override;

private:
  C *m_owner;
};

template <class C>
MemberLteUeCphySapUser<C>::MemberLteUeCphySapUser(C *owner) : m_owner(owner) {}

template <class C>
void MemberLteUeCphySapUser<C>::RecvMasterInformationBlock(
    uint16_t cellId, LteRrcSap::MasterInformationBlock mib) {
  m_owner->DoRecvMasterInformationBlock(cellId, mib);
}

template <class C>
void MemberLteUeCphySapUser<C>::RecvSystemInformationBlockType1(
    uint16_t cellId, LteRrcSap::SystemInformationBlockType1 sib1) {
  m_owner->DoRecvSystemInformationBlockType1(cellId, sib1);
}

template <class C>
void MemberLteUeCphySapUser<C>::ReportUeMeasurements(
    LteUeCphySapUser::UeMeasurementsParameters params) {
  m_owner->DoReportUeMeasurements(params);
}

template <class C> void MemberLteUeCphySapUser<C>::NotifyOutOfSync() {
  m_owner->DoNotifyOutOfSync();
}

template <class C> void MemberLteUeCphySapUser<C>::NotifyInSync() {
  m_owner->DoNotifyInSync();
}

template <class C>
void MemberLteUeCphySapUser<C>::ResetSyncIndicationCounter() {
  m_owner->DoResetSyncIndicationCounter();
}

} // namespace ns3

#endif
