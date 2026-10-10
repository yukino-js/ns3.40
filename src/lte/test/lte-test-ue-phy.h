
#ifndef LTE_TEST_UE_PHY_H
#define LTE_TEST_UE_PHY_H

#include "ns3/lte-control-messages.h"
#include "ns3/lte-phy.h"

namespace ns3 {

class LteTestUePhy : public LtePhy {
public:
  LteTestUePhy();

  LteTestUePhy(Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy);

  ~LteTestUePhy() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  void DoSendMacPdu(Ptr<Packet> p) override;

  Ptr<SpectrumValue> CreateTxPowerSpectralDensity() override;

  void GenerateCtrlCqiReport(const SpectrumValue &sinr) override;

  void GenerateDataCqiReport(const SpectrumValue &sinr) override;

  void ReportInterference(const SpectrumValue &interf) override;

  void ReportRsReceivedPower(const SpectrumValue &power) override;

  virtual void ReceiveLteControlMessage(Ptr<LteControlMessage> msg);

  SpectrumValue GetSinr();

private:
  SpectrumValue m_sinr;
};

} // namespace ns3

#endif
