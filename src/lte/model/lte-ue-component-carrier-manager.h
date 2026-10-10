
#ifndef LTE_UE_COMPONENT_CARRIER_MANAGER_H
#define LTE_UE_COMPONENT_CARRIER_MANAGER_H

#include "lte-mac-sap.h"
#include "lte-ue-ccm-rrc-sap.h"

#include <ns3/object.h>

#include <map>
#include <vector>

namespace ns3 {

class LteUeCcmRrcSapUser;
class LteUeCcmRrcSapProvider;

class LteMacSapUser;
class LteMacSapProvider;

class LteUeComponentCarrierManager : public Object {
public:
  LteUeComponentCarrierManager();
  ~LteUeComponentCarrierManager() override;

  static TypeId GetTypeId();

  virtual void SetLteCcmRrcSapUser(LteUeCcmRrcSapUser *s);

  virtual LteUeCcmRrcSapProvider *GetLteCcmRrcSapProvider();

  virtual LteMacSapProvider *GetLteMacSapProvider() = 0;

  bool SetComponentCarrierMacSapProviders(uint8_t componentCarrierId,
                                          LteMacSapProvider *sap);

  void SetNumberOfComponentCarriers(uint8_t noOfComponentCarriers);

protected:
  void DoDispose() override;

  LteUeCcmRrcSapUser *m_ccmRrcSapUser;
  LteUeCcmRrcSapProvider *m_ccmRrcSapProvider;

  std::map<uint8_t, LteMacSapUser *> m_lcAttached;
  std::map<uint8_t, std::map<uint8_t, LteMacSapProvider *>>
      m_componentCarrierLcMap;
  uint8_t m_noOfComponentCarriers;
  std::map<uint8_t, LteMacSapProvider *> m_macSapProvidersMap;
};

} // namespace ns3

#endif
