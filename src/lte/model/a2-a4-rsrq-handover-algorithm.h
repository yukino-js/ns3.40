
#ifndef A2_A4_RSRQ_HANDOVER_ALGORITHM_H
#define A2_A4_RSRQ_HANDOVER_ALGORITHM_H

#include "lte-handover-algorithm.h"
#include "lte-handover-management-sap.h"
#include "lte-rrc-sap.h"

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

#include <map>

namespace ns3 {

class A2A4RsrqHandoverAlgorithm : public LteHandoverAlgorithm {
public:
  A2A4RsrqHandoverAlgorithm();

  ~A2A4RsrqHandoverAlgorithm() override;

  static TypeId GetTypeId();

  void
  SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s) override;
  LteHandoverManagementSapProvider *
  GetLteHandoverManagementSapProvider() override;

  friend class MemberLteHandoverManagementSapProvider<
      A2A4RsrqHandoverAlgorithm>;

protected:
  void DoInitialize() override;
  void DoDispose() override;

  void DoReportUeMeas(uint16_t rnti,
                      LteRrcSap::MeasResults measResults) override;

private:
  void EvaluateHandover(uint16_t rnti, uint8_t servingCellRsrq);

  bool IsValidNeighbour(uint16_t cellId);

  void UpdateNeighbourMeasurements(uint16_t rnti, uint16_t cellId,
                                   uint8_t rsrq);

  std::vector<uint8_t> m_a2MeasIds;
  std::vector<uint8_t> m_a4MeasIds;

  class UeMeasure : public SimpleRefCount<UeMeasure> {
  public:
    uint16_t m_cellId;
    uint8_t m_rsrp;
    uint8_t m_rsrq;
  };

  typedef std::map<uint16_t, Ptr<UeMeasure>> MeasurementRow_t;

  typedef std::map<uint16_t, MeasurementRow_t> MeasurementTable_t;

  MeasurementTable_t m_neighbourCellMeasures;

  uint8_t m_servingCellThreshold;

  uint8_t m_neighbourCellOffset;

  LteHandoverManagementSapUser *m_handoverManagementSapUser;
  LteHandoverManagementSapProvider *m_handoverManagementSapProvider;
};

} // namespace ns3

#endif
