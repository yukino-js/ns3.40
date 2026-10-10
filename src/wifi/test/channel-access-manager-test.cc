
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/channel-access-manager.h"
#include "ns3/frame-exchange-manager.h"
#include "ns3/interference-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/qos-txop.h"
#include "ns3/simulator.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/test.h"

#include <list>
#include <numeric>

using namespace ns3;

template <typename TxopType> class ChannelAccessManagerTest;

template <typename TxopType> class TxopTest : public TxopType {
public:
  TxopTest(ChannelAccessManagerTest<TxopType> *test, uint32_t i);

  void QueueTx(uint64_t txTime, uint64_t expectedGrantTime);

private:
  friend class ChannelAccessManagerTest<TxopType>;

  void DoDispose() override;
  void NotifyChannelAccessed(uint8_t linkId,
                             Time txopDuration = Seconds(0)) override;
  bool HasFramesToTransmit(uint8_t linkId) override;
  void NotifySleep(uint8_t linkId) override;
  void NotifyWakeUp(uint8_t linkId) override;
  void GenerateBackoff(uint8_t linkId) override;

  typedef std::pair<uint64_t, uint64_t> ExpectedGrant;
  typedef std::list<ExpectedGrant> ExpectedGrants;

  struct ExpectedBackoff {
    uint64_t at;
    uint32_t nSlots;
  };

  typedef std::list<ExpectedBackoff> ExpectedBackoffs;

  ExpectedBackoffs m_expectedInternalCollision;
  ExpectedBackoffs m_expectedBackoff;
  ExpectedGrants m_expectedGrants;

  ChannelAccessManagerTest<TxopType> *m_test;
  uint32_t m_i;
};

class ChannelAccessManagerStub : public ChannelAccessManager {
public:
  ChannelAccessManagerStub() {}

  void SetSifs(Time sifs) { m_sifs = sifs; }

  void SetSlot(Time slot) { m_slot = slot; }

  void SetEifsNoDifs(Time eifsNoDifs) { m_eifsNoDifs = eifsNoDifs; }

private:
  Time GetSifs() const override { return m_sifs; }

  Time GetSlot() const override { return m_slot; }

  Time GetEifsNoDifs() const override { return m_eifsNoDifs; }

  Time m_slot;
  Time m_sifs;
  Time m_eifsNoDifs;
};

template <typename TxopType>
class FrameExchangeManagerStub : public FrameExchangeManager {
public:
  FrameExchangeManagerStub(ChannelAccessManagerTest<TxopType> *test)
      : m_test(test) {}

  bool StartTransmission(Ptr<Txop> dcf, uint16_t allowedWidth) override {
    dcf->NotifyChannelAccessed(0);
    return true;
  }

  void NotifyInternalCollision(Ptr<Txop> txop) override {
    m_test->NotifyInternalCollision(DynamicCast<TxopTest<TxopType>>(txop));
  }

  void NotifySwitchingStartNow(Time duration) override {
    m_test->NotifyChannelSwitching();
  }

private:
  ChannelAccessManagerTest<TxopType> *m_test;
};

