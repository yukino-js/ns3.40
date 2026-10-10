
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/ff-mac-scheduler.h"
#include "ns3/log.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-enb-phy.h"
#include "ns3/lte-global-pathloss-database.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/lte-ue-phy.h"
#include "ns3/mobility-helper.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include <ns3/lte-chunk-processor.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteAntennaTest");

class LteEnbAntennaTestCase : public TestCase {
public:
  static std::string BuildNameString(double orientationDegrees,
                                     double beamwidthDegrees, double x,
                                     double y);
  LteEnbAntennaTestCase(double orientationDegrees, double beamwidthDegrees,
                        double x, double y, double antennaGainDb);
  LteEnbAntennaTestCase();
  ~LteEnbAntennaTestCase() override;

private:
  void DoRun() override;

  double m_orientationDegrees;
  double m_beamwidthDegrees;
  double m_x;
  double m_y;
  double m_antennaGainDb;
};

std::string LteEnbAntennaTestCase::BuildNameString(double orientationDegrees,
                                                   double beamwidthDegrees,
                                                   double x, double y) {
  std::ostringstream oss;
  oss << "o=" << orientationDegrees << ", bw=" << beamwidthDegrees
      << ", x=" << x << ", y=" << y;
  return oss.str();
}

LteEnbAntennaTestCase::LteEnbAntennaTestCase(double orientationDegrees,
                                             double beamwidthDegrees, double x,
                                             double y, double antennaGainDb)
    : TestCase(BuildNameString(orientationDegrees, beamwidthDegrees, x, y)),
      m_orientationDegrees(orientationDegrees),
      m_beamwidthDegrees(beamwidthDegrees), m_x(x), m_y(y),
      m_antennaGainDb(antennaGainDb) {
  NS_LOG_FUNCTION(this);
}

LteEnbAntennaTestCase::~LteEnbAntennaTestCase() {}

void LteEnbAntennaTestCase::DoRun() {
  Config::Reset();
  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                     BooleanValue(false));
  Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                     BooleanValue(false));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));

  Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl",
                     BooleanValue(false));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

  lteHelper->SetAttribute(
      "PathlossModel",
      StringValue("ns3::ConstantSpectrumPropagationLossModel"));
  lteHelper->SetPathlossModelAttribute("Loss", DoubleValue(0.0));

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);
  NodeContainer allNodes = NodeContainer(enbNodes, ueNodes);

  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(m_x, m_y, 0.0));
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(allNodes);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
  lteHelper->SetSchedulerAttribute("UlCqiFilter",
                                   EnumValue(FfMacScheduler::PUSCH_UL_CQI));
  lteHelper->SetEnbAntennaModelType("ns3::CosineAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute("Orientation",
                                         DoubleValue(m_orientationDegrees));
  lteHelper->SetEnbAntennaModelAttribute("HorizontalBeamwidth",
                                         DoubleValue(m_beamwidthDegrees));
  lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue(0.0));

  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(25));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(25));

  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  EpsBearer::Qci q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
  EpsBearer bearer(q);
  lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  Ptr<LtePhy> uePhy =
      ueDevs.Get(0)->GetObject<LteUeNetDevice>()->GetPhy()->GetObject<LtePhy>();
  Ptr<LteChunkProcessor> testDlSinr = Create<LteChunkProcessor>();
  LteSpectrumValueCatcher dlSinrCatcher;
  testDlSinr->AddCallback(
      MakeCallback(&LteSpectrumValueCatcher::ReportValue, &dlSinrCatcher));
  uePhy->GetDownlinkSpectrumPhy()->AddDataSinrChunkProcessor(testDlSinr);

  Ptr<LtePhy> enbphy = enbDevs.Get(0)
                           ->GetObject<LteEnbNetDevice>()
                           ->GetPhy()
                           ->GetObject<LtePhy>();
  Ptr<LteChunkProcessor> testUlSinr = Create<LteChunkProcessor>();
  LteSpectrumValueCatcher ulSinrCatcher;
  testUlSinr->AddCallback(
      MakeCallback(&LteSpectrumValueCatcher::ReportValue, &ulSinrCatcher));
  enbphy->GetUplinkSpectrumPhy()->AddDataSinrChunkProcessor(testUlSinr);

  DownlinkLteGlobalPathlossDatabase dlPathlossDb;
  UplinkLteGlobalPathlossDatabase ulPathlossDb;
  Config::Connect(
      "/ChannelList/0/PathLoss",
      MakeCallback(&DownlinkLteGlobalPathlossDatabase::UpdatePathloss,
                   &dlPathlossDb));
  Config::Connect("/ChannelList/1/PathLoss",
                  MakeCallback(&UplinkLteGlobalPathlossDatabase::UpdatePathloss,
                               &ulPathlossDb));

  Simulator::Stop(Seconds(0.035));
  Simulator::Run();

  const double enbTxPowerDbm = 30;
  const double ueTxPowerDbm = 10;
  const double ktDbm = -174;
  const double noisePowerDbm = ktDbm + 10 * std::log10(25 * 180000);
  const double ueNoiseFigureDb = 9.0;
  const double enbNoiseFigureDb = 5.0;
  double tolerance =
      (m_antennaGainDb != 0) ? std::abs(m_antennaGainDb) * 0.001 : 0.001;

  double expectedSinrDl =
      enbTxPowerDbm + m_antennaGainDb - noisePowerDbm + ueNoiseFigureDb;
  if (expectedSinrDl > 0) {
    double calculatedSinrDbDl = -INFINITY;
    if (dlSinrCatcher.GetValue()) {
      calculatedSinrDbDl =
          10.0 * std::log10(dlSinrCatcher.GetValue()->operator[](0));
    }
    double calculatedAntennaGainDbDl =
        -(enbTxPowerDbm - calculatedSinrDbDl - noisePowerDbm - ueNoiseFigureDb);
    NS_LOG_INFO("expected " << m_antennaGainDb << " actual "
                            << calculatedAntennaGainDbDl << " tol "
                            << tolerance);
    NS_TEST_ASSERT_MSG_EQ_TOL(calculatedAntennaGainDbDl, m_antennaGainDb,
                              tolerance, "Wrong DL antenna gain!");
  }
  double expectedSinrUl =
      ueTxPowerDbm + m_antennaGainDb - noisePowerDbm + enbNoiseFigureDb;
  if (expectedSinrUl > 0) {
    double calculatedSinrDbUl = -INFINITY;
    if (ulSinrCatcher.GetValue()) {
      calculatedSinrDbUl =
          10.0 * std::log10(ulSinrCatcher.GetValue()->operator[](0));
    }
    double calculatedAntennaGainDbUl =
        -(ueTxPowerDbm - calculatedSinrDbUl - noisePowerDbm - enbNoiseFigureDb);
    NS_TEST_ASSERT_MSG_EQ_TOL(calculatedAntennaGainDbUl, m_antennaGainDb,
                              tolerance, "Wrong UL antenna gain!");
  }

  double measuredLossDl = dlPathlossDb.GetPathloss(1, 1);
  NS_TEST_ASSERT_MSG_EQ_TOL(measuredLossDl, -m_antennaGainDb, tolerance,
                            "Wrong DL loss!");
  double measuredLossUl = ulPathlossDb.GetPathloss(1, 1);
  NS_TEST_ASSERT_MSG_EQ_TOL(measuredLossUl, -m_antennaGainDb, tolerance,
                            "Wrong UL loss!");

  Simulator::Destroy();
}

