
#ifndef LTE_TEST_PRIMARY_CELL_CHANGE_H
#define LTE_TEST_PRIMARY_CELL_CHANGE_H

#include <ns3/lte-ue-rrc.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/test.h>
#include <ns3/vector.h>

#include <vector>

using namespace ns3;

class LtePrimaryCellChangeTestSuite : public TestSuite {
public:
  LtePrimaryCellChangeTestSuite();
};

class LtePrimaryCellChangeTestCase : public TestCase {
public:
  LtePrimaryCellChangeTestCase(std::string name, bool isIdealRrc,
                               int64_t rngRun,
                               uint8_t numberOfComponentCarriers,
                               uint8_t sourceComponentCarrier,
                               uint8_t targetComponentCarrier);

  ~LtePrimaryCellChangeTestCase() override;

private:
  void DoRun() override;

  void StateTransitionCallback(std::string context, uint64_t imsi,
                               uint16_t cellId, uint16_t rnti,
                               LteUeRrc::State oldState,
                               LteUeRrc::State newState);
  void InitialPrimaryCellChangeEndOkCallback(std::string context, uint64_t imsi,
                                             uint16_t cellId);
  void ConnectionEstablishedCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti);

  bool m_isIdealRrc;
  int64_t m_rngRun;
  uint8_t m_numberOfComponentCarriers;
  uint8_t m_sourceComponentCarrier;
  uint8_t m_targetComponentCarrier;

  std::map<uint64_t, LteUeRrc::State> m_lastState;
};

#endif
