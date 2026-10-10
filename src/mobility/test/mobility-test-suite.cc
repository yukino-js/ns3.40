

#include "ns3/boolean.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/scheduler.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/vector.h"
#include "ns3/waypoint-mobility-model.h"

using namespace ns3;

class WaypointLazyNotifyFalse : public TestCase {
public:
  WaypointLazyNotifyFalse();
  ~WaypointLazyNotifyFalse() override;

private:
  void TestXPosition(double expectedXPos);
  void CourseChangeCallback(std::string path, Ptr<const MobilityModel> model);
  void DoRun() override;
  Ptr<Node> m_node;
  Ptr<WaypointMobilityModel> m_mob;
  int m_courseChanges;
};

WaypointLazyNotifyFalse::WaypointLazyNotifyFalse()
    : TestCase("Test behavior when LazyNotify is false"), m_courseChanges(0) {}

WaypointLazyNotifyFalse::~WaypointLazyNotifyFalse() {}

void WaypointLazyNotifyFalse::TestXPosition(double expectedXPos) {
  Vector pos = m_mob->GetPosition();
  NS_TEST_EXPECT_MSG_EQ_TOL(pos.x, expectedXPos, 0.001, "Position not equal");
}

void WaypointLazyNotifyFalse::CourseChangeCallback(
    std::string path, Ptr<const MobilityModel> model) {
  NS_TEST_EXPECT_MSG_EQ_TOL(m_courseChanges * 10.0,
                            Simulator::Now().GetSeconds(), 0.001,
                            "Course change not notified correctly");
  m_courseChanges++;
}

void WaypointLazyNotifyFalse::DoRun() {
  m_node = CreateObject<Node>();
  m_mob = CreateObject<WaypointMobilityModel>();
  m_node->AggregateObject(m_mob);
  Waypoint wpt(Seconds(0.0), Vector(0.0, 0.0, 0.0));
  m_mob->AddWaypoint(wpt);
  Waypoint wpt2(Seconds(10.0), Vector(10.0, 10.0, 10.0));
  m_mob->AddWaypoint(wpt2);
  Waypoint wpt3(Seconds(20.0), Vector(20.0, 20.0, 20.0));
  m_mob->AddWaypoint(wpt3);

  Simulator::Schedule(Seconds(5.0), &WaypointLazyNotifyFalse::TestXPosition,
                      this, 5);
  Simulator::Run();
  Simulator::Destroy();
}

class WaypointLazyNotifyTrue : public TestCase {
public:
  WaypointLazyNotifyTrue();
  ~WaypointLazyNotifyTrue() override;

private:
  void TestXPosition(double expectedXPos);
  void CourseChangeCallback(std::string path, Ptr<const MobilityModel> model);
  void DoRun() override;
  Ptr<Node> m_node;
  Ptr<WaypointMobilityModel> m_mob;
};

WaypointLazyNotifyTrue::WaypointLazyNotifyTrue()
    : TestCase("Test behavior when LazyNotify is true") {}

WaypointLazyNotifyTrue::~WaypointLazyNotifyTrue() {}

void WaypointLazyNotifyTrue::TestXPosition(double expectedXPos) {
  Vector pos = m_mob->GetPosition();
  NS_TEST_EXPECT_MSG_EQ_TOL(pos.x, expectedXPos, 0.001, "Position not equal");
}

void WaypointLazyNotifyTrue::CourseChangeCallback(
    std::string path, Ptr<const MobilityModel> model) {
  NS_TEST_EXPECT_MSG_EQ_TOL(15, Simulator::Now().GetSeconds(), 0.001,
                            "Course change not notified correctly");
}

void WaypointLazyNotifyTrue::DoRun() {
  m_node = CreateObject<Node>();
  m_mob = CreateObject<WaypointMobilityModel>();
  m_mob->SetAttributeFailSafe("LazyNotify", BooleanValue(true));
  m_node->AggregateObject(m_mob);
  Waypoint wpt(Seconds(0.0), Vector(0.0, 0.0, 0.0));
  m_mob->AddWaypoint(wpt);
  Waypoint wpt2(Seconds(10.0), Vector(10.0, 10.0, 10.0));
  m_mob->AddWaypoint(wpt2);
  Waypoint wpt3(Seconds(20.0), Vector(20.0, 20.0, 20.0));
  m_mob->AddWaypoint(wpt3);

  Simulator::Schedule(Seconds(15.0), &WaypointLazyNotifyTrue::TestXPosition,
                      this, 15);
  Simulator::Run();
  Simulator::Destroy();
}

