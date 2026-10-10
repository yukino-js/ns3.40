
#ifndef LTE_TEST_RADIO_LINK_FAILURE_H
#define LTE_TEST_RADIO_LINK_FAILURE_H

#include <ns3/lte-ue-rrc.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/test.h>
#include <ns3/vector.h>

#include <vector>

namespace ns3 {

class LteUeNetDevice;

}

using namespace ns3;

class LteRadioLinkFailureTestSuite : public TestSuite {
public:
  LteRadioLinkFailureTestSuite();
};

class LteRadioLinkFailureTestCase : public TestCase {
public:
  LteRadioLinkFailureTestCase(uint32_t numEnbs, uint32_t numUes, Time simTime,
                              bool isIdealRrc,
                              std::vector<Vector> uePositionList,
                              std::vector<Vector> enbPositionList,
                              Vector ueJumpAwayPosition,
                              std::vector<Time> checkConnectedList);

  ~LteRadioLinkFailureTestCase() override;

private:
  std::string BuildNameString(uint32_t numEnbs, uint32_t numUes,
                              bool isIdealRrc);
  void DoRun() override;

  void CheckConnected(Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices);

  void CheckIdle(Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices);

  bool CheckUeExistAtEnb(uint16_t rnti, Ptr<NetDevice> enbDevice);

  void UeStateTransitionCallback(std::string context, uint64_t imsi,
                                 uint16_t cellId, uint16_t rnti,
                                 LteUeRrc::State oldState,
                                 LteUeRrc::State newState);

  void ConnectionEstablishedUeCallback(std::string context, uint64_t imsi,
                                       uint16_t cellId, uint16_t rnti);

  void ConnectionEstablishedEnbCallback(std::string context, uint64_t imsi,
                                        uint16_t cellId, uint16_t rnti);

  void ConnectionReleaseAtEnbCallback(std::string context, uint64_t imsi,
                                      uint16_t cellId, uint16_t rnti);

  void PhySyncDetectionCallback(std::string context, uint64_t imsi,
                                uint16_t rnti, uint16_t cellId,
                                std::string type, uint8_t count);

  void RadioLinkFailureCallback(std::string context, uint64_t imsi,
                                uint16_t cellId, uint16_t rnti);

  void JumpAway(Vector UeJumpAwayPositionList);

  uint32_t m_numEnbs;
  uint32_t m_numUes;
  Time m_simTime;
  bool m_isIdealRrc;
  std::vector<Vector> m_uePositionList;
  std::vector<Vector> m_enbPositionList;
  std::vector<Time> m_checkConnectedList;
  Vector m_ueJumpAwayPosition;

  LteUeRrc::State m_lastState;

  bool m_radioLinkFailureDetected;
  uint32_t m_numOfInSyncIndications;
  uint32_t m_numOfOutOfSyncIndications;
  Ptr<MobilityModel> m_ueMobility;
};

#endif
