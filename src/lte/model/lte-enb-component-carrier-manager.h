
#ifndef LTE_ENB_COMPONENT_CARRIER_MANAGER_H
#define LTE_ENB_COMPONENT_CARRIER_MANAGER_H

#include "lte-ccm-mac-sap.h"
#include "lte-ccm-rrc-sap.h"
#include "lte-enb-cmac-sap.h"
#include "lte-enb-rrc.h"
#include "lte-mac-sap.h"
#include "lte-rrc-sap.h"

#include <ns3/object.h>

#include <map>
#include <vector>

namespace ns3 {

class LteCcmRrcSapUser;
class LteCcmRrcSapProvider;
class LteMacSapUser;
class LteMacSapProvider;
class LteEnbCmacSapProvider;
class LteCcmMacSapProvider;

class LteEnbComponentCarrierManager : public Object {
public:
  LteEnbComponentCarrierManager();
  ~LteEnbComponentCarrierManager() override;
  static TypeId GetTypeId();

  virtual void SetLteCcmRrcSapUser(LteCcmRrcSapUser *s);

  virtual LteCcmRrcSapProvider *GetLteCcmRrcSapProvider();

  virtual LteCcmMacSapUser *GetLteCcmMacSapUser();

  virtual LteMacSapProvider *GetLteMacSapProvider();

  virtual bool SetMacSapProvider(uint8_t componentCarrierId,
                                 LteMacSapProvider *sap);

  virtual bool SetCcmMacSapProviders(uint8_t componentCarrierId,
                                     LteCcmMacSapProvider *sap);

  virtual void SetNumberOfComponentCarriers(uint16_t noOfComponentCarriers);

protected:
  void DoDispose() override;

  virtual void DoReportUeMeas(uint16_t rnti,
                              LteRrcSap::MeasResults measResults) = 0;

  struct UeInfo {
    std::map<uint8_t, LteMacSapUser *> m_ueAttached;
    std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> m_rlcLcInstantiated;
    uint8_t m_enabledComponentCarrier;
    uint8_t m_ueState;
  };

  std::map<uint16_t, UeInfo> m_ueInfo;
  uint16_t m_noOfComponentCarriers;
  Ptr<LteEnbRrc> m_rrc;

  LteMacSapProvider *m_macSapProvider;
  std::map<uint8_t, LteMacSapProvider *> m_macSapProvidersMap;
  std::map<uint8_t, LteCcmMacSapProvider *> m_ccmMacSapProviderMap;
  LteCcmMacSapUser *m_ccmMacSapUser;
  LteCcmRrcSapUser *m_ccmRrcSapUser;
  LteCcmRrcSapProvider *m_ccmRrcSapProvider;
};

} // namespace ns3

#endif
