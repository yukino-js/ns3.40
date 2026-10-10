#include "ns3/log.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/simulator.h"
#include "ns3/snr-to-block-error-rate-manager.h"
#include "ns3/test.h"
#include "ns3/wimax-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WimaxPhyTest");

class Ns3WimaxSimpleOFDMTestCase : public TestCase {
public:
  Ns3WimaxSimpleOFDMTestCase();
  ~Ns3WimaxSimpleOFDMTestCase() override;

private:
  void DoRun() override;
  bool DoRunOnce(double FrameDuration);
};

Ns3WimaxSimpleOFDMTestCase::Ns3WimaxSimpleOFDMTestCase()
    : TestCase("Test the Phy model with different frame durations") {}

Ns3WimaxSimpleOFDMTestCase::~Ns3WimaxSimpleOFDMTestCase() {}

bool Ns3WimaxSimpleOFDMTestCase::DoRunOnce(double FrameDuration) {
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
  NodeContainer ssNodes;
  NodeContainer bsNodes;
  ssNodes.Create(3);
  bsNodes.Create(1);

  WimaxHelper wimax;

  NetDeviceContainer ssDevs;
  NetDeviceContainer bsDevs;

  ssDevs = wimax.Install(ssNodes, WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler,
                         FrameDuration);
  bsDevs = wimax.Install(bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION,
                         WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler,
                         FrameDuration);

  Simulator::Stop(Seconds(1));
  Simulator::Run();
  for (int i = 0; i < 3; i++) {
    if (!ssDevs.Get(i)
             ->GetObject<SubscriberStationNetDevice>()
             ->IsRegistered()) {
      NS_LOG_DEBUG("SS[" << i << "] not registered");
      return true;
    }
  }
  Simulator::Destroy();
  return (false);
}

void Ns3WimaxSimpleOFDMTestCase::DoRun() {
  double frameDuratioTab[7] = {0.0025, 0.004, 0.005, 0.008, 0.01, 0.0125, 0.02};
  for (int i = 0; i < 7; i++) {
    NS_LOG_DEBUG("Frame Duration = " << frameDuratioTab[i]);
    if (DoRunOnce(frameDuratioTab[i])) {
      return;
    }
  }
}

class Ns3WimaxSNRtoBLERTestCase : public TestCase {
public:
  Ns3WimaxSNRtoBLERTestCase();
  ~Ns3WimaxSNRtoBLERTestCase() override;

private:
  void DoRun() override;
  bool DoRunOnce(uint8_t modulationType);
};

Ns3WimaxSNRtoBLERTestCase::Ns3WimaxSNRtoBLERTestCase()
    : TestCase("Test the SNR to block error rate module") {}

Ns3WimaxSNRtoBLERTestCase::~Ns3WimaxSNRtoBLERTestCase() {}

bool Ns3WimaxSNRtoBLERTestCase::DoRunOnce(uint8_t modulationType) {
  SNRToBlockErrorRateManager l_SNRToBlockErrorRateManager;
  l_SNRToBlockErrorRateManager.LoadTraces();
  SNRToBlockErrorRateRecord *BLERRec;

  for (double i = -5; i < 40; i += 0.1) {
    BLERRec = l_SNRToBlockErrorRateManager.GetSNRToBlockErrorRateRecord(
        i, modulationType);
    delete BLERRec;
  }
  return false;
}

void Ns3WimaxSNRtoBLERTestCase::DoRun() {
  for (int i = 0; i < 7; i++) {
    DoRunOnce(i);
  }
}

class Ns3WimaxPhyTestSuite : public TestSuite {
public:
  Ns3WimaxPhyTestSuite();
};

Ns3WimaxPhyTestSuite::Ns3WimaxPhyTestSuite()
    : TestSuite("wimax-phy-layer", UNIT) {
  AddTestCase(new Ns3WimaxSNRtoBLERTestCase, TestCase::QUICK);
  AddTestCase(new Ns3WimaxSimpleOFDMTestCase, TestCase::QUICK);
}

static Ns3WimaxPhyTestSuite ns3WimaxPhyTestSuite;
