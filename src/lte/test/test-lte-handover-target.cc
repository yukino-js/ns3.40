
#include <ns3/boolean.h>
#include <ns3/callback.h>
#include <ns3/config.h>
#include <ns3/data-rate.h>
#include <ns3/double.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-static-routing.h>
#include <ns3/log.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-enb-phy.h>
#include <ns3/lte-helper.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/point-to-point-epc-helper.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/position-allocator.h>
#include <ns3/simulator.h>
#include <ns3/test.h>
#include <ns3/uinteger.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteHandoverTargetTest");

class LteHandoverTargetTestCase : public TestCase {
public:
  LteHandoverTargetTestCase(std::string name, Vector uePosition,
                            uint8_t gridSizeX, uint8_t gridSizeY,
                            uint16_t sourceCellId, uint16_t targetCellId,
                            std::string handoverAlgorithmType);

  ~LteHandoverTargetTestCase() override;

  void HandoverStartCallback(std::string context, uint64_t imsi,
                             uint16_t sourceCellId, uint16_t rnti,
                             uint16_t targetCellId);

  void CellShutdownCallback();

private:
  void DoRun() override;

  void DoTeardown() override;

  Vector m_uePosition;
  uint8_t m_gridSizeX;
  uint8_t m_gridSizeY;
  uint16_t m_sourceCellId;
  uint16_t m_targetCellId;
  std::string m_handoverAlgorithmType;

  Ptr<LteEnbNetDevice> m_sourceEnbDev;
  bool m_hasHandoverOccurred;
};

LteHandoverTargetTestCase::LteHandoverTargetTestCase(
    std::string name, Vector uePosition, uint8_t gridSizeX, uint8_t gridSizeY,
    uint16_t sourceCellId, uint16_t targetCellId,
    std::string handoverAlgorithmType)
    : TestCase(name), m_uePosition(uePosition), m_gridSizeX(gridSizeX),
      m_gridSizeY(gridSizeY), m_sourceCellId(sourceCellId),
      m_targetCellId(targetCellId),
      m_handoverAlgorithmType(handoverAlgorithmType), m_sourceEnbDev(nullptr),
      m_hasHandoverOccurred(false) {
  NS_LOG_INFO(this << " name=" << name);

  uint16_t nEnb = gridSizeX * gridSizeY;

  if (sourceCellId > nEnb) {
    NS_FATAL_ERROR("Invalid source cell ID " << sourceCellId);
  }

  if (targetCellId > nEnb) {
    NS_FATAL_ERROR("Invalid target cell ID " << targetCellId);
  }
}

LteHandoverTargetTestCase::~LteHandoverTargetTestCase() {
  NS_LOG_FUNCTION(this);
}

void LteHandoverTargetTestCase::HandoverStartCallback(std::string context,
                                                      uint64_t imsi,
                                                      uint16_t sourceCellId,
                                                      uint16_t rnti,
                                                      uint16_t targetCellId) {
  NS_LOG_FUNCTION(this << context << imsi << sourceCellId << rnti
                       << targetCellId);

  uint64_t timeNowMs = Simulator::Now().GetMilliSeconds();
  NS_TEST_ASSERT_MSG_GT(timeNowMs, 500, "Handover occurred but too early");
  NS_TEST_ASSERT_MSG_EQ(sourceCellId, m_sourceCellId,
                        "Handover occurred but with wrong source cell");
  NS_TEST_ASSERT_MSG_EQ(targetCellId, m_targetCellId,
                        "Handover occurred but with wrong target cell");
  m_hasHandoverOccurred = true;
}

void LteHandoverTargetTestCase::CellShutdownCallback() {
  NS_LOG_FUNCTION(this);

  if (m_sourceEnbDev) {
    NS_ASSERT(m_sourceEnbDev->GetCellId() == m_sourceCellId);
    NS_LOG_INFO("Shutting down cell " << m_sourceCellId);
    Ptr<LteEnbPhy> phy = m_sourceEnbDev->GetPhy();
    phy->SetTxPower(1);
  }
}

