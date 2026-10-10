
#ifndef LTE_TEST_SECONDARY_CELL_SELECTION_H
#define LTE_TEST_SECONDARY_CELL_SELECTION_H

#include <ns3/lte-ue-rrc.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/test.h>
#include <ns3/vector.h>

#include <vector>

using namespace ns3;

class LteSecondaryCellSelectionTestSuite : public TestSuite {
public:
  LteSecondaryCellSelectionTestSuite();
};

class LteSecondaryCellSelectionTestCase : public TestCase {
public:
  LteSecondaryCellSelectionTestCase(std::string name, bool isIdealRrc,
                                    uint64_t rngRun,
                                    uint8_t numberOfComponentCarriers);

  ~LteSecondaryCellSelectionTestCase() override;

private:
  void DoRun() override;

  void StateTransitionCallback(std::string context, uint64_t imsi,
                               uint16_t cellId, uint16_t rnti,
                               LteUeRrc::State oldState,
                               LteUeRrc::State newState);
  void InitialSecondaryCellSelectionEndOkCallback(std::string context,
                                                  uint64_t imsi,
                                                  uint16_t cellId);
  void ConnectionEstablishedCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti);

  bool m_isIdealRrc;
  uint64_t m_rngRun;
  uint8_t m_numberOfComponentCarriers;

  std::map<uint64_t, LteUeRrc::State> m_lastState;
};

#endif
