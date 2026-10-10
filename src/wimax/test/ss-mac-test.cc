#include "ns3/log.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/wimax-helper.h"

using namespace ns3;

class Ns3WimaxNetworkEntryTestCase : public TestCase {
public:
  Ns3WimaxNetworkEntryTestCase();
  ~Ns3WimaxNetworkEntryTestCase() override;

private:
  void DoRun() override;
};

Ns3WimaxNetworkEntryTestCase::Ns3WimaxNetworkEntryTestCase()
    : TestCase("Test the network entry procedure") {}

Ns3WimaxNetworkEntryTestCase::~Ns3WimaxNetworkEntryTestCase() {}

void Ns3WimaxNetworkEntryTestCase::DoRun() {
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
  NodeContainer ssNodes;
  NodeContainer bsNodes;

  ssNodes.Create(10);
  bsNodes.Create(1);

  WimaxHelper wimax;

  NetDeviceContainer ssDevs;
  NetDeviceContainer bsDevs;

  ssDevs = wimax.Install(ssNodes, WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
  bsDevs = wimax.Install(bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
  Simulator::Stop(Seconds(1));
  Simulator::Run();
  for (int i = 0; i < 10; i++) {
    NS_TEST_EXPECT_MSG_EQ(
        ssDevs.Get(i)->GetObject<SubscriberStationNetDevice>()->IsRegistered(),
        true, "SS[" << i << "] IsNotRegistered");
  }
  Simulator::Destroy();
}

class Ns3WimaxManagementConnectionsTestCase : public TestCase {
public:
  Ns3WimaxManagementConnectionsTestCase();
  ~Ns3WimaxManagementConnectionsTestCase() override;

private:
  void DoRun() override;
};

Ns3WimaxManagementConnectionsTestCase::Ns3WimaxManagementConnectionsTestCase()
    : TestCase("Test if the management connections are correctly setup") {}

Ns3WimaxManagementConnectionsTestCase::
    ~Ns3WimaxManagementConnectionsTestCase() {}

void Ns3WimaxManagementConnectionsTestCase::DoRun() {
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
  NodeContainer ssNodes;
  NodeContainer bsNodes;

  ssNodes.Create(10);
  bsNodes.Create(1);

  WimaxHelper wimax;

  NetDeviceContainer ssDevs;
  NetDeviceContainer bsDevs;

  ssDevs = wimax.Install(ssNodes, WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
  bsDevs = wimax.Install(bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
  Simulator::Stop(Seconds(1));
  Simulator::Run();
  for (int i = 0; i < 10; i++) {
    NS_TEST_EXPECT_MSG_EQ(
        ssDevs.Get(i)
            ->GetObject<SubscriberStationNetDevice>()
            ->GetAreManagementConnectionsAllocated(),
        true, "Management connections for SS[" << i << "] are not allocated");
  }
  Simulator::Destroy();
}

class Ns3WimaxSSMacTestSuite : public TestSuite {
public:
  Ns3WimaxSSMacTestSuite();
};

Ns3WimaxSSMacTestSuite::Ns3WimaxSSMacTestSuite()
    : TestSuite("wimax-ss-mac-layer", UNIT) {
  AddTestCase(new Ns3WimaxNetworkEntryTestCase, TestCase::QUICK);
  AddTestCase(new Ns3WimaxManagementConnectionsTestCase, TestCase::QUICK);
}

static Ns3WimaxSSMacTestSuite ns3WimaxSSMacTestSuite;
