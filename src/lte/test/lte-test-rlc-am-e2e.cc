
#include "lte-test-rlc-am-e2e.h"

#include "lte-simple-helper.h"
#include "lte-test-entities.h"

#include "ns3/config-store.h"
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

NS_LOG_COMPONENT_DEFINE("LteRlcAmE2eTest");

LteRlcAmE2eTestSuite::LteRlcAmE2eTestSuite()
    : TestSuite("lte-rlc-am-e2e", SYSTEM) {

  double losses[] = {0.0, 0.05, 0.10, 0.15, 0.25, 0.50, 0.75, 0.90, 0.95};
  uint32_t runs[] = {
      1111,  2222,  3333,  4444,  5555,  6666,  7777,  8888,  9999,  11110,
      12221, 13332, 14443, 15554, 16665, 17776, 18887, 19998, 21109, 22220,
      23331, 24442, 25553, 26664, 27775, 28886, 29997, 31108, 32219, 33330,
  };

  for (uint32_t l = 0; l < (sizeof(losses) / sizeof(double)); l++) {
    for (uint32_t s = 0; s < (sizeof(runs) / sizeof(uint32_t)); s++) {
      for (uint32_t sduArrivalType = 0; sduArrivalType <= 1; ++sduArrivalType) {
        std::ostringstream name;
        name << " losses = " << losses[l] * 100 << "%; run = " << runs[s];

        bool bulkSduArrival;
        switch (sduArrivalType) {
        case 0:
          bulkSduArrival = false;
          name << "; continuous SDU arrival";
          break;
        case 1:
          bulkSduArrival = true;
          name << "; bulk SDU arrival";
          break;
        default:
          NS_FATAL_ERROR("unsupported option");
          break;
        }

        TestCase::TestDuration testDuration;
        if (l == 1 && s == 0) {
          testDuration = TestCase::QUICK;
        } else if (s <= 4) {
          testDuration = TestCase::EXTENSIVE;
        } else {
          testDuration = TestCase::TAKES_FOREVER;
        }
        AddTestCase(new LteRlcAmE2eTestCase(name.str(), runs[s], losses[l],
                                            bulkSduArrival),
                    testDuration);
      }
    }
  }
}

static LteRlcAmE2eTestSuite lteRlcAmE2eTestSuite;

LteRlcAmE2eTestCase::LteRlcAmE2eTestCase(std::string name, uint32_t run,
                                         double losses, bool bulkSduArrival)
    : TestCase(name), m_run(run), m_losses(losses),
      m_bulkSduArrival(bulkSduArrival), m_dlDrops(0), m_ulDrops(0) {
  NS_LOG_INFO("Creating LteRlcAmTestingTestCase: " + name);
}

LteRlcAmE2eTestCase::~LteRlcAmE2eTestCase() {}

void LteRlcAmE2eTestCase::DlDropEvent(Ptr<const Packet> p) { m_dlDrops++; }

void LteRlcAmE2eTestCase::UlDropEvent(Ptr<const Packet> p) { m_ulDrops++; }

