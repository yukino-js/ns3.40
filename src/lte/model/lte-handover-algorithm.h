
#ifndef LTE_HANDOVER_ALGORITHM_H
#define LTE_HANDOVER_ALGORITHM_H

#include "lte-rrc-sap.h"

#include <ns3/object.h>

namespace ns3 {

class LteHandoverManagementSapUser;
class LteHandoverManagementSapProvider;

class LteHandoverAlgorithm : public Object {
public:
  LteHandoverAlgorithm();
  ~LteHandoverAlgorithm() override;

  static TypeId GetTypeId();

  virtual void
  SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s) = 0;

  virtual LteHandoverManagementSapProvider *
  GetLteHandoverManagementSapProvider() = 0;

protected:
  void DoDispose() override;

  virtual void DoReportUeMeas(uint16_t rnti,
                              LteRrcSap::MeasResults measResults) = 0;
};

} // namespace ns3

#endif
