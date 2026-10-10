
#ifndef NO_OP_HANDOVER_ALGORITHM_H
#define NO_OP_HANDOVER_ALGORITHM_H

#include "lte-handover-algorithm.h"
#include "lte-handover-management-sap.h"
#include "lte-rrc-sap.h"

namespace ns3 {

class NoOpHandoverAlgorithm : public LteHandoverAlgorithm {
public:
  NoOpHandoverAlgorithm();

  ~NoOpHandoverAlgorithm() override;

  static TypeId GetTypeId();

  void
  SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s) override;
  LteHandoverManagementSapProvider *
  GetLteHandoverManagementSapProvider() override;

  friend class MemberLteHandoverManagementSapProvider<NoOpHandoverAlgorithm>;

protected:
  void DoInitialize() override;
  void DoDispose() override;

  void DoReportUeMeas(uint16_t rnti,
                      LteRrcSap::MeasResults measResults) override;

private:
  LteHandoverManagementSapUser *m_handoverManagementSapUser;
  LteHandoverManagementSapProvider *m_handoverManagementSapProvider;
};

} // namespace ns3

#endif
