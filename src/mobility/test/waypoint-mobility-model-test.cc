
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/waypoint-mobility-model.h"

using namespace ns3;

class WaypointMobilityModelNotifyTest : public TestCase {
public:
  WaypointMobilityModelNotifyTest(bool lazy)
      : TestCase(
            lazy ? "Check Waypoint Mobility Model LAZY notification accuracy"
                 : "Check Waypoint Mobility Model NON-LAZY notification "
                   "accuracy"),
        lazyNotify(lazy) {}

  ~WaypointMobilityModelNotifyTest() override {}

private:
  std::vector<Ptr<MobilityModel>> mobilityStack;
  uint32_t mobilityCount;
  uint32_t waypointCount;
  std::deque<Waypoint> waypoints;
  bool lazyNotify;

private:
  void DoRun() override;
  void DoTeardown() override;
  void ForceUpdates();
  void CourseChangeCallback(Ptr<const MobilityModel> model);
};

void WaypointMobilityModelNotifyTest::DoTeardown() {
  mobilityStack.clear();
  waypoints.clear();
}

void WaypointMobilityModelNotifyTest::DoRun() {
  mobilityCount = 1;
  waypointCount = 100;

  ObjectFactory mobilityFactory;
  mobilityFactory.SetTypeId("ns3::WaypointMobilityModel");
  mobilityFactory.Set("LazyNotify", BooleanValue(lazyNotify));

  for (uint32_t i = 0; i < mobilityCount; i++) {
    Ptr<MobilityModel> model =
        mobilityFactory.Create()->GetObject<MobilityModel>();

    mobilityStack.push_back(model);
    Simulator::Schedule(Seconds(0.0), &Object::Initialize, model);
  }

  Waypoint wpt(Seconds(0.0), Vector(0.0, 0.0, 0.0));

  for (uint32_t iw = 0; iw < waypointCount; ++iw) {
    wpt.time += Seconds(1.0);
    waypoints.push_back(wpt);
  }

  for (auto i = mobilityStack.begin(); i != mobilityStack.end(); ++i) {
    Ptr<WaypointMobilityModel> mob = (*i)->GetObject<WaypointMobilityModel>();
    mob->TraceConnectWithoutContext(
        "CourseChange",
        MakeCallback(&WaypointMobilityModelNotifyTest::CourseChangeCallback,
                     this));

    for (auto w = waypoints.begin(); w != waypoints.end(); ++w) {
      mob->AddWaypoint(*w);
    }
  }

  for (double updateTime = 0.5; updateTime <= ((double)waypointCount + 1.5);
       updateTime += 1.0) {
    Simulator::Schedule(Seconds(updateTime),
                        &WaypointMobilityModelNotifyTest::ForceUpdates, this);
  }

  Simulator::Stop(Seconds((double)waypointCount + 2.0));
  Simulator::Run();
  Simulator::Destroy();
}

void WaypointMobilityModelNotifyTest::ForceUpdates() {
  for (auto i = mobilityStack.begin(); i != mobilityStack.end(); ++i) {
    Ptr<WaypointMobilityModel> mob = (*i)->GetObject<WaypointMobilityModel>();
    mob->Update();
  }
}

void WaypointMobilityModelNotifyTest::CourseChangeCallback(
    Ptr<const MobilityModel> model) {
  const Time now = Simulator::Now();
  const double sec = now.GetSeconds();
  Ptr<const WaypointMobilityModel> mob =
      model->GetObject<WaypointMobilityModel>();

  NS_TEST_EXPECT_MSG_EQ(now, mob->m_current.time,
                        "Waypoint time not properly updated");

  if (!lazyNotify) {
    NS_TEST_EXPECT_MSG_EQ(sec - ((double)((int)sec)) + sec, sec,
                          "Course didn't change on one second time boundary "
                          "with NON-LAZY notifications");
  } else {
    NS_TEST_EXPECT_MSG_EQ(
        sec - ((double)((int)sec)), 0.5,
        "Course didn't change between waypoints with LAZY notifications");
  }
}

class WaypointMobilityModelAddWaypointTest : public TestCase {
public:
  WaypointMobilityModelAddWaypointTest()
      : TestCase("Check Waypoint Mobility Model waypoint add") {}

  ~WaypointMobilityModelAddWaypointTest() override {}

private:
  Ptr<MobilityModel> m_mobilityModel;
  uint32_t m_waypointCount;
  uint32_t m_waypointCounter;
  Waypoint m_nextWaypoint;

private:
  void DoRun() override;
  void DoTeardown() override;
  void CourseChangeCallback(Ptr<const MobilityModel> model);
};

void WaypointMobilityModelAddWaypointTest::DoTeardown() {
  m_mobilityModel = nullptr;
}

void WaypointMobilityModelAddWaypointTest::DoRun() {
  m_waypointCount = 10;
  m_waypointCounter = 1;

  ObjectFactory mobilityFactory;
  mobilityFactory.SetTypeId("ns3::WaypointMobilityModel");
  mobilityFactory.Set("LazyNotify", BooleanValue(false));

  m_mobilityModel = mobilityFactory.Create()->GetObject<MobilityModel>();
  m_mobilityModel->TraceConnectWithoutContext(
      "CourseChange",
      MakeCallback(&WaypointMobilityModelAddWaypointTest::CourseChangeCallback,
                   this));

  Simulator::Schedule(Seconds(0.0), &Object::Initialize, m_mobilityModel);

  Ptr<WaypointMobilityModel> mob =
      DynamicCast<WaypointMobilityModel>(m_mobilityModel);
  Waypoint m_nextWaypoint(Seconds(m_waypointCounter), Vector(0.0, 0.0, 0.0));
  mob->AddWaypoint(m_nextWaypoint);

  Simulator::Stop(Seconds((double)m_waypointCount + 2.0));
  Simulator::Run();
  Simulator::Destroy();
}

void WaypointMobilityModelAddWaypointTest::CourseChangeCallback(
    Ptr<const MobilityModel> model) {
  const Time now = Simulator::Now();
  Ptr<WaypointMobilityModel> mob =
      DynamicCast<WaypointMobilityModel>(m_mobilityModel);

  std::cout << now << " CourseChangeCallback" << std::endl;

  NS_TEST_EXPECT_MSG_EQ(now, Seconds(m_waypointCounter),
                        "Waypoint time not properly set");

  if (now < Seconds((double)m_waypointCount)) {
    m_waypointCounter++;
    m_nextWaypoint =
        Waypoint(Seconds(m_waypointCounter), Vector(0.0, 0.0, 0.0));
    mob->AddWaypoint(m_nextWaypoint);
  }
}

static struct WaypointMobilityModelTestSuite : public TestSuite {
  WaypointMobilityModelTestSuite()
      : TestSuite("waypoint-mobility-model", UNIT) {
    AddTestCase(new WaypointMobilityModelNotifyTest(true), TestCase::QUICK);
    AddTestCase(new WaypointMobilityModelNotifyTest(false), TestCase::QUICK);
    AddTestCase(new WaypointMobilityModelAddWaypointTest(), TestCase::QUICK);
  }
} g_waypointMobilityModelTestSuite;
