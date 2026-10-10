#include "ns3/internet-stack-helper.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/log.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/service-flow.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/wimax-helper.h"

using namespace ns3;

class Ns3WimaxSchedulingTestCase : public TestCase {
public:
  Ns3WimaxSchedulingTestCase();
  ~Ns3WimaxSchedulingTestCase() override;

private:
  void DoRun() override;
  bool DoRunOnce(WimaxHelper::SchedulerType scheduler);
};

Ns3WimaxSchedulingTestCase::Ns3WimaxSchedulingTestCase()
    : TestCase("Test the 2 different schedulers") {}

Ns3WimaxSchedulingTestCase::~Ns3WimaxSchedulingTestCase() {}

bool Ns3WimaxSchedulingTestCase::DoRunOnce(
    WimaxHelper::SchedulerType scheduler) {
  NodeContainer ssNodes;
  NodeContainer bsNodes;
  ssNodes.Create(2);
  bsNodes.Create(1);
  WimaxHelper wimax;
  NetDeviceContainer ssDevs;
  NetDeviceContainer bsDevs;
  ssDevs = wimax.Install(ssNodes, WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
  bsDevs = wimax.Install(bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
  ssDevs.Get(0)->GetObject<SubscriberStationNetDevice>()->SetModulationType(
      WimaxPhy::MODULATION_TYPE_QAM16_12);
  ssDevs.Get(1)->GetObject<SubscriberStationNetDevice>()->SetModulationType(
      WimaxPhy::MODULATION_TYPE_QAM16_12);

  InternetStackHelper stack;
  stack.Install(bsNodes);
  stack.Install(ssNodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer SSinterfaces = address.Assign(ssDevs);
  Ipv4InterfaceContainer BSinterface = address.Assign(bsDevs);

  Simulator::Stop(Seconds(2 + 0.1));

  IpcsClassifierRecord DlClassifierUgs(
      Ipv4Address("0.0.0.0"), Ipv4Mask("0.0.0.0"), SSinterfaces.GetAddress(0),
      Ipv4Mask("255.255.255.255"), 0, 65000, 100, 100, 17, 1);
  ServiceFlow DlServiceFlowUgs =
      wimax.CreateServiceFlow(ServiceFlow::SF_DIRECTION_DOWN,
                              ServiceFlow::SF_TYPE_RTPS, DlClassifierUgs);
  IpcsClassifierRecord UlClassifierUgs(
      SSinterfaces.GetAddress(1), Ipv4Mask("255.255.255.255"),
      Ipv4Address("0.0.0.0"), Ipv4Mask("0.0.0.0"), 0, 65000, 100, 100, 17, 1);
  ServiceFlow UlServiceFlowUgs = wimax.CreateServiceFlow(
      ServiceFlow::SF_DIRECTION_UP, ServiceFlow::SF_TYPE_RTPS, UlClassifierUgs);
  ssDevs.Get(0)->GetObject<SubscriberStationNetDevice>()->AddServiceFlow(
      DlServiceFlowUgs);
  ssDevs.Get(1)->GetObject<SubscriberStationNetDevice>()->AddServiceFlow(
      UlServiceFlowUgs);

  Simulator::Run();
  Simulator::Destroy();
  return false;
}

void Ns3WimaxSchedulingTestCase::DoRun() {
  if (DoRunOnce(WimaxHelper::SCHED_TYPE_SIMPLE)) {
    return;
  }
  if (DoRunOnce(WimaxHelper::SCHED_TYPE_RTPS)) {
    return;
  }
}

class Ns3WimaxSFTypeTestCase : public TestCase {
public:
  Ns3WimaxSFTypeTestCase();
  ~Ns3WimaxSFTypeTestCase() override;

private:
  void DoRun() override;
  bool DoRunOnce(ServiceFlow::SchedulingType schedulingType);
};

Ns3WimaxSFTypeTestCase::Ns3WimaxSFTypeTestCase()
    : TestCase("Test the service flow scheduling types") {}

Ns3WimaxSFTypeTestCase::~Ns3WimaxSFTypeTestCase() {}

bool Ns3WimaxSFTypeTestCase::DoRunOnce(
    ServiceFlow::SchedulingType schedulingType) {
  NodeContainer ssNodes;
  NodeContainer bsNodes;
  ssNodes.Create(2);
  bsNodes.Create(1);
  WimaxHelper wimax;
  NetDeviceContainer ssDevs;
  NetDeviceContainer bsDevs;
  ssDevs = wimax.Install(ssNodes, WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                         WimaxHelper::SCHED_TYPE_SIMPLE);
  bsDevs = wimax.Install(bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                         WimaxHelper::SCHED_TYPE_SIMPLE);
  ssDevs.Get(0)->GetObject<SubscriberStationNetDevice>()->SetModulationType(
      WimaxPhy::MODULATION_TYPE_QAM16_12);
  ssDevs.Get(1)->GetObject<SubscriberStationNetDevice>()->SetModulationType(
      WimaxPhy::MODULATION_TYPE_QAM16_12);

  InternetStackHelper stack;
  stack.Install(bsNodes);
  stack.Install(ssNodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer SSinterfaces = address.Assign(ssDevs);
  Ipv4InterfaceContainer BSinterface = address.Assign(bsDevs);

  Simulator::Stop(Seconds(2 + 0.1));

  IpcsClassifierRecord DlClassifier(
      Ipv4Address("0.0.0.0"), Ipv4Mask("0.0.0.0"), SSinterfaces.GetAddress(0),
      Ipv4Mask("255.255.255.255"), 0, 65000, 100, 100, 17, 1);
  ServiceFlow DlServiceFlow = wimax.CreateServiceFlow(
      ServiceFlow::SF_DIRECTION_DOWN, schedulingType, DlClassifier);
  IpcsClassifierRecord UlClassifier(
      SSinterfaces.GetAddress(1), Ipv4Mask("255.255.255.255"),
      Ipv4Address("0.0.0.0"), Ipv4Mask("0.0.0.0"), 0, 65000, 100, 100, 17, 1);
  ServiceFlow UlServiceFlow = wimax.CreateServiceFlow(
      ServiceFlow::SF_DIRECTION_UP, schedulingType, UlClassifier);
  ssDevs.Get(0)->GetObject<SubscriberStationNetDevice>()->AddServiceFlow(
      DlServiceFlow);
  ssDevs.Get(1)->GetObject<SubscriberStationNetDevice>()->AddServiceFlow(
      UlServiceFlow);

  Simulator::Run();
  Simulator::Destroy();
  return false;
}

void Ns3WimaxSFTypeTestCase::DoRun() {
  if (DoRunOnce(ServiceFlow::SF_TYPE_UGS)) {
    return;
  }
  if (DoRunOnce(ServiceFlow::SF_TYPE_RTPS)) {
    return;
  }
  if (DoRunOnce(ServiceFlow::SF_TYPE_BE)) {
    return;
  }
}

class Ns3WimaxQoSTestSuite : public TestSuite {
public:
  Ns3WimaxQoSTestSuite();
};

Ns3WimaxQoSTestSuite::Ns3WimaxQoSTestSuite() : TestSuite("wimax-qos", SYSTEM) {
  AddTestCase(new Ns3WimaxSFTypeTestCase, TestCase::QUICK);
  AddTestCase(new Ns3WimaxSchedulingTestCase, TestCase::QUICK);
}

static Ns3WimaxQoSTestSuite ns3WimaxQoSTestSuite;
