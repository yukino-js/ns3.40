
#include "ns3/test.h"
#include "ns3/trickle-timer.h"

#include <algorithm>
#include <numeric>
#include <vector>

namespace ns3 {

namespace tests {

class TrickleTimerTestCase : public TestCase {
public:
  TrickleTimerTestCase();
  void DoRun() override;
  void ExpireTimer();
  std::vector<Time> m_expiredTimes;

  void TransientOver();

  void TestSteadyState(Time unit);

  void TestRedundancy(Time unit);

  void ConsistentEvent(Time interval, TrickleTimer *tricklePtr);

  bool m_enableDataCollection;
};

TrickleTimerTestCase::TrickleTimerTestCase()
    : TestCase("Check the Trickle Timer algorithm") {}

void TrickleTimerTestCase::ExpireTimer() {
  if (!m_enableDataCollection) {
    return;
  }

  m_expiredTimes.push_back(Simulator::Now());
}

void TrickleTimerTestCase::TransientOver() { m_enableDataCollection = true; }

void TrickleTimerTestCase::TestSteadyState(Time unit) {
  m_expiredTimes.clear();
  m_enableDataCollection = false;

  TrickleTimer trickle(unit, 4, 1);
  trickle.SetFunction(&TrickleTimerTestCase::ExpireTimer, this);
  trickle.Enable();
  trickle.Reset();

  NS_TEST_EXPECT_MSG_EQ(trickle.GetDoublings(), 4,
                        "The doublings re-compute mechanism is not working.");

  Simulator::Schedule(unit * 31, &TrickleTimerTestCase::TransientOver, this);

  Simulator::Stop(unit * 50000);

  Simulator::Run();
  Simulator::Destroy();

  std::vector<Time> expirationFrequency;

  expirationFrequency.resize(m_expiredTimes.size());
  std::adjacent_difference(m_expiredTimes.begin(), m_expiredTimes.end(),
                           expirationFrequency.begin());
  expirationFrequency.erase(expirationFrequency.begin());

  int64x64_t min = (*std::min_element(expirationFrequency.begin(),
                                      expirationFrequency.end())) /
                   unit;
  int64x64_t max = (*std::max_element(expirationFrequency.begin(),
                                      expirationFrequency.end())) /
                   unit;

  NS_TEST_EXPECT_MSG_GT_OR_EQ(min.GetDouble(), 8, "Timer did fire too fast ??");
  NS_TEST_EXPECT_MSG_LT_OR_EQ(max.GetDouble(), 24,
                              "Timer did fire too slow ??");
}

void TrickleTimerTestCase::TestRedundancy(Time unit) {
  m_expiredTimes.clear();
  m_enableDataCollection = false;

  TrickleTimer trickle(unit, 4, 1);
  trickle.SetFunction(&TrickleTimerTestCase::ExpireTimer, this);
  trickle.Enable();
  trickle.Reset();

  NS_TEST_EXPECT_MSG_EQ(trickle.GetDoublings(), 4,
                        "The doublings re-compute mechanism is not working.");

  Simulator::Schedule(unit * 31, &TrickleTimerTestCase::TransientOver, this);
  Simulator::Schedule(unit * 31, &TrickleTimerTestCase::ConsistentEvent, this,
                      unit * 8, &trickle);

  Simulator::Stop(unit * 50000);

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_EXPECT_MSG_EQ(m_expiredTimes.size(), 0,
                        "Timer did fire while being suppressed ??");
}

void TrickleTimerTestCase::ConsistentEvent(Time interval,
                                           TrickleTimer *tricklePtr) {
  tricklePtr->ConsistentEvent();
  Simulator::Schedule(interval, &TrickleTimerTestCase::ConsistentEvent, this,
                      interval, tricklePtr);
}

void TrickleTimerTestCase::DoRun() {
  TestSteadyState(Time(1));
  TestSteadyState(Seconds(1));
  TestRedundancy(Seconds(1));
}

class TrickleTimerTestSuite : public TestSuite {
public:
  TrickleTimerTestSuite() : TestSuite("trickle-timer") {
    AddTestCase(new TrickleTimerTestCase());
  }
};

static TrickleTimerTestSuite g_trickleTimerTestSuite;

} // namespace tests

} // namespace ns3
