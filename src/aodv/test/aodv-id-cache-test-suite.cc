#include "ns3/aodv-id-cache.h"
#include "ns3/test.h"

namespace ns3 {
namespace aodv {

class IdCacheTest : public TestCase {
public:
  IdCacheTest() : TestCase("Id Cache"), cache(Seconds(10)) {}

  void DoRun() override;

private:
  void CheckTimeout1();
  void CheckTimeout2();
  void CheckTimeout3();

  IdCache cache;
};

void IdCacheTest::DoRun() {
  NS_TEST_EXPECT_MSG_EQ(cache.GetLifeTime(), Seconds(10), "Lifetime");
  NS_TEST_EXPECT_MSG_EQ(cache.IsDuplicate(Ipv4Address("1.2.3.4"), 3), false,
                        "Unknown ID & address");
  NS_TEST_EXPECT_MSG_EQ(cache.IsDuplicate(Ipv4Address("1.2.3.4"), 4), false,
                        "Unknown ID");
  NS_TEST_EXPECT_MSG_EQ(cache.IsDuplicate(Ipv4Address("4.3.2.1"), 3), false,
                        "Unknown address");
  NS_TEST_EXPECT_MSG_EQ(cache.IsDuplicate(Ipv4Address("1.2.3.4"), 3), true,
                        "Known address & ID");
  cache.SetLifetime(Seconds(15));
  NS_TEST_EXPECT_MSG_EQ(cache.GetLifeTime(), Seconds(15), "New lifetime");
  cache.IsDuplicate(Ipv4Address("1.1.1.1"), 4);
  cache.IsDuplicate(Ipv4Address("1.1.1.1"), 4);
  cache.IsDuplicate(Ipv4Address("2.2.2.2"), 5);
  cache.IsDuplicate(Ipv4Address("3.3.3.3"), 6);
  NS_TEST_EXPECT_MSG_EQ(cache.GetSize(), 6, "trivial");

  Simulator::Schedule(Seconds(5), &IdCacheTest::CheckTimeout1, this);
  Simulator::Schedule(Seconds(11), &IdCacheTest::CheckTimeout2, this);
  Simulator::Schedule(Seconds(30), &IdCacheTest::CheckTimeout3, this);
  Simulator::Run();
  Simulator::Destroy();
}

void IdCacheTest::CheckTimeout1() {
  NS_TEST_EXPECT_MSG_EQ(cache.GetSize(), 6, "Nothing expire");
}

void IdCacheTest::CheckTimeout2() {
  NS_TEST_EXPECT_MSG_EQ(cache.GetSize(), 3, "3 records left");
}

void IdCacheTest::CheckTimeout3() {
  NS_TEST_EXPECT_MSG_EQ(cache.GetSize(), 0, "All records expire");
}

class IdCacheTestSuite : public TestSuite {
public:
  IdCacheTestSuite() : TestSuite("aodv-routing-id-cache", UNIT) {
    AddTestCase(new IdCacheTest, TestCase::QUICK);
  }
} g_idCacheTestSuite;

} // namespace aodv
} // namespace ns3
