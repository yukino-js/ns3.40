
#ifndef NO_OP_COMPONENT_CARRIER_MANAGER_H
#define NO_OP_COMPONENT_CARRIER_MANAGER_H

#include "lte-ccm-rrc-sap.h"
#include "lte-enb-component-carrier-manager.h"
#include "lte-rrc-sap.h"

#include <map>

namespace ns3 {

class UeManager;
class LteCcmRrcSapProvider;

class NoOpComponentCarrierManager : public LteEnbComponentCarrierManager {
  friend class EnbMacMemberLteMacSapProvider<NoOpComponentCarrierManager>;
  friend class MemberLteCcmRrcSapProvider<NoOpComponentCarrierManager>;
  friend class MemberLteCcmRrcSapUser<NoOpComponentCarrierManager>;
  friend class MemberLteCcmMacSapUser<NoOpComponentCarrierManager>;

public:
  NoOpComponentCarrierManager();
  ~NoOpComponentCarrierManager() override;
  static TypeId GetTypeId();

protected:
  void DoInitialize() override;
  void DoDispose() override;
  void DoReportUeMeas(uint16_t rnti,
                      LteRrcSap::MeasResults measResults) override;
  virtual void DoAddUe(uint16_t rnti, uint8_t state);
  virtual void DoAddLc(LteEnbCmacSapProvider::LcInfo lcInfo,
                       LteMacSapUser *msu);
  virtual std::vector<LteCcmRrcSapProvider::LcsConfig>
  DoSetupDataRadioBearer(EpsBearer bearer, uint8_t bearerId, uint16_t rnti,
                         uint8_t lcid, uint8_t lcGroup, LteMacSapUser *msu);
  virtual void DoTransmitPdu(LteMacSapProvider::TransmitPduParameters params);
  virtual void
  DoReportBufferStatus(LteMacSapProvider::ReportBufferStatusParameters params);
  virtual void
  DoNotifyTxOpportunity(LteMacSapUser::TxOpportunityParameters txOpParams);
  virtual void DoReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams);
  virtual void DoNotifyHarqDeliveryFailure();
  virtual void DoRemoveUe(uint16_t rnti);
  virtual std::vector<uint8_t> DoReleaseDataRadioBearer(uint16_t rnti,
                                                        uint8_t lcid);
  virtual LteMacSapUser *
  DoConfigureSignalBearer(LteEnbCmacSapProvider::LcInfo lcinfo,
                          LteMacSapUser *msu);
  virtual void DoUlReceiveMacCe(MacCeListElement_s bsr,
                                uint8_t componentCarrierId);
  virtual void DoUlReceiveSr(uint16_t rnti, uint8_t componentCarrierId);
  virtual void DoNotifyPrbOccupancy(double prbOccupancy,
                                    uint8_t componentCarrierId);

protected:
  std::map<uint8_t, double> m_ccPrbOccupancy;
};

class RrComponentCarrierManager : public NoOpComponentCarrierManager {
public:
  RrComponentCarrierManager();
  ~RrComponentCarrierManager() override;
  static TypeId GetTypeId();

protected:
  void DoReportBufferStatus(
      LteMacSapProvider::ReportBufferStatusParameters params) override;
  void DoUlReceiveMacCe(MacCeListElement_s bsr,
                        uint8_t componentCarrierId) override;
  void DoUlReceiveSr(uint16_t rnti, uint8_t componentCarrierId) override;

private:
  uint8_t m_lastCcIdForSr{0};
};

} // namespace ns3

#endif
