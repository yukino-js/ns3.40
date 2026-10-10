
#ifndef LTE_ENB_CPHY_SAP_H
#define LTE_ENB_CPHY_SAP_H

#include "lte-rrc-sap.h"

#include <ns3/ptr.h>

#include <stdint.h>

namespace ns3 {

class LteEnbNetDevice;

class LteEnbCphySapProvider {
public:
  virtual ~LteEnbCphySapProvider();

  virtual void SetCellId(uint16_t cellId) = 0;

  virtual void SetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth) = 0;

  virtual void SetEarfcn(uint32_t ulEarfcn, uint32_t dlEarfcn) = 0;

  virtual void AddUe(uint16_t rnti) = 0;

  virtual void RemoveUe(uint16_t rnti) = 0;

  virtual void SetPa(uint16_t rnti, double pa) = 0;

  virtual void SetTransmissionMode(uint16_t rnti, uint8_t txMode) = 0;

  virtual void SetSrsConfigurationIndex(uint16_t rnti, uint16_t srsCi) = 0;

  virtual void
  SetMasterInformationBlock(LteRrcSap::MasterInformationBlock mib) = 0;

  virtual void SetSystemInformationBlockType1(
      LteRrcSap::SystemInformationBlockType1 sib1) = 0;

  virtual int8_t GetReferenceSignalPower() = 0;
};

class LteEnbCphySapUser {
public:
  virtual ~LteEnbCphySapUser();
};

template <class C>
class MemberLteEnbCphySapProvider : public LteEnbCphySapProvider {
public:
  MemberLteEnbCphySapProvider(C *owner);

  MemberLteEnbCphySapProvider() = delete;

  void SetCellId(uint16_t cellId) override;
  void SetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth) override;
  void SetEarfcn(uint32_t ulEarfcn, uint32_t dlEarfcn) override;
  void AddUe(uint16_t rnti) override;
  void RemoveUe(uint16_t rnti) override;
  void SetPa(uint16_t rnti, double pa) override;
  void SetTransmissionMode(uint16_t rnti, uint8_t txMode) override;
  void SetSrsConfigurationIndex(uint16_t rnti, uint16_t srsCi) override;
  void
  SetMasterInformationBlock(LteRrcSap::MasterInformationBlock mib) override;
  void SetSystemInformationBlockType1(
      LteRrcSap::SystemInformationBlockType1 sib1) override;
  int8_t GetReferenceSignalPower() override;

private:
  C *m_owner;
};

template <class C>
MemberLteEnbCphySapProvider<C>::MemberLteEnbCphySapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetCellId(uint16_t cellId) {
  m_owner->DoSetCellId(cellId);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetBandwidth(uint16_t ulBandwidth,
                                                  uint16_t dlBandwidth) {
  m_owner->DoSetBandwidth(ulBandwidth, dlBandwidth);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetEarfcn(uint32_t ulEarfcn,
                                               uint32_t dlEarfcn) {
  m_owner->DoSetEarfcn(ulEarfcn, dlEarfcn);
}

template <class C> void MemberLteEnbCphySapProvider<C>::AddUe(uint16_t rnti) {
  m_owner->DoAddUe(rnti);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::RemoveUe(uint16_t rnti) {
  m_owner->DoRemoveUe(rnti);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetPa(uint16_t rnti, double pa) {
  m_owner->DoSetPa(rnti, pa);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetTransmissionMode(uint16_t rnti,
                                                         uint8_t txMode) {
  m_owner->DoSetTransmissionMode(rnti, txMode);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetSrsConfigurationIndex(uint16_t rnti,
                                                              uint16_t srsCi) {
  m_owner->DoSetSrsConfigurationIndex(rnti, srsCi);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetMasterInformationBlock(
    LteRrcSap::MasterInformationBlock mib) {
  m_owner->DoSetMasterInformationBlock(mib);
}

template <class C>
void MemberLteEnbCphySapProvider<C>::SetSystemInformationBlockType1(
    LteRrcSap::SystemInformationBlockType1 sib1) {
  m_owner->DoSetSystemInformationBlockType1(sib1);
}

template <class C>
int8_t MemberLteEnbCphySapProvider<C>::GetReferenceSignalPower() {
  return m_owner->DoGetReferenceSignalPower();
}

template <class C> class MemberLteEnbCphySapUser : public LteEnbCphySapUser {
public:
  MemberLteEnbCphySapUser(C *owner);

  MemberLteEnbCphySapUser() = delete;

private:
  C *m_owner;
};

template <class C>
MemberLteEnbCphySapUser<C>::MemberLteEnbCphySapUser(C *owner)
    : m_owner(owner) {}

} // namespace ns3

#endif
