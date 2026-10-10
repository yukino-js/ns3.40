
#include "lte-test-rlc-um-e2e.h"

#include "lte-simple-helper.h"
#include "lte-test-entities.h"

#include "ns3/config.h"
#include "ns3/error-model.h"
#include "ns3/log.h"
#include "ns3/lte-rlc-header.h"
#include "ns3/lte-rlc-um.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteRlcUmE2eTest");

LteRlcUmE2eTestSuite::LteRlcUmE2eTestSuite()
    : TestSuite("lte-rlc-um-e2e", SYSTEM) {

  double losses[] = {0.0, 0.10, 0.25, 0.50, 0.75, 0.90, 1.00};
  uint32_t seeds[] = {1111, 2222, 3333, 4444, 5555,
                      6666, 7777, 8888, 9999, 10101};

  for (uint32_t l = 0; l < (sizeof(losses) / sizeof(double)); l++) {
    for (uint32_t s = 0; s < (sizeof(seeds) / sizeof(uint32_t)); s++) {
      std::ostringstream name;
      name << " Losses = " << losses[l] << "%. Seed = " << seeds[s];
      TestCase::TestDuration testDuration;
      if (l == 1 && s == 0) {
        testDuration = TestCase::QUICK;
      } else {
        testDuration = TestCase::EXTENSIVE;
      }
      AddTestCase(new LteRlcUmE2eTestCase(name.str(), seeds[s], losses[l]),
                  testDuration);
    }
  }
}

static LteRlcUmE2eTestSuite lteRlcUmE2eTestSuite;

LteRlcUmE2eTestCase::LteRlcUmE2eTestCase(std::string name, uint32_t seed,
                                         double losses)
    : TestCase(name) {

  m_seed = seed;
  m_losses = losses;

  m_dlDrops = 0;
  m_ulDrops = 0;
}

LteRlcUmE2eTestCase::~LteRlcUmE2eTestCase() {}

void LteRlcUmE2eTestCase::DlDropEvent(Ptr<const Packet> p) { m_dlDrops++; }

void LteRlcUmE2eTestCase::UlDropEvent(Ptr<const Packet> p) { m_ulDrops++; }

void LteRlcUmE2eTestCase::DoRun() {
  uint16_t numberOfNodes = 1;

  RngSeedManager::SetSeed(m_seed);

  Ptr<LteSimpleHelper> lteSimpleHelper = CreateObject<LteSimpleHelper>();

  lteSimpleHelper->SetAttribute("RlcEntity", StringValue("RlcUm"));

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);

  NetDeviceContainer enbLteDevs = lteSimpleHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteSimpleHelper->InstallUeDevice(ueNodes);

  Ptr<RateErrorModel> dlEm = CreateObject<RateErrorModel>();
  dlEm->SetAttribute("ErrorRate", DoubleValue(m_losses));
  dlEm->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

  Ptr<RateErrorModel> ulEm = CreateObject<RateErrorModel>();
  ulEm->SetAttribute("ErrorRate", DoubleValue(m_losses));
  ulEm->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

  ueLteDevs.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(dlEm));
  ueLteDevs.Get(0)->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&LteRlcUmE2eTestCase::DlDropEvent, this));
  enbLteDevs.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(ulEm));
  enbLteDevs.Get(0)->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&LteRlcUmE2eTestCase::UlDropEvent, this));

  lteSimpleHelper->m_enbRrc->SetArrivalTime(Seconds(0.010));
  lteSimpleHelper->m_enbRrc->SetPduSize(100);

  lteSimpleHelper->m_enbMac->SetTxOppSize(150);
  lteSimpleHelper->m_enbMac->SetTxOppTime(Seconds(0.005));
  lteSimpleHelper->m_enbMac->SetTxOpportunityMode(LteTestMac::RANDOM_MODE);

  lteSimpleHelper->m_ueRrc->SetArrivalTime(Seconds(0.010));
  lteSimpleHelper->m_ueRrc->SetPduSize(100);

  lteSimpleHelper->m_ueMac->SetTxOppSize(150);
  lteSimpleHelper->m_ueMac->SetTxOppTime(Seconds(0.005));
  lteSimpleHelper->m_ueMac->SetTxOpportunityMode(LteTestMac::RANDOM_MODE);

  Simulator::Schedule(Seconds(0.100), &LteTestRrc::Start,
                      lteSimpleHelper->m_enbRrc);
  Simulator::Schedule(Seconds(10.100), &LteTestRrc::Stop,
                      lteSimpleHelper->m_enbRrc);

  Simulator::Schedule(Seconds(20.100), &LteTestRrc::Start,
                      lteSimpleHelper->m_ueRrc);
  Simulator::Schedule(Seconds(30.100), &LteTestRrc::Stop,
                      lteSimpleHelper->m_ueRrc);

  Simulator::Stop(Seconds(31.000));
  Simulator::Run();

  uint32_t txEnbRrcPdus = lteSimpleHelper->m_enbRrc->GetTxPdus();
  uint32_t rxUeRrcPdus = lteSimpleHelper->m_ueRrc->GetRxPdus();

  uint32_t txUeRrcPdus = lteSimpleHelper->m_ueRrc->GetTxPdus();
  uint32_t rxEnbRrcPdus = lteSimpleHelper->m_enbRrc->GetRxPdus();

  NS_LOG_INFO(m_seed << "\t" << m_losses << "\t" << txEnbRrcPdus << "\t"
                     << rxUeRrcPdus << "\t" << m_dlDrops);
  NS_LOG_INFO(m_seed << "\t" << m_losses << "\t" << txUeRrcPdus << "\t"
                     << rxEnbRrcPdus << "\t" << m_ulDrops);

  NS_TEST_ASSERT_MSG_EQ(txEnbRrcPdus, rxUeRrcPdus + m_dlDrops,
                        "Downlink: TX PDUs ("
                            << txEnbRrcPdus << ") != RX PDUs (" << rxUeRrcPdus
                            << ") + DROPS (" << m_dlDrops << ")");
  NS_TEST_ASSERT_MSG_EQ(txUeRrcPdus, rxEnbRrcPdus + m_ulDrops,
                        "Uplink: TX PDUs (" << txUeRrcPdus << ") != RX PDUs ("
                                            << rxEnbRrcPdus << ") + DROPS ("
                                            << m_ulDrops << ")");

  Simulator::Destroy();
}
