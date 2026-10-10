
#ifndef A3_RSRP_HANDOVER_ALGORITHM_H
#define A3_RSRP_HANDOVER_ALGORITHM_H

#include "lte-handover-algorithm.h"
#include "lte-handover-management-sap.h"
#include "lte-rrc-sap.h"

#include <ns3/nstime.h>

namespace ns3 {

class A3RsrpHandoverAlgorithm : public LteHandoverAlgorithm {
public:
  A3RsrpHandoverAlgorithm();

  ~A3RsrpHandoverAlgorithm() override;

  static TypeId GetTypeId();

  void
  SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s) override;
  LteHandoverManagementSapProvider *
  GetLteHandoverManagementSapProvider() override;

  friend class MemberLteHandoverManagementSapProvider<A3RsrpHandoverAlgorithm>;

protected:
  void DoInitialize() override;
  void DoDispose() override;

  void DoReportUeMeas(uint16_t rnti,
                      LteRrcSap::MeasResults measResults) override;

private:
  bool IsValidNeighbour(uint16_t cellId);

  std::vector<uint8_t> m_measIds;

  double m_hysteresisDb;
  Time m_timeToTrigger;

  LteHandoverManagementSapUser *m_handoverManagementSapUser;
  LteHandoverManagementSapProvider *m_handoverManagementSapProvider;
};

} // namespace ns3

#endif
