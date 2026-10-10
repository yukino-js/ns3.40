
#ifndef LTE_TEST_CELL_SELECTION_H
#define LTE_TEST_CELL_SELECTION_H

#include <ns3/lte-ue-rrc.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/test.h>
#include <ns3/vector.h>

#include <vector>

namespace ns3 {

class LteUeNetDevice;

}

using namespace ns3;

class LteCellSelectionTestSuite : public TestSuite {
public:
  LteCellSelectionTestSuite();
};

class LteCellSelectionTestCase : public TestCase {
public:
  struct UeSetup_t {
    Vector position;
    bool isCsgMember;
    Time checkPoint;
    uint16_t expectedCellId1;
    uint16_t expectedCellId2;
    UeSetup_t(double relPosX, double relPosY, bool isCsgMember, Time checkPoint,
              uint16_t expectedCellId1, uint16_t expectedCellId2);
  };

  LteCellSelectionTestCase(std::string name, bool isEpcMode, bool isIdealRrc,
                           double interSiteDistance,
                           std::vector<UeSetup_t> ueSetupList);

  ~LteCellSelectionTestCase() override;

private:
  void DoRun() override;

  void CheckPoint(Ptr<LteUeNetDevice> ueDev, uint16_t expectedCellId1,
                  uint16_t expectedCellId2);

  void StateTransitionCallback(std::string context, uint64_t imsi,
                               uint16_t cellId, uint16_t rnti,
                               LteUeRrc::State oldState,
                               LteUeRrc::State newState);
  void InitialCellSelectionEndOkCallback(std::string context, uint64_t imsi,
                                         uint16_t cellId);
  void InitialCellSelectionEndErrorCallback(std::string context, uint64_t imsi,
                                            uint16_t cellId);
  void ConnectionEstablishedCallback(std::string context, uint64_t imsi,
                                     uint16_t cellId, uint16_t rnti);

  bool m_isEpcMode;
  bool m_isIdealRrc;
  double m_interSiteDistance;
  std::vector<UeSetup_t> m_ueSetupList;

  std::vector<LteUeRrc::State> m_lastState;
};

#endif