class WaypointInitialPositionIsWaypoint : public TestCase {
public:
  WaypointInitialPositionIsWaypoint();
  ~WaypointInitialPositionIsWaypoint() override;

private:
  void TestXPosition(Ptr<const WaypointMobilityModel> model,
                     double expectedXPos);
  void TestNumWaypoints(Ptr<const WaypointMobilityModel> model, uint32_t num);
  void DoRun() override;
  Ptr<WaypointMobilityModel> m_mob1;
  Ptr<WaypointMobilityModel> m_mob2;
  Ptr<WaypointMobilityModel> m_mob3;
  Ptr<WaypointMobilityModel> m_mob4;
  Ptr<WaypointMobilityModel> m_mob5;
};

WaypointInitialPositionIsWaypoint::WaypointInitialPositionIsWaypoint()
    : TestCase("Test behavior of Waypoint InitialPositionIsWaypoint") {}

WaypointInitialPositionIsWaypoint::~WaypointInitialPositionIsWaypoint() {}

void WaypointInitialPositionIsWaypoint::TestXPosition(
    Ptr<const WaypointMobilityModel> model, double expectedXPos) {
  Vector pos = model->GetPosition();
  NS_TEST_EXPECT_MSG_EQ_TOL(pos.x, expectedXPos, 0.001, "Position not equal");
}

void WaypointInitialPositionIsWaypoint::TestNumWaypoints(
    Ptr<const WaypointMobilityModel> model, uint32_t num) {
  NS_TEST_EXPECT_MSG_EQ(model->WaypointsLeft(), num,
                        "Unexpected number of waypoints left");
}