template <typename TxopType> class ChannelAccessManagerTest : public TestCase {
public:
  ChannelAccessManagerTest();
  void DoRun() override;

  void NotifyAccessGranted(uint32_t i);
  void NotifyInternalCollision(Ptr<TxopTest<TxopType>> state);
  void GenerateBackoff(uint32_t i);
  void NotifyChannelSwitching();

private:
  void StartTest(uint64_t slotTime, uint64_t sifs, uint64_t eifsNoDifsNoSifs,
                 uint32_t ackTimeoutValue = 20, uint16_t chWidth = 20);
  void AddTxop(uint32_t aifsn);
  void EndTest();
  void ExpectInternalCollision(uint64_t time, uint32_t nSlots, uint32_t from);
  void ExpectBackoff(uint64_t time, uint32_t nSlots, uint32_t from);
  void ExpectBusy(uint64_t time, bool busy);
  void DoCheckBusy(bool busy);
  void AddRxOkEvt(uint64_t at, uint64_t duration);
  void AddRxErrorEvt(uint64_t at, uint64_t duration);
  void AddRxErrorEvt(uint64_t at, uint64_t duration, uint64_t timeUntilError);
  void AddRxInsideSifsEvt(uint64_t at, uint64_t duration);
  void AddTxEvt(uint64_t at, uint64_t duration);
  void AddNavReset(uint64_t at, uint64_t duration);
  void AddNavStart(uint64_t at, uint64_t duration);
  void AddAckTimeoutReset(uint64_t at);
  void AddAccessRequest(uint64_t at, uint64_t txTime,
                        uint64_t expectedGrantTime, uint32_t from);
  void AddAccessRequestWithAckTimeout(uint64_t at, uint64_t txTime,
                                      uint64_t expectedGrantTime,
                                      uint32_t from);
  void AddAccessRequestWithSuccessfulAck(uint64_t at, uint64_t txTime,
                                         uint64_t expectedGrantTime,
                                         uint32_t ackDelay, uint32_t from);
  void DoAccessRequest(uint64_t txTime, uint64_t expectedGrantTime,
                       Ptr<TxopTest<TxopType>> state);
  void AddCcaBusyEvt(uint64_t at, uint64_t duration,
                     WifiChannelListType channelType = WIFI_CHANLIST_PRIMARY,
                     const std::vector<Time> &per20MhzDurations = {});
  void AddSwitchingEvt(uint64_t at, uint64_t duration);
  void AddRxStartEvt(uint64_t at, uint64_t duration);

  typedef std::vector<Ptr<TxopTest<TxopType>>> TxopTests;

  Ptr<FrameExchangeManagerStub<TxopType>> m_feManager;
  Ptr<ChannelAccessManagerStub> m_ChannelAccessManager;
  Ptr<SpectrumWifiPhy> m_phy;
  TxopTests m_txop;
  uint32_t m_ackTimeoutValue;
};

template <typename TxopType>
void TxopTest<TxopType>::QueueTx(uint64_t txTime, uint64_t expectedGrantTime) {
  m_expectedGrants.emplace_back(txTime, expectedGrantTime);
}

template <typename TxopType>
TxopTest<TxopType>::TxopTest(ChannelAccessManagerTest<TxopType> *test,
                             uint32_t i)
    : m_test(test), m_i(i) {}

template <typename TxopType> void TxopTest<TxopType>::DoDispose() {
  m_test = nullptr;
  TxopType::DoDispose();
}

template <typename TxopType>
void TxopTest<TxopType>::NotifyChannelAccessed(uint8_t linkId,
                                               Time txopDuration) {
  Txop::GetLink(0).access = Txop::NOT_REQUESTED;
  m_test->NotifyAccessGranted(m_i);
}

template <typename TxopType>
void TxopTest<TxopType>::GenerateBackoff(uint8_t linkId) {
  m_test->GenerateBackoff(m_i);
}

template <typename TxopType>
bool TxopTest<TxopType>::HasFramesToTransmit(uint8_t linkId) {
  return !m_expectedGrants.empty();
}

template <typename TxopType>
void TxopTest<TxopType>::NotifySleep(uint8_t linkId) {}

template <typename TxopType>
void TxopTest<TxopType>::NotifyWakeUp(uint8_t linkId) {}

