
#ifndef SIMPLE_UE_COMPONENT_CARRIER_MANAGER_H
#define SIMPLE_UE_COMPONENT_CARRIER_MANAGER_H

#include "lte-rrc-sap.h"
#include "lte-ue-ccm-rrc-sap.h"
#include "lte-ue-component-carrier-manager.h"

#include <map>

namespace ns3 {
class LteUeCcmRrcSapProvider;

class SimpleUeComponentCarrierManager : public LteUeComponentCarrierManager {
public:
  SimpleUeComponentCarrierManager();

  ~SimpleUeComponentCarrierManager() override;

  static TypeId GetTypeId();

  LteMacSapProvider *GetLteMacSapProvider() override;

  friend class MemberLteUeCcmRrcSapProvider<SimpleUeComponentCarrierManager>;

  friend class SimpleUeCcmMacSapProvider;
  friend class SimpleUeCcmMacSapUser;

protected:
  void DoInitialize() override;
  void DoDispose() override;
  void DoReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults);
  void DoTransmitPdu(LteMacSapProvider::TransmitPduParameters params);
  virtual void
  DoReportBufferStatus(LteMacSapProvider::ReportBufferStatusParameters params);
  void DoNotifyHarqDeliveryFailure();
  void DoNotifyTxOpportunity(LteMacSapUser::TxOpportunityParameters txOpParams);
  void DoReceivePdu(LteMacSapUser::ReceivePduParameters rxPduParams);
  virtual std::vector<LteUeCcmRrcSapProvider::LcsConfig>
  DoAddLc(uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
          LteMacSapUser *msu);
  std::vector<uint16_t> DoRemoveLc(uint8_t lcid);
  virtual LteMacSapUser *
  DoConfigureSignalBearer(uint8_t lcId,
                          LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
                          LteMacSapUser *msu);
  void DoReset();

protected:
  LteMacSapUser *m_ccmMacSapUser;
  LteMacSapProvider *m_ccmMacSapProvider;
};

} // namespace ns3

#endif
