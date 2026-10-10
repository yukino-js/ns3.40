#include "ns3/test.h"
#include "ns3/watchdog.h"

namespace ns3 {

namespace tests {

class WatchdogTestCase : public TestCase {
public:
  WatchdogTestCase();
  void DoRun() override;
  void Expire(int arg);
  bool m_expired;
  Time m_expiredTime;
  int m_expiredArgument;
};

WatchdogTestCase::WatchdogTestCase()
    : TestCase("Check that we can keepalive a watchdog") {}

void WatchdogTestCase::Expire(int arg) {
  m_expired = true;
  m_expiredTime = Simulator::Now();
  m_expiredArgument = arg;
}

void WatchdogTestCase::DoRun() {
  m_expired = false;
  m_expiredArgument = 0;
  m_expiredTime = Seconds(0);

  Watchdog watchdog;
  watchdog.SetFunction(&WatchdogTestCase::Expire, this);
  watchdog.SetArguments(1);
  watchdog.Ping(MicroSeconds(10));
  Simulator::Schedule(MicroSeconds(5), &Watchdog::Ping, &watchdog,
                      MicroSeconds(20));
  Simulator::Schedule(MicroSeconds(20), &Watchdog::Ping, &watchdog,
                      MicroSeconds(2));
  Simulator::Schedule(MicroSeconds(23), &Watchdog::Ping, &watchdog,
                      MicroSeconds(17));
  Simulator::Run();
  Simulator::Destroy();
  NS_TEST_ASSERT_MSG_EQ(m_expired, true, "The timer did not expire ??");
  NS_TEST_ASSERT_MSG_EQ(m_expiredTime, MicroSeconds(40),
                        "The timer did not expire at the expected time ?");
  NS_TEST_ASSERT_MSG_EQ(m_expiredArgument, 1,
                        "We did not get the right argument");
}

class WatchdogTestSuite : public TestSuite {
public:
  WatchdogTestSuite() : TestSuite("watchdog") {
    AddTestCase(new WatchdogTestCase());
  }
};

static WatchdogTestSuite g_watchdogTestSuite;

} // namespace tests

} // namespace ns3
