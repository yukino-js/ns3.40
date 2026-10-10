
#ifndef LTE_CCM_MAC_SAP_H
#define LTE_CCM_MAC_SAP_H

#include "ff-mac-common.h"
#include "lte-mac-sap.h"

namespace ns3 {
class LteCcmMacSapProvider {
public:
  virtual ~LteCcmMacSapProvider();

  virtual void ReportMacCeToScheduler(MacCeListElement_s bsr) = 0;

  virtual void ReportSrToScheduler(uint16_t rnti) = 0;
};

class LteCcmMacSapUser : public LteMacSapUser {
public:
  ~LteCcmMacSapUser() override;
  virtual void UlReceiveMacCe(MacCeListElement_s bsr,
                              uint8_t componentCarrierId) = 0;

  virtual void UlReceiveSr(uint16_t rnti, uint8_t componentCarrierId) = 0;

  virtual void NotifyPrbOccupancy(double prbOccupancy,
                                  uint8_t componentCarrierId) = 0;
};

template <class C>
class MemberLteCcmMacSapProvider : public LteCcmMacSapProvider {
public:
  MemberLteCcmMacSapProvider(C *owner);
  void ReportMacCeToScheduler(MacCeListElement_s bsr) override;
  void ReportSrToScheduler(uint16_t rnti) override;

private:
  C *m_owner;
};

template <class C>
MemberLteCcmMacSapProvider<C>::MemberLteCcmMacSapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberLteCcmMacSapProvider<C>::ReportMacCeToScheduler(
    MacCeListElement_s bsr) {
  m_owner->DoReportMacCeToScheduler(bsr);
}

template <class C>
void MemberLteCcmMacSapProvider<C>::ReportSrToScheduler(uint16_t rnti) {
  m_owner->DoReportSrToScheduler(rnti);
}

template <class C> class MemberLteCcmMacSapUser : public LteCcmMacSapUser {
public:
  MemberLteCcmMacSapUser(C *owner);
  void UlReceiveMacCe(MacCeListElement_s bsr,
                      uint8_t componentCarrierId) override;
  void UlReceiveSr(uint16_t rnti, uint8_t componentCarrierId) override;
  void NotifyPrbOccupancy(double prbOccupancy,
                          uint8_t componentCarrierId) override;
  void NotifyTxOpportunity(
      LteMacSapUser::TxOpportunityParameters txOpParams) override;
  void ReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams) override;
  void NotifyHarqDeliveryFailure() override;

private:
  C *m_owner;
};

template <class C>
MemberLteCcmMacSapUser<C>::MemberLteCcmMacSapUser(C *owner) : m_owner(owner) {}

template <class C>
void MemberLteCcmMacSapUser<C>::UlReceiveMacCe(MacCeListElement_s bsr,
                                               uint8_t componentCarrierId) {
  m_owner->DoUlReceiveMacCe(bsr, componentCarrierId);
}

template <class C>
void MemberLteCcmMacSapUser<C>::UlReceiveSr(uint16_t rnti,
                                            uint8_t componentCarrierId) {
  m_owner->DoUlReceiveSr(rnti, componentCarrierId);
}

template <class C>
void MemberLteCcmMacSapUser<C>::NotifyPrbOccupancy(double prbOccupancy,
                                                   uint8_t componentCarrierId) {
  m_owner->DoNotifyPrbOccupancy(prbOccupancy, componentCarrierId);
}

template <class C>
void MemberLteCcmMacSapUser<C>::NotifyTxOpportunity(
    LteMacSapUser::TxOpportunityParameters txOpParams) {
  m_owner->DoNotifyTxOpportunity(txOpParams);
}

template <class C>
void MemberLteCcmMacSapUser<C>::ReceivePdu(
    LteMacSapUser::ReceivePduParameters rxPduParams) {
  m_owner->DoReceivePdu(rxPduParams);
}

template <class C> void MemberLteCcmMacSapUser<C>::NotifyHarqDeliveryFailure() {
  m_owner->DoNotifyHarqDeliveryFailure();
}

} // namespace ns3

#endif