void LteRlcAmE2eTestCase::DoRun() {
  uint16_t numberOfNodes = 1;

  Config::SetGlobal("RngRun", UintegerValue(m_run));
  Config::SetDefault("ns3::LteRlcAm::PollRetransmitTimer",
                     TimeValue(MilliSeconds(20)));
  Config::SetDefault("ns3::LteRlcAm::ReorderingTimer",
                     TimeValue(MilliSeconds(10)));
  Config::SetDefault("ns3::LteRlcAm::StatusProhibitTimer",
                     TimeValue(MilliSeconds(40)));
  Config::SetDefault("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue(0));

  Ptr<LteSimpleHelper> lteSimpleHelper = CreateObject<LteSimpleHelper>();

  lteSimpleHelper->SetAttribute("RlcEntity", StringValue("RlcAm"));

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);

  NetDeviceContainer enbLteDevs = lteSimpleHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteSimpleHelper->InstallUeDevice(ueNodes);

  Ptr<RateErrorModel> dlEm = CreateObject<RateErrorModel>();
  dlEm->AssignStreams(3);
  dlEm->SetAttribute("ErrorRate", DoubleValue(m_losses));
  dlEm->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

  ueLteDevs.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(dlEm));
  ueLteDevs.Get(0)->TraceConnectWithoutContext(
      "PhyRxDrop", MakeCallback(&LteRlcAmE2eTestCase::DlDropEvent, this));

  uint32_t sduSizeBytes = 100;
  uint32_t numSdu = 1000;
  double sduStartTimeSeconds = 0.100;
  double sduStopTimeSeconds;
  double sduArrivalTimeSeconds;
  uint32_t dlTxOppSizeBytes = 150;
  double dlTxOpprTimeSeconds = 0.003;
  uint32_t ulTxOppSizeBytes = 140;
  double ulTxOpprTimeSeconds = 0.003;

  if (m_bulkSduArrival) {
    sduStopTimeSeconds = sduStartTimeSeconds + 0.010;
  } else {
    sduStopTimeSeconds = sduStartTimeSeconds + 10;
  }
  sduArrivalTimeSeconds = (sduStopTimeSeconds - sduStartTimeSeconds) / numSdu;

  lteSimpleHelper->m_enbRrc->SetArrivalTime(Seconds(sduArrivalTimeSeconds));
  lteSimpleHelper->m_enbRrc->SetPduSize(sduSizeBytes);

  lteSimpleHelper->m_enbMac->SetTxOppSize(dlTxOppSizeBytes);
  lteSimpleHelper->m_enbMac->SetTxOppTime(Seconds(dlTxOpprTimeSeconds));
  lteSimpleHelper->m_enbMac->SetTxOpportunityMode(LteTestMac::AUTOMATIC_MODE);

  lteSimpleHelper->m_ueMac->SetTxOppSize(ulTxOppSizeBytes);
  lteSimpleHelper->m_ueMac->SetTxOppTime(Seconds(ulTxOpprTimeSeconds));
  lteSimpleHelper->m_ueMac->SetTxOpportunityMode(LteTestMac::AUTOMATIC_MODE);

  Simulator::Schedule(Seconds(sduStartTimeSeconds), &LteTestRrc::Start,
                      lteSimpleHelper->m_enbRrc);
  Simulator::Schedule(Seconds(sduStopTimeSeconds), &LteTestRrc::Stop,
                      lteSimpleHelper->m_enbRrc);

  double maxDlThroughput = (dlTxOppSizeBytes / (dlTxOppSizeBytes + 4.0)) *
                           (dlTxOppSizeBytes / dlTxOpprTimeSeconds) *
                           (1.0 - m_losses);
  const double statusProhibitSeconds = 0.020;
  double pollFrequency = (1.0 / dlTxOpprTimeSeconds) * (1 - m_losses);
  double statusFrequency = std::min(pollFrequency, 1.0 / statusProhibitSeconds);
  const uint32_t numNackSnPerStatusPdu = (ulTxOppSizeBytes * 8 - 14) / 10;
  double maxRetxThroughput =
      ((double)numNackSnPerStatusPdu * (double)dlTxOppSizeBytes) *
      statusFrequency;
  double throughput = std::min(maxDlThroughput, maxRetxThroughput);
  double totBytes =
      ((sduSizeBytes) * (sduStopTimeSeconds - sduStartTimeSeconds) /
       sduArrivalTimeSeconds);

  Time margin;
  if (m_losses < 0.07) {
    margin = Seconds(0.500);
  } else if (m_losses < 0.20) {
    margin = Seconds(1);
  } else if (m_losses < 0.50) {
    margin = Seconds(2);
  } else if (m_losses < 0.70) {
    margin = Seconds(10);
  } else if (m_losses < 0.91) {
    margin = Seconds(20);
  } else {
    margin = Seconds(30);
  }
  Time stopTime = Seconds(std::max(sduStartTimeSeconds + totBytes / throughput,
                                   sduStopTimeSeconds)) +
                  margin;

  NS_LOG_INFO("statusFrequency=" << statusFrequency
                                 << ", maxDlThroughput=" << maxDlThroughput
                                 << ", maxRetxThroughput=" << maxRetxThroughput
                                 << ", totBytes=" << totBytes
                                 << ", stopTime=" << stopTime.As(Time::S));

  Simulator::Stop(stopTime);
  Simulator::Run();

  uint32_t txEnbRrcPdus = lteSimpleHelper->m_enbRrc->GetTxPdus();
  uint32_t rxUeRrcPdus = lteSimpleHelper->m_ueRrc->GetRxPdus();

  uint32_t txEnbRlcPdus = lteSimpleHelper->m_enbMac->GetTxPdus();
  uint32_t rxUeRlcPdus = lteSimpleHelper->m_ueMac->GetRxPdus();

  NS_LOG_INFO("Run = " << m_run);
  NS_LOG_INFO("Loss rate (%) = " << uint32_t(m_losses * 100));

  NS_LOG_INFO("RLC PDUs   TX: " << txEnbRlcPdus << "   RX: " << rxUeRlcPdus
                                << "   LOST: " << m_dlDrops << " ("
                                << (100.0 * (double)m_dlDrops) / txEnbRlcPdus
                                << "%)");

  NS_TEST_ASSERT_MSG_EQ(txEnbRlcPdus, rxUeRlcPdus + m_dlDrops,
                        "lost RLC PDUs don't match TX + RX");

  NS_LOG_INFO("eNB tx RRC count = " << txEnbRrcPdus);
  NS_LOG_INFO("UE rx RRC count = " << rxUeRrcPdus);

  NS_TEST_ASSERT_MSG_EQ(txEnbRrcPdus, rxUeRrcPdus,
                        "TX PDUs (" << txEnbRrcPdus << ") != RX PDUs ("
                                    << rxUeRrcPdus << ")");

  Simulator::Destroy();
}