void WaypointInitialPositionIsWaypoint::DoRun() {
  m_mob1 = CreateObject<WaypointMobilityModel>();
  m_mob1->SetAttributeFailSafe("InitialPositionIsWaypoint",
                               BooleanValue(false));
  m_mob1->SetPosition(Vector(10.0, 10.0, 10.0));
  Simulator::Schedule(Seconds(1.0),
                      &WaypointInitialPositionIsWaypoint::TestNumWaypoints,
                      this, m_mob1, 0);
  Simulator::Schedule(Seconds(15.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob1, 10.0);

  m_mob2 = CreateObject<WaypointMobilityModel>();
  m_mob2->SetAttributeFailSafe("InitialPositionIsWaypoint",
                               BooleanValue(false));
  Waypoint wpt21(Seconds(5.0), Vector(15.0, 15.0, 15.0));
  m_mob2->AddWaypoint(wpt21);
  Waypoint wpt22(Seconds(10.0), Vector(20.0, 20.0, 20.0));
  m_mob2->AddWaypoint(wpt22);
  m_mob2->SetPosition(Vector(10.0, 10.0, 10.0));
  Simulator::Schedule(Seconds(3.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob2, 10.0);
  Simulator::Schedule(Seconds(3.0),
                      &WaypointInitialPositionIsWaypoint::TestNumWaypoints,
                      this, m_mob2, 1);
  Simulator::Schedule(Seconds(8.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob2, 18.0);
  Simulator::Schedule(Seconds(8.0),
                      &WaypointInitialPositionIsWaypoint::TestNumWaypoints,
                      this, m_mob2, 0);

  m_mob3 = CreateObject<WaypointMobilityModel>();
  m_mob3->SetAttributeFailSafe("InitialPositionIsWaypoint", BooleanValue(true));
  m_mob3->SetPosition(Vector(10.0, 10.0, 10.0));
  Simulator::Schedule(Seconds(1.0),
                      &WaypointInitialPositionIsWaypoint::TestNumWaypoints,
                      this, m_mob3, 0);
  Simulator::Schedule(Seconds(15.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob3, 10.0);

  m_mob4 = CreateObject<WaypointMobilityModel>();
  m_mob4->SetAttributeFailSafe("InitialPositionIsWaypoint", BooleanValue(true));
  Waypoint wpt41(Seconds(5.0), Vector(15.0, 15.0, 15.0));
  m_mob4->AddWaypoint(wpt41);
  Waypoint wpt42(Seconds(10.0), Vector(20.0, 20.0, 20.0));
  m_mob4->AddWaypoint(wpt42);
  m_mob4->SetPosition(Vector(10.0, 10.0, 10.0));
  Simulator::Schedule(Seconds(3.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob4, 10.0);
  Simulator::Schedule(Seconds(3.0),
                      &WaypointInitialPositionIsWaypoint::TestNumWaypoints,
                      this, m_mob4, 1);
  Simulator::Schedule(Seconds(6.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob4, 16.0);
  Simulator::Schedule(Seconds(15.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob4, 20.0);

  m_mob5 = CreateObject<WaypointMobilityModel>();
  m_mob5->SetAttributeFailSafe("InitialPositionIsWaypoint", BooleanValue(true));
  Waypoint wpt51(Seconds(0.0), Vector(200.0, 200.0, 200.0));
  m_mob5->AddWaypoint(wpt51);
  Waypoint wpt52(Seconds(5.0), Vector(15.0, 15.0, 15.0));
  m_mob5->AddWaypoint(wpt52);
  Waypoint wpt53(Seconds(10.0), Vector(20.0, 20.0, 20.0));
  m_mob5->AddWaypoint(wpt53);
  m_mob5->SetPosition(Vector(10.0, 10.0, 10.0));
  Simulator::Schedule(Seconds(3.0),
                      &WaypointInitialPositionIsWaypoint::TestXPosition, this,
                      m_mob5, 10.0);

  Simulator::Run();
  Simulator::Destroy();
}

class WaypointMobilityModelViaHelper : public TestCase {
public:
  WaypointMobilityModelViaHelper();
  ~WaypointMobilityModelViaHelper() override;

private:
  void TestXPosition(Ptr<const WaypointMobilityModel> mob, double expectedXPos);
  void DoRun() override;
};

WaypointMobilityModelViaHelper::WaypointMobilityModelViaHelper()
    : TestCase("Test behavior using MobilityHelper and PositionAllocator") {}

WaypointMobilityModelViaHelper::~WaypointMobilityModelViaHelper() {}

void WaypointMobilityModelViaHelper::TestXPosition(
    Ptr<const WaypointMobilityModel> mob, double expectedXPos) {
  Vector pos = mob->GetPosition();
  NS_TEST_EXPECT_MSG_EQ_TOL(pos.x, expectedXPos, 0.001, "Position not equal");
}

void WaypointMobilityModelViaHelper::DoRun() {
  NodeContainer c;
  c.Create(1);
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::WaypointMobilityModel",
                            "InitialPositionIsWaypoint", BooleanValue(false));
  mobility.Install(c);

  Ptr<WaypointMobilityModel> mob = c.Get(0)->GetObject<WaypointMobilityModel>();
  Waypoint wpt(Seconds(5.0), Vector(20.0, 20.0, 20.0));
  Waypoint wpt2(Seconds(10.0), Vector(10.0, 10.0, 10.0));
  mob->AddWaypoint(wpt);
  mob->AddWaypoint(wpt2);
  Simulator::Schedule(Seconds(3),
                      &WaypointMobilityModelViaHelper::TestXPosition, this, mob,
                      20);
  Simulator::Schedule(Seconds(7.5),
                      &WaypointMobilityModelViaHelper::TestXPosition, this, mob,
                      15);

  NodeContainer c2;
  c2.Create(1);
  MobilityHelper mobility2;
  Ptr<ListPositionAllocator> positionAlloc2 =
      CreateObject<ListPositionAllocator>();
  positionAlloc2->Add(Vector(0.0, 0.0, 0.0));
  mobility2.SetPositionAllocator(positionAlloc2);
  mobility2.SetMobilityModel("ns3::WaypointMobilityModel",
                             "InitialPositionIsWaypoint", BooleanValue(true));
  mobility2.Install(c2);
  Ptr<WaypointMobilityModel> mob2 =
      c2.Get(0)->GetObject<WaypointMobilityModel>();
  Waypoint wpt3(Seconds(5.0), Vector(20.0, 20.0, 20.0));
  mob2->AddWaypoint(wpt3);
  Simulator::Schedule(Seconds(3),
                      &WaypointMobilityModelViaHelper::TestXPosition, this,
                      mob2, 12);

  Simulator::Run();
  Simulator::Destroy();
}

class MobilityTestSuite : public TestSuite {
public:
  MobilityTestSuite();
};

MobilityTestSuite::MobilityTestSuite() : TestSuite("mobility", UNIT) {
  AddTestCase(new WaypointLazyNotifyFalse, TestCase::QUICK);
  AddTestCase(new WaypointLazyNotifyTrue, TestCase::QUICK);
  AddTestCase(new WaypointInitialPositionIsWaypoint, TestCase::QUICK);
  AddTestCase(new WaypointMobilityModelViaHelper, TestCase::QUICK);
}

static MobilityTestSuite mobilityTestSuite;
