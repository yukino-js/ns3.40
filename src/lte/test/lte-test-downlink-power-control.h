
#ifndef LTE_TEST_DOWNLINK_POWER_CONTROL_H
#define LTE_TEST_DOWNLINK_POWER_CONTROL_H

#include "ns3/lte-spectrum-value-helper.h"
#include "ns3/spectrum-test.h"
#include "ns3/spectrum-value.h"
#include "ns3/test.h"
#include <ns3/lte-rrc-sap.h>

using namespace ns3;

class LteDownlinkPowerControlTestSuite : public TestSuite {
public:
  LteDownlinkPowerControlTestSuite();

  double CalculateRbTxPower(double txPower, uint8_t pa);
};

class LteDownlinkPowerControlSpectrumValueTestCase : public TestCase {
public:
  LteDownlinkPowerControlSpectrumValueTestCase(std::string name,
                                               uint16_t earfcn, uint16_t bw,
                                               double powerTx,
                                               std::map<int, double> powerTxMap,
                                               std::vector<int> activeRbs,
                                               SpectrumValue &expected);
  ~LteDownlinkPowerControlSpectrumValueTestCase() override;

private:
  void DoRun() override;
  Ptr<SpectrumValue> m_actual;
  Ptr<SpectrumValue> m_expected;
};

class LteDownlinkPowerControlTestCase : public TestCase {
public:
  LteDownlinkPowerControlTestCase(bool changePower, uint8_t pa,
                                  std::string name);
  ~LteDownlinkPowerControlTestCase() override;

private:
  void DoRun() override;

  bool m_changePdschConfigDedicated;
  LteRrcSap::PdschConfigDedicated m_pdschConfigDedicated;
  double m_expectedPowerDiff;
};

class LteDownlinkPowerControlRrcConnectionReconfigurationTestCase
    : public TestCase {
public:
  LteDownlinkPowerControlRrcConnectionReconfigurationTestCase(bool useIdealRrc,
                                                              std::string name);
  ~LteDownlinkPowerControlRrcConnectionReconfigurationTestCase() override;

  void ConnectionReconfigurationEnb(std::string context, uint64_t imsi,
                                    uint16_t cellid, uint16_t rnti);

  void ConnectionReconfigurationUe(std::string context, uint64_t imsi,
                                   uint16_t cellid, uint16_t rnti);

  void ChangePdschConfigDedicated(uint16_t rnti, uint8_t pa);

private:
  void DoRun() override;
  bool m_useIdealRrc;

  bool m_changePdschConfigDedicatedTriggered;
  bool m_connectionReconfigurationUeReceived;
  bool m_connectionReconfigurationEnbCompleted;
};

#endif