class LteAntennaTestSuite : public TestSuite {
public:
  LteAntennaTestSuite();
};

LteAntennaTestSuite::LteAntennaTestSuite() : TestSuite("lte-antenna", SYSTEM) {
  NS_LOG_FUNCTION(this);

  AddTestCase(new LteEnbAntennaTestCase(0.0, 90.0, 1.0, 0.0, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 90.0, 1.0, 1.0, -3.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 90.0, 1.0, -1.0, -3.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 90.0, -1.0, -1.0, -36.396),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 90.0, -1.0, -0.0, -1414.6),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 90.0, -1.0, 1.0, -36.396),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(45.0, 90.0, 1.0, 1.0, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-45.0, 90.0, 1.0, -1.0, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(90.0, 90.0, 1.0, 1.0, -3.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-90.0, 90.0, 1.0, -1.0, -3.0),
              TestCase::QUICK);

  AddTestCase(new LteEnbAntennaTestCase(0.0, 120.0, 1.0, 0.0, 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 120.0, 0.5, sin(M_PI / 3), -3.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 120.0, 0.5, -sin(M_PI / 3), -3.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 120.0, -1.0, -2.0, -13.410),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(0.0, 120.0, -1.0, 1.0, -20.034),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(60.0, 120.0, 0.5, sin(M_PI / 3), 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-60.0, 120.0, 0.5, -sin(M_PI / 3), 0.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-60.0, 120.0, 0.5, -sin(M_PI / 3), 0.0),
              TestCase::QUICK);
  AddTestCase(
      new LteEnbAntennaTestCase(-120.0, 120.0, -0.5, -sin(M_PI / 3), 0.0),
      TestCase::QUICK);
  AddTestCase(
      new LteEnbAntennaTestCase(-120.0, 120.0, 0.5, -sin(M_PI / 3), -3.0),
      TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-120.0, 120.0, -1, 0, -3.0),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-120.0, 120.0, -1, 2, -15.578),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-120.0, 120.0, 1, 0, -14.457),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-120.0, 120.0, 1, 2, -73.154),
              TestCase::QUICK);
  AddTestCase(new LteEnbAntennaTestCase(-120.0, 120.0, 1, -0.1, -12.754),
              TestCase::QUICK);
}

static LteAntennaTestSuite g_lteAntennaTestSuite;
