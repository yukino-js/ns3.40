
#ifndef LTE_TEST_UPLINK_POWER_CONTROL_H
#define LTE_TEST_UPLINK_POWER_CONTROL_H

#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/spectrum-test.h"
#include "ns3/spectrum-value.h"
#include "ns3/test.h"

using namespace ns3;

namespace ns3 {
class LteFfrSimple;
class LteUePowerControl;
class MobilityModel;
} // namespace ns3

class LteUplinkPowerControlTestSuite : public TestSuite {
public:
  LteUplinkPowerControlTestSuite();
};

class LteUplinkPowerControlTestCase : public TestCase {
public:
  LteUplinkPowerControlTestCase(std::string name);
  ~LteUplinkPowerControlTestCase() override;

  void TeleportUe(uint32_t x, uint32_t y, double expectedPuschTxPower,
                  double expectedPucchTxPower, double expectedSrsTxPower);

  void SetTpcConfiguration(uint32_t tpc, uint32_t tpcNum,
                           double expectedPuschTxPower,
                           double expectedPucchTxPower,
                           double expectedSrsTxPower);

  void PuschTxPowerTrace(uint16_t cellId, uint16_t rnti, double txPower);
  void PucchTxPowerTrace(uint16_t cellId, uint16_t rnti, double txPower);
  void SrsTxPowerTrace(uint16_t cellId, uint16_t rnti, double txPower);

protected:
  void DoRun() override;

  Ptr<MobilityModel> m_ueMobility;
  Time m_teleportTime;

  double m_expectedPuschTxPower;
  double m_expectedPucchTxPower;
  double m_expectedSrsTxPower;

  Ptr<LteUePowerControl> m_ueUpc;
  bool m_accumulatedMode;

  Ptr<LteFfrSimple> m_ffrSimple;
};

class LteUplinkOpenLoopPowerControlTestCase
    : public LteUplinkPowerControlTestCase {
public:
  LteUplinkOpenLoopPowerControlTestCase(std::string name);
  ~LteUplinkOpenLoopPowerControlTestCase() override;

private:
  void DoRun() override;
};

class LteUplinkClosedLoopPowerControlAbsoluteModeTestCase
    : public LteUplinkPowerControlTestCase {
public:
  LteUplinkClosedLoopPowerControlAbsoluteModeTestCase(std::string name);
  ~LteUplinkClosedLoopPowerControlAbsoluteModeTestCase() override;

private:
  void DoRun() override;
};

class LteUplinkClosedLoopPowerControlAccumulatedModeTestCase
    : public LteUplinkPowerControlTestCase {
public:
  LteUplinkClosedLoopPowerControlAccumulatedModeTestCase(std::string name);
  ~LteUplinkClosedLoopPowerControlAccumulatedModeTestCase() override;

private:
  void DoRun() override;
};

#endif