template <typename TxopType>
ChannelAccessManagerTest<TxopType>::ChannelAccessManagerTest()
    : TestCase("ChannelAccessManager") {}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::NotifyAccessGranted(uint32_t i) {
  Ptr<TxopTest<TxopType>> state = m_txop[i];
  NS_TEST_EXPECT_MSG_EQ(state->m_expectedGrants.empty(), false,
                        "Have expected grants");
  if (!state->m_expectedGrants.empty()) {
    std::pair<uint64_t, uint64_t> expected = state->m_expectedGrants.front();
    state->m_expectedGrants.pop_front();
    NS_TEST_EXPECT_MSG_EQ(Simulator::Now(), MicroSeconds(expected.second),
                          "Expected access grant is now");
    m_ChannelAccessManager->NotifyTxStartNow(MicroSeconds(expected.first));
    m_ChannelAccessManager->NotifyAckTimeoutStartNow(
        MicroSeconds(m_ackTimeoutValue + expected.first));
  }
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddTxEvt(uint64_t at,
                                                  uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyTxStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::NotifyInternalCollision(
    Ptr<TxopTest<TxopType>> state) {
  NS_TEST_EXPECT_MSG_EQ(state->m_expectedInternalCollision.empty(), false,
                        "Have expected internal collisions");
  if (!state->m_expectedInternalCollision.empty()) {
    struct TxopTest<TxopType>::ExpectedBackoff expected =
        state->m_expectedInternalCollision.front();
    state->m_expectedInternalCollision.pop_front();
    NS_TEST_EXPECT_MSG_EQ(Simulator::Now(), MicroSeconds(expected.at),
                          "Expected internal collision time is now");
    state->StartBackoffNow(expected.nSlots, 0);
  }
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::GenerateBackoff(uint32_t i) {
  Ptr<TxopTest<TxopType>> state = m_txop[i];
  NS_TEST_EXPECT_MSG_EQ(state->m_expectedBackoff.empty(), false,
                        "Have expected backoffs");
  if (!state->m_expectedBackoff.empty()) {
    struct TxopTest<TxopType>::ExpectedBackoff expected =
        state->m_expectedBackoff.front();
    state->m_expectedBackoff.pop_front();
    NS_TEST_EXPECT_MSG_EQ(Simulator::Now(), MicroSeconds(expected.at),
                          "Expected backoff is now");
    state->StartBackoffNow(expected.nSlots, 0);
  }
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::NotifyChannelSwitching() {
  for (auto &state : m_txop) {
    if (!state->m_expectedGrants.empty()) {
      std::pair<uint64_t, uint64_t> expected = state->m_expectedGrants.front();
      state->m_expectedGrants.pop_front();
      NS_TEST_EXPECT_MSG_EQ(Simulator::Now(), MicroSeconds(expected.second),
                            "Expected grant is now");
    }
    state->Txop::GetLink(0).access = Txop::NOT_REQUESTED;
  }
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::ExpectInternalCollision(
    uint64_t time, uint32_t nSlots, uint32_t from) {
  Ptr<TxopTest<TxopType>> state = m_txop[from];
  struct TxopTest<TxopType>::ExpectedBackoff col;
  col.at = time;
  col.nSlots = nSlots;
  state->m_expectedInternalCollision.push_back(col);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::ExpectBackoff(uint64_t time,
                                                       uint32_t nSlots,
                                                       uint32_t from) {
  Ptr<TxopTest<TxopType>> state = m_txop[from];
  struct TxopTest<TxopType>::ExpectedBackoff backoff;
  backoff.at = time;
  backoff.nSlots = nSlots;
  state->m_expectedBackoff.push_back(backoff);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::ExpectBusy(uint64_t time, bool busy) {
  Simulator::Schedule(MicroSeconds(time) - Now(),
                      &ChannelAccessManagerTest::DoCheckBusy, this, busy);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::DoCheckBusy(bool busy) {
  NS_TEST_EXPECT_MSG_EQ(m_ChannelAccessManager->IsBusy(), busy,
                        "Incorrect busy/idle state");
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::StartTest(uint64_t slotTime,
                                                   uint64_t sifs,
                                                   uint64_t eifsNoDifsNoSifs,
                                                   uint32_t ackTimeoutValue,
                                                   uint16_t chWidth) {
  m_ChannelAccessManager = CreateObject<ChannelAccessManagerStub>();
  m_feManager = CreateObject<FrameExchangeManagerStub<TxopType>>(this);
  m_ChannelAccessManager->SetupFrameExchangeManager(m_feManager);
  m_ChannelAccessManager->SetSlot(MicroSeconds(slotTime));
  m_ChannelAccessManager->SetSifs(MicroSeconds(sifs));
  m_ChannelAccessManager->SetEifsNoDifs(MicroSeconds(eifsNoDifsNoSifs + sifs));
  m_ackTimeoutValue = ackTimeoutValue;
  m_phy = CreateObject<SpectrumWifiPhy>();
  m_phy->SetInterferenceHelper(CreateObject<InterferenceHelper>());
  m_phy->AddChannel(CreateObject<MultiModelSpectrumChannel>());
  m_phy->SetOperatingChannel(
      WifiPhy::ChannelTuple{0, chWidth, WIFI_PHY_BAND_UNSPECIFIED, 0});
  m_phy->ConfigureStandard(WIFI_STANDARD_80211ac);
  m_ChannelAccessManager->SetupPhyListener(m_phy);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddTxop(uint32_t aifsn) {
  Ptr<TxopTest<TxopType>> txop =
      CreateObject<TxopTest<TxopType>>(this, m_txop.size());
  m_txop.push_back(txop);
  m_ChannelAccessManager->Add(txop);
  auto mac = CreateObject<AdhocWifiMac>();
  mac->SetWifiPhys({nullptr});
  txop->SetWifiMac(mac);
  txop->SetAifsn(aifsn);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::EndTest() {
  Simulator::Run();

  for (auto i = m_txop.begin(); i != m_txop.end(); i++) {
    Ptr<TxopTest<TxopType>> state = *i;
    NS_TEST_EXPECT_MSG_EQ(state->m_expectedGrants.empty(), true,
                          "Have no expected grants");
    NS_TEST_EXPECT_MSG_EQ(state->m_expectedInternalCollision.empty(), true,
                          "Have no internal collisions");
    NS_TEST_EXPECT_MSG_EQ(state->m_expectedBackoff.empty(), true,
                          "Have no expected backoffs");
    state->Dispose();
    state = nullptr;
  }
  m_txop.clear();

  m_ChannelAccessManager->RemovePhyListener(m_phy);
  m_phy->Dispose();
  m_ChannelAccessManager->Dispose();
  m_ChannelAccessManager = nullptr;
  m_feManager = nullptr;
  Simulator::Destroy();
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddRxOkEvt(uint64_t at,
                                                    uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyRxStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
  Simulator::Schedule(MicroSeconds(at + duration) - Now(),
                      &ChannelAccessManager::NotifyRxEndOkNow,
                      m_ChannelAccessManager);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddRxInsideSifsEvt(uint64_t at,
                                                            uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyRxStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddRxErrorEvt(uint64_t at,
                                                       uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyRxStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
  Simulator::Schedule(MicroSeconds(at + duration) - Now(),
                      &ChannelAccessManager::NotifyRxEndErrorNow,
                      m_ChannelAccessManager);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddRxErrorEvt(
    uint64_t at, uint64_t duration, uint64_t timeUntilError) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyRxStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
  Simulator::Schedule(MicroSeconds(at + timeUntilError) - Now(),
                      &ChannelAccessManager::NotifyRxEndErrorNow,
                      m_ChannelAccessManager);
  Simulator::Schedule(MicroSeconds(at + timeUntilError) - Now(),
                      &ChannelAccessManager::NotifyCcaBusyStartNow,
                      m_ChannelAccessManager,
                      MicroSeconds(duration - timeUntilError),
                      WIFI_CHANLIST_PRIMARY, std::vector<Time>{});
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddNavReset(uint64_t at,
                                                     uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyNavResetNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddNavStart(uint64_t at,
                                                     uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyNavStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddAckTimeoutReset(uint64_t at) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyAckTimeoutResetNow,
                      m_ChannelAccessManager);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddAccessRequest(
    uint64_t at, uint64_t txTime, uint64_t expectedGrantTime, uint32_t from) {
  AddAccessRequestWithSuccessfulAck(at, txTime, expectedGrantTime, 0, from);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddAccessRequestWithAckTimeout(
    uint64_t at, uint64_t txTime, uint64_t expectedGrantTime, uint32_t from) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManagerTest::DoAccessRequest, this, txTime,
                      expectedGrantTime, m_txop[from]);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddAccessRequestWithSuccessfulAck(
    uint64_t at, uint64_t txTime, uint64_t expectedGrantTime, uint32_t ackDelay,
    uint32_t from) {
  NS_ASSERT(ackDelay < m_ackTimeoutValue);
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManagerTest::DoAccessRequest, this, txTime,
                      expectedGrantTime, m_txop[from]);
  AddAckTimeoutReset(expectedGrantTime + txTime + ackDelay);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::DoAccessRequest(
    uint64_t txTime, uint64_t expectedGrantTime,
    Ptr<TxopTest<TxopType>> state) {
  if (m_ChannelAccessManager->NeedBackoffUponAccess(state)) {
    state->GenerateBackoff(0);
  }
  state->QueueTx(txTime, expectedGrantTime);
  m_ChannelAccessManager->RequestAccess(state);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddCcaBusyEvt(
    uint64_t at, uint64_t duration, WifiChannelListType channelType,
    const std::vector<Time> &per20MhzDurations) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyCcaBusyStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration),
                      channelType, per20MhzDurations);
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddSwitchingEvt(uint64_t at,
                                                         uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifySwitchingStartNow,
                      m_ChannelAccessManager, nullptr, MicroSeconds(duration));
}

template <typename TxopType>
void ChannelAccessManagerTest<TxopType>::AddRxStartEvt(uint64_t at,
                                                       uint64_t duration) {
  Simulator::Schedule(MicroSeconds(at) - Now(),
                      &ChannelAccessManager::NotifyRxStartNow,
                      m_ChannelAccessManager, MicroSeconds(duration));
}

template <> void ChannelAccessManagerTest<Txop>::DoRun() {
  StartTest(1, 3, 10);
  AddTxop(1);
  AddAccessRequest(1, 1, 5, 0);
  AddAccessRequest(8, 2, 12, 0);
  EndTest();

  StartTest(1, 3, 10);
  AddTxop(1);
  AddAccessRequest(1, 1, 5, 0);
  AddRxInsideSifsEvt(7, 10);
  AddTxEvt(9, 1);
  AddAccessRequest(14, 2, 18, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddRxOkEvt(80, 20);
  AddAccessRequest(30, 2, 118, 0);
  ExpectBackoff(30, 4, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddAccessRequest(30, 2, 70, 0);
  ExpectBackoff(30, 0, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddRxOkEvt(60, 40);
  AddAccessRequest(30, 2, 110, 0);
  ExpectBackoff(30, 0, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddAccessRequest(62, 2, 72, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddAccessRequest(70, 2, 80, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxErrorEvt(20, 40);
  AddAccessRequest(30, 2, 102, 0);
  ExpectBackoff(30, 4, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxErrorEvt(20, 40);
  AddAccessRequest(70, 2, 86, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxErrorEvt(20, 40, 20);
  ExpectBusy(41, true);
  ExpectBusy(59, true);
  ExpectBusy(61, false);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxErrorEvt(20, 40);
  AddAccessRequest(30, 2, 101, 0);
  ExpectBackoff(30, 4, 0);
  AddRxOkEvt(69, 6);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddTxop(3);
  AddRxOkEvt(20, 40);
  AddAccessRequest(30, 10, 78, 0);
  ExpectBackoff(30, 2, 0);
  AddAccessRequest(40, 2, 110, 1);
  ExpectBackoff(40, 0, 1);
  ExpectInternalCollision(78, 1, 1);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(0);
  AddTxop(2);
  AddAccessRequestWithAckTimeout(20, 20, 34, 1);
  AddAccessRequest(64, 10, 80, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(0);
  AddTxop(2);
  AddAccessRequestWithSuccessfulAck(20, 20, 34, 2, 1);
  AddAccessRequest(55, 10, 62, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(2);
  AddAccessRequest(20, 20, 34, 0);
  AddRxOkEvt(60, 2);
  AddAccessRequest(61, 10, 80, 0);
  ExpectBackoff(61, 1, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddNavStart(60, 15);
  AddRxOkEvt(66, 5);
  AddNavStart(71, 0);
  AddAccessRequest(30, 10, 93, 0);
  ExpectBackoff(30, 2, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddNavStart(60, 15);
  AddRxOkEvt(66, 5);
  AddNavReset(71, 2);
  AddAccessRequest(30, 10, 91, 0);
  ExpectBackoff(30, 2, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(2);
  AddRxOkEvt(20, 40);
  AddAccessRequest(80, 10, 94, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(2);
  AddRxOkEvt(20, 40);
  AddRxOkEvt(78, 8);
  AddAccessRequest(30, 50, 108, 0);
  ExpectBackoff(30, 3, 0);
  EndTest();

  StartTest(1, 3, 10);
  AddTxop(1);
  AddSwitchingEvt(0, 20);
  AddAccessRequest(21, 1, 25, 0);
  EndTest();

  StartTest(1, 3, 10);
  AddTxop(1);
  AddSwitchingEvt(20, 20);
  AddCcaBusyEvt(30, 20);
  ExpectBackoff(45, 2, 0);
  AddAccessRequest(45, 1, 56, 0);
  EndTest();

  StartTest(1, 3, 10);
  AddTxop(1);
  AddRxStartEvt(20, 40);
  AddSwitchingEvt(30, 20);
  AddAccessRequest(51, 1, 55, 0);
  EndTest();

  StartTest(1, 3, 10);
  AddTxop(1);
  AddCcaBusyEvt(20, 40);
  AddSwitchingEvt(30, 20);
  AddAccessRequest(51, 1, 55, 0);
  EndTest();

  StartTest(1, 3, 10);
  AddTxop(1);
  AddNavStart(20, 40);
  AddSwitchingEvt(30, 20);
  AddAccessRequest(51, 1, 55, 0);
  EndTest();

  StartTest(1, 3, 10);
  AddTxop(1);
  AddAccessRequestWithAckTimeout(20, 20, 24, 0);
  AddAccessRequest(49, 1, 54, 0);
  AddSwitchingEvt(54, 5);
  AddAccessRequest(60, 1, 64, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 40);
  AddAccessRequest(30, 2, 80, 0);
  ExpectBackoff(30, 4, 0);
  AddSwitchingEvt(80, 20);
  AddAccessRequest(101, 2, 111, 0);
  EndTest();
}

template <> void ChannelAccessManagerTest<QosTxop>::DoRun() {
  StartTest(4, 6, 10, 20, 40);
  AddTxop(1);
  AddRxOkEvt(20, 30);
  AddCcaBusyEvt(50, 10, WIFI_CHANLIST_SECONDARY);
  AddAccessRequest(52, 20, 60, 0);
  EndTest();

  StartTest(4, 6, 10, 20, 80);
  AddTxop(1);
  AddRxOkEvt(20, 30);
  AddCcaBusyEvt(50, 10, WIFI_CHANLIST_SECONDARY);
  AddAccessRequest(58, 20, 60, 0);
  EndTest();

  StartTest(4, 6, 10, 20, 80);
  AddTxop(1);
  AddRxOkEvt(20, 30);
  AddCcaBusyEvt(50, 14, WIFI_CHANLIST_SECONDARY40);
  AddAccessRequest(62, 20, 64, 0);
  EndTest();

  StartTest(4, 6, 10, 20, 160);
  AddTxop(1);
  AddRxErrorEvt(20, 30);
  AddCcaBusyEvt(50, 26, WIFI_CHANLIST_SECONDARY);
  AddAccessRequest(55, 20, 76, 0);
  EndTest();

  StartTest(4, 6, 10, 20, 160);
  AddTxop(1);
  AddRxErrorEvt(20, 30);
  AddCcaBusyEvt(50, 26, WIFI_CHANLIST_SECONDARY40);
  AddAccessRequest(70, 20, 76, 0);
  EndTest();

  StartTest(4, 6, 10, 20, 160);
  AddTxop(1);
  AddRxErrorEvt(20, 30);
  AddCcaBusyEvt(50, 34, WIFI_CHANLIST_SECONDARY80);
  AddAccessRequest(82, 20, 84, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 30);
  AddAccessRequest(30, 20, 76, 0);
  ExpectBackoff(30, 4, 0);
  EndTest();

  StartTest(4, 6, 10);
  AddTxop(1);
  AddRxOkEvt(20, 30);
  AddRxOkEvt(61, 10);
  AddRxOkEvt(87, 10);
  AddAccessRequest(30, 20, 107, 0);
  ExpectBackoff(30, 3, 0);
  EndTest();
}

class LargestIdlePrimaryChannelTest : public TestCase {
public:
  LargestIdlePrimaryChannelTest();
  ~LargestIdlePrimaryChannelTest() override = default;

private:
  void DoRun() override;

  void RunOne(uint16_t chWidth, WifiChannelListType busyChannel);

  Ptr<ChannelAccessManager> m_cam;
  Ptr<SpectrumWifiPhy> m_phy;
};

LargestIdlePrimaryChannelTest::LargestIdlePrimaryChannelTest()
    : TestCase("Check calculation of the largest idle primary channel") {}

void LargestIdlePrimaryChannelTest::RunOne(uint16_t chWidth,
                                           WifiChannelListType busyChannel) {

  Time start = Simulator::Now();

  Time ccaBusyStartDelay = MilliSeconds(1);
  Time ccaBusyDuration = MilliSeconds(1);
  Simulator::Schedule(
      ccaBusyStartDelay, &ChannelAccessManager::NotifyCcaBusyStartNow, m_cam,
      ccaBusyDuration, busyChannel,
      std::vector<Time>(chWidth == 20 ? 0 : chWidth / 20, Seconds(0)));

  uint16_t idleWidth =
      (busyChannel == WifiChannelListType::WIFI_CHANLIST_PRIMARY)
          ? 0
          : ((1 << (busyChannel - 1)) * 20);

  Time checkTime1 = start + ccaBusyStartDelay + ccaBusyDuration / 2;
  Simulator::Schedule(checkTime1 - start, [=]() {
    Time interval1 = (ccaBusyStartDelay + ccaBusyDuration) / 2;
    NS_TEST_EXPECT_MSG_EQ(
        m_cam->GetLargestIdlePrimaryChannel(interval1, checkTime1), idleWidth,
        "Incorrect width of the idle channel in an interval "
            << "ending within CCA_BUSY (channel width: " << chWidth
            << " MHz, busy channel: " << busyChannel << ")");
  });

  Time ccaBusyRxInterval = MilliSeconds(1);
  Time checkTime2 =
      start + ccaBusyStartDelay + ccaBusyDuration + ccaBusyRxInterval / 2;
  Simulator::Schedule(checkTime2 - start, [=]() {
    Time interval2 = (ccaBusyDuration + ccaBusyRxInterval) / 2;
    NS_TEST_EXPECT_MSG_EQ(
        m_cam->GetLargestIdlePrimaryChannel(interval2, checkTime2), idleWidth,
        "Incorrect width of the idle channel in an interval "
            << "starting within CCA_BUSY (channel width: " << chWidth
            << " MHz, busy channel: " << busyChannel << ")");
  });

  Time rxDuration = MilliSeconds(1);
  Simulator::Schedule(ccaBusyStartDelay + ccaBusyDuration + ccaBusyRxInterval,
                      &ChannelAccessManager::NotifyRxStartNow, m_cam,
                      rxDuration);

  Time checkTime3 = start + ccaBusyStartDelay + ccaBusyDuration +
                    ccaBusyRxInterval + rxDuration;
  Simulator::Schedule(checkTime3 - start, [=]() {
    Time interval3 = ccaBusyDuration / 2 + ccaBusyRxInterval;
    Time end3 = checkTime3 - rxDuration;
    NS_TEST_EXPECT_MSG_EQ(m_cam->GetLargestIdlePrimaryChannel(interval3, end3),
                          idleWidth,
                          "Incorrect width of the idle channel in an interval "
                              << "preceding RX start and overlapping CCA_BUSY "
                              << "(channel width: " << chWidth
                              << " MHz, busy channel: " << busyChannel << ")");
  });

  const Time &checkTime4 = checkTime3;
  Simulator::Schedule(checkTime4 - start, [=]() {
    const Time &interval4 = ccaBusyRxInterval;
    Time end4 = checkTime4 - rxDuration;
    NS_TEST_EXPECT_MSG_EQ(
        m_cam->GetLargestIdlePrimaryChannel(interval4, end4), chWidth,
        "Incorrect width of the idle channel in the interval "
            << "following CCA_BUSY and preceding RX start (channel "
            << "width: " << chWidth << " MHz, busy channel: " << busyChannel
            << ")");
  });

  Time interval5 = MilliSeconds(1);
  Time checkTime5 = checkTime4 + interval5;
  Simulator::Schedule(checkTime5 - start, [=]() {
    NS_TEST_EXPECT_MSG_EQ(
        m_cam->GetLargestIdlePrimaryChannel(interval5, checkTime5), chWidth,
        "Incorrect width of the idle channel in an interval "
            << "following RX end (channel width: " << chWidth
            << " MHz, busy channel: " << busyChannel << ")");
  });

  const Time &checkTime6 = checkTime5;
  Simulator::Schedule(checkTime6 - start, [=]() {
    Time interval6 = interval5 + rxDuration / 2;
    NS_TEST_EXPECT_MSG_EQ(
        m_cam->GetLargestIdlePrimaryChannel(interval6, checkTime6), 0,
        "Incorrect width of the idle channel in an interval "
            << "overlapping RX (channel width: " << chWidth
            << " MHz, busy channel: " << busyChannel << ")");
  });
}

void LargestIdlePrimaryChannelTest::DoRun() {
  m_cam = CreateObject<ChannelAccessManager>();
  uint16_t delay = 0;
  uint8_t channel = 0;
  std::list<WifiChannelListType> busyChannels;

  for (uint16_t chWidth : {20, 40, 80, 160}) {
    busyChannels.push_back(static_cast<WifiChannelListType>(channel));

    for (const auto busyChannel : busyChannels) {
      Simulator::Schedule(Seconds(delay), [this, chWidth, busyChannel]() {
        if (m_phy) {
          m_cam->RemovePhyListener(m_phy);
          m_phy->Dispose();
        }
        m_phy = CreateObject<SpectrumWifiPhy>();
        m_phy->SetInterferenceHelper(CreateObject<InterferenceHelper>());
        m_phy->AddChannel(CreateObject<MultiModelSpectrumChannel>());
        m_phy->SetOperatingChannel(
            WifiPhy::ChannelTuple{0, chWidth, WIFI_PHY_BAND_5GHZ, 0});
        m_phy->ConfigureStandard(WIFI_STANDARD_80211ax);
        m_cam->SetupPhyListener(m_phy);
        RunOne(chWidth, busyChannel);
      });
      delay++;
    }
    channel++;
  }

  Simulator::Run();
  m_cam->RemovePhyListener(m_phy);
  m_phy->Dispose();
  m_cam->Dispose();
  Simulator::Destroy();
}

class TxopTestSuite : public TestSuite {
public:
  TxopTestSuite();
};

TxopTestSuite::TxopTestSuite() : TestSuite("wifi-devices-dcf", UNIT) {
  AddTestCase(new ChannelAccessManagerTest<Txop>, TestCase::QUICK);
}

static TxopTestSuite g_dcfTestSuite;

class QosTxopTestSuite : public TestSuite {
public:
  QosTxopTestSuite();
};

QosTxopTestSuite::QosTxopTestSuite() : TestSuite("wifi-devices-edca", UNIT) {
  AddTestCase(new ChannelAccessManagerTest<QosTxop>, TestCase::QUICK);
}

static QosTxopTestSuite g_edcaTestSuite;

class ChannelAccessManagerTestSuite : public TestSuite {
public:
  ChannelAccessManagerTestSuite();
};

ChannelAccessManagerTestSuite::ChannelAccessManagerTestSuite()
    : TestSuite("wifi-channel-access-manager", UNIT) {
  AddTestCase(new LargestIdlePrimaryChannelTest, TestCase::QUICK);
}

static ChannelAccessManagerTestSuite g_camTestSuite;
