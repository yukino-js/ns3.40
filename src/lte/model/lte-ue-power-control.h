
#ifndef LTE_UE_POWER_CONTROL_H
#define LTE_UE_POWER_CONTROL_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>

#include <vector>

namespace ns3 {

class LteUePowerControl : public Object {
public:
  LteUePowerControl();
  ~LteUePowerControl() override;

  static TypeId GetTypeId();
  void DoInitialize() override;
  void DoDispose() override;

  void SetPcmax(double value);
  double GetPcmax();

  void SetTxPower(double value);
  void ConfigureReferenceSignalPower(int8_t referenceSignalPower);

  void SetCellId(uint16_t cellId);
  void SetRnti(uint16_t rnti);

  void SetPoNominalPusch(int16_t value);
  void SetPoUePusch(int16_t value);
  void SetAlpha(double value);

  void SetRsrp(double value);
  void SetRsrpFilterCoefficient(uint8_t rsrpFilterCoefficient);
  void ReportTpc(uint8_t tpc);

  void CalculatePuschTxPower();
  void CalculatePucchTxPower();
  void CalculateSrsTxPower();

  double GetPuschTxPower(std::vector<int> rb);
  double GetPucchTxPower(std::vector<int> rb);
  double GetSrsTxPower(std::vector<int> rb);

  typedef void (*TxPowerTracedCallback)(uint16_t cellId, uint16_t rnti,
                                        double power);

private:
  void SetSubChannelMask(std::vector<int> mask);

  double m_txPower;
  double m_Pcmax;
  double m_Pcmin;

  double m_curPuschTxPower;
  double m_curPucchTxPower;
  double m_curSrsTxPower;

  double m_referenceSignalPower;
  bool m_rsrpSet;
  double m_rsrp;

  std::vector<int16_t> m_PoNominalPusch;
  std::vector<int16_t> m_PoUePusch;

  int16_t m_PsrsOffset;

  uint16_t m_M_Pusch;
  std::vector<double> m_alpha;
  double m_pathLoss;
  double m_deltaTF;

  std::vector<int8_t> m_deltaPusch;
  double m_fc;

  uint16_t m_srsBandwidth;

  bool m_closedLoop;
  bool m_accumulationEnabled;

  uint16_t m_cellId;
  uint16_t m_rnti;
  uint8_t m_pcRsrpFilterCoefficient;
  TracedCallback<uint16_t, uint16_t, double> m_reportPuschTxPower;
  TracedCallback<uint16_t, uint16_t, double> m_reportPucchTxPower;
  TracedCallback<uint16_t, uint16_t, double> m_reportSrsTxPower;
};

} // namespace ns3

#endif