void LteHandoverTargetTestCase::DoRun() {
  NS_LOG_INFO(this << " " << GetName());

  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(38));
  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                     BooleanValue(false));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);
  lteHelper->SetAttribute(
      "PathlossModel", StringValue("ns3::FriisSpectrumPropagationLossModel"));
  lteHelper->SetAttribute("UseIdealRrc", BooleanValue(true));

  if (m_handoverAlgorithmType == "ns3::A2A4RsrqHandoverAlgorithm") {
    lteHelper->SetHandoverAlgorithmType("ns3::A2A4RsrqHandoverAlgorithm");
    lteHelper->SetHandoverAlgorithmAttribute("ServingCellThreshold",
                                             UintegerValue(30));
    lteHelper->SetHandoverAlgorithmAttribute("NeighbourCellOffset",
                                             UintegerValue(1));
  } else if (m_handoverAlgorithmType == "ns3::A3RsrpHandoverAlgorithm") {
    lteHelper->SetHandoverAlgorithmType("ns3::A3RsrpHandoverAlgorithm");
    lteHelper->SetHandoverAlgorithmAttribute("Hysteresis", DoubleValue(1.5));
    lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",
                                             TimeValue(MilliSeconds(128)));
  } else {
    NS_FATAL_ERROR("Unknown handover algorithm " << m_handoverAlgorithmType);
  }

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(m_gridSizeX * m_gridSizeY);
  ueNodes.Create(1);

  MobilityHelper enbMobility;
  enbMobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(130.0), "DeltaY",
      DoubleValue(130.0), "GridWidth", UintegerValue(m_gridSizeX), "LayoutType",
      StringValue("RowFirst"));
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.Install(enbNodes);

  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(m_uePosition);
  MobilityHelper ueMobility;
  ueMobility.SetPositionAllocator(positionAlloc);
  ueMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  ueMobility.Install(ueNodes);

  Ptr<Node> pgw = epcHelper->GetPgwNode();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
  NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
      ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
                                             Ipv4Mask("255.0.0.0"), 1);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
    Ptr<Node> ueNode = ueNodes.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                     1);
  }

  lteHelper->AddX2Interface(enbNodes);

  Config::Connect(
      "/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
      MakeCallback(&LteHandoverTargetTestCase::HandoverStartCallback, this));

  Ptr<NetDevice> sourceEnb = enbDevs.Get(m_sourceCellId - 1);
  m_sourceEnbDev = sourceEnb->GetObject<LteEnbNetDevice>();
  NS_ASSERT(m_sourceEnbDev);
  NS_ASSERT(m_sourceEnbDev->GetCellId() == m_sourceCellId);

  lteHelper->Attach(ueDevs.Get(0), sourceEnb);

  Simulator::Schedule(Seconds(0.5),
                      &LteHandoverTargetTestCase::CellShutdownCallback, this);

  Simulator::Stop(Seconds(1));
  Simulator::Run();
  Simulator::Destroy();
}

void LteHandoverTargetTestCase::DoTeardown() {
  NS_LOG_FUNCTION(this);
  NS_TEST_ASSERT_MSG_EQ(m_hasHandoverOccurred, true, "Handover did not occur");
}

class LteHandoverTargetTestSuite : public TestSuite {
public:
  LteHandoverTargetTestSuite();
};

LteHandoverTargetTestSuite::LteHandoverTargetTestSuite()
    : TestSuite("lte-handover-target", SYSTEM) {

  AddTestCase(new LteHandoverTargetTestCase("4 cells and A2-A4-RSRQ algorithm",
                                            Vector(20, 40, 0), 2, 2, 1, 3,
                                            "ns3::A2A4RsrqHandoverAlgorithm"),
              TestCase::QUICK);
  AddTestCase(new LteHandoverTargetTestCase(
                  "4 cells and strongest cell algorithm", Vector(20, 40, 0), 2,
                  2, 1, 3, "ns3::A3RsrpHandoverAlgorithm"),
              TestCase::QUICK);

  AddTestCase(new LteHandoverTargetTestCase("6 cells and A2-A4-RSRQ algorithm",
                                            Vector(150, 90, 0), 3, 2, 5, 2,
                                            "ns3::A2A4RsrqHandoverAlgorithm"),
              TestCase::EXTENSIVE);
  AddTestCase(new LteHandoverTargetTestCase(
                  "6 cells and strongest cell algorithm", Vector(150, 90, 0), 3,
                  2, 5, 2, "ns3::A3RsrpHandoverAlgorithm"),
              TestCase::EXTENSIVE);
}

static LteHandoverTargetTestSuite g_lteHandoverTargetTestSuiteInstance;
