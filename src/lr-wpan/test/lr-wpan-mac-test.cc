
#include <ns3/constant-position-mobility-model.h>
#include <ns3/core-module.h>
#include <ns3/log.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/packet.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("lr-wpan-mac-test");

class TestRxOffWhenIdleAfterCsmaFailure : public TestCase {
public:
  TestRxOffWhenIdleAfterCsmaFailure();
  ~TestRxOffWhenIdleAfterCsmaFailure() override;

private:
  void DataIndication(McpsDataIndicationParams params, Ptr<Packet> p);
  void DataConfirm(McpsDataConfirmParams params);
  void StateChangeNotificationDev0(std::string context, Time now,
                                   LrWpanPhyEnumeration oldState,
                                   LrWpanPhyEnumeration newState);
  void StateChangeNotificationDev2(std::string context, Time now,
                                   LrWpanPhyEnumeration oldState,
                                   LrWpanPhyEnumeration newState);

  void DoRun() override;

  LrWpanPhyEnumeration m_dev0State;
};

TestRxOffWhenIdleAfterCsmaFailure::TestRxOffWhenIdleAfterCsmaFailure()
    : TestCase("Test PHY going to TRX_OFF state after CSMA failure") {}

TestRxOffWhenIdleAfterCsmaFailure::~TestRxOffWhenIdleAfterCsmaFailure() {}

void TestRxOffWhenIdleAfterCsmaFailure::DataIndication(
    McpsDataIndicationParams params, Ptr<Packet> p) {
  NS_LOG_DEBUG("Received packet of size " << p->GetSize());
}

void TestRxOffWhenIdleAfterCsmaFailure::DataConfirm(
    McpsDataConfirmParams params) {
  if (params.m_status == LrWpanMcpsDataConfirmStatus::IEEE_802_15_4_SUCCESS) {
    NS_LOG_DEBUG("LrWpanMcpsDataConfirmStatus = Success");
  } else if (params.m_status == LrWpanMcpsDataConfirmStatus::
                                    IEEE_802_15_4_CHANNEL_ACCESS_FAILURE) {
    NS_LOG_DEBUG("LrWpanMcpsDataConfirmStatus =  Channel Access Failure");
  }
}

void TestRxOffWhenIdleAfterCsmaFailure::StateChangeNotificationDev0(
    std::string context, Time now, LrWpanPhyEnumeration oldState,
    LrWpanPhyEnumeration newState) {
  NS_LOG_DEBUG(Simulator::Now().As(Time::S)
               << context << "PHY state change at " << now.As(Time::S)
               << " from "
               << LrWpanHelper::LrWpanPhyEnumerationPrinter(oldState) << " to "
               << LrWpanHelper::LrWpanPhyEnumerationPrinter(newState));

  m_dev0State = newState;
}

void TestRxOffWhenIdleAfterCsmaFailure::StateChangeNotificationDev2(
    std::string context, Time now, LrWpanPhyEnumeration oldState,
    LrWpanPhyEnumeration newState) {
  NS_LOG_DEBUG(Simulator::Now().As(Time::S)
               << context << "PHY state change at " << now.As(Time::S)
               << " from "
               << LrWpanHelper::LrWpanPhyEnumerationPrinter(oldState) << " to "
               << LrWpanHelper::LrWpanPhyEnumerationPrinter(newState));
}

void TestRxOffWhenIdleAfterCsmaFailure::DoRun() {

  LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC));

  LogComponentEnable("LrWpanMac", LOG_LEVEL_DEBUG);
  LogComponentEnable("LrWpanCsmaCa", LOG_LEVEL_DEBUG);
  LogComponentEnable("lr-wpan-mac-test", LOG_LEVEL_DEBUG);

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();
  Ptr<Node> interferenceNode = CreateObject<Node>();

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev2 = CreateObject<LrWpanNetDevice>();

  dev0->SetAddress(Mac16Address("00:01"));
  dev1->SetAddress(Mac16Address("00:02"));
  dev2->SetAddress(Mac16Address("00:03"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  dev0->SetChannel(channel);
  dev1->SetChannel(channel);
  dev2->SetChannel(channel);

  n0->AddDevice(dev0);
  n1->AddDevice(dev1);
  interferenceNode->AddDevice(dev2);

  dev0->GetPhy()->TraceConnect(
      "TrxState", std::string("[address 00:01]"),
      MakeCallback(
          &TestRxOffWhenIdleAfterCsmaFailure::StateChangeNotificationDev0,
          this));
  dev2->GetPhy()->TraceConnect(
      "TrxState", std::string("[address 00:03]"),
      MakeCallback(
          &TestRxOffWhenIdleAfterCsmaFailure::StateChangeNotificationDev2,
          this));

  Ptr<ConstantPositionMobilityModel> sender0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender0Mobility->SetPosition(Vector(0, 0, 0));
  dev0->GetPhy()->SetMobility(sender0Mobility);
  Ptr<ConstantPositionMobilityModel> sender1Mobility =
      CreateObject<ConstantPositionMobilityModel>();

  sender1Mobility->SetPosition(Vector(0, 1, 0));
  dev1->GetPhy()->SetMobility(sender1Mobility);

  Ptr<ConstantPositionMobilityModel> sender3Mobility =
      CreateObject<ConstantPositionMobilityModel>();

  sender3Mobility->SetPosition(Vector(0, 2, 0));
  dev2->GetPhy()->SetMobility(sender3Mobility);

  McpsDataConfirmCallback cb0;
  cb0 = MakeCallback(&TestRxOffWhenIdleAfterCsmaFailure::DataConfirm, this);
  dev0->GetMac()->SetMcpsDataConfirmCallback(cb0);

  McpsDataIndicationCallback cb1;
  cb1 = MakeCallback(&TestRxOffWhenIdleAfterCsmaFailure::DataIndication, this);
  dev0->GetMac()->SetMcpsDataIndicationCallback(cb1);

  McpsDataConfirmCallback cb2;
  cb2 = MakeCallback(&TestRxOffWhenIdleAfterCsmaFailure::DataConfirm, this);
  dev1->GetMac()->SetMcpsDataConfirmCallback(cb2);

  McpsDataIndicationCallback cb3;
  cb3 = MakeCallback(&TestRxOffWhenIdleAfterCsmaFailure::DataIndication, this);
  dev1->GetMac()->SetMcpsDataIndicationCallback(cb3);

  dev0->GetMac()->SetRxOnWhenIdle(false);
  dev1->GetMac()->SetRxOnWhenIdle(false);

  dev0->GetCsmaCa()->SetMacMinBE(0);
  dev2->GetCsmaCa()->SetMacMinBE(0);

  dev0->GetCsmaCa()->SetMacMaxCSMABackoffs(0);
  dev2->GetCsmaCa()->SetMacMaxCSMABackoffs(0);

  Ptr<Packet> p0 = Create<Packet>(50);
  McpsDataRequestParams params;
  params.m_dstPanId = 0;

  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstAddr = Mac16Address("00:02");

  params.m_msduHandle = 0;

  Simulator::ScheduleWithContext(1, Seconds(0.00033),
                                 &LrWpanMac::McpsDataRequest, dev0->GetMac(),
                                 params, p0);

  Ptr<Packet> p2 = Create<Packet>(60);
  params.m_dstAddr = Mac16Address("00:02");

  Simulator::ScheduleWithContext(2, Seconds(0.0), &LrWpanMac::McpsDataRequest,
                                 dev2->GetMac(), params, p2);

  NS_LOG_DEBUG("----------- Start of TestRxOffWhenIdleAfterCsmaFailure "
               "-------------------");
  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(
      m_dev0State, LrWpanPhyEnumeration::IEEE_802_15_4_PHY_TRX_OFF,
      "Error, dev0 [00:01] PHY should be in TRX_OFF after CSMA failure");

  Simulator::Destroy();
}

class TestActiveScanPanDescriptors : public TestCase {
public:
  TestActiveScanPanDescriptors();
  ~TestActiveScanPanDescriptors() override;

private:
  void ScanConfirm(MlmeScanConfirmParams params);

  void BeaconNotifyIndication(MlmeBeaconNotifyIndicationParams params);

  void DoRun() override;

  std::vector<PanDescriptor> m_panDescriptorList;
  uint32_t g_beaconPayloadSize;
};

TestActiveScanPanDescriptors::TestActiveScanPanDescriptors()
    : TestCase("Test the reception of PAN descriptors while performing an "
               "active scan") {}

TestActiveScanPanDescriptors::~TestActiveScanPanDescriptors() {}

void TestActiveScanPanDescriptors::ScanConfirm(MlmeScanConfirmParams params) {
  if (params.m_status == MLMESCAN_SUCCESS) {
    m_panDescriptorList = params.m_panDescList;
  }
}

void TestActiveScanPanDescriptors::BeaconNotifyIndication(
    MlmeBeaconNotifyIndicationParams params) {
  g_beaconPayloadSize = params.m_sdu->GetSize();
}

void TestActiveScanPanDescriptors::DoRun() {

  Ptr<Node> coord1 = CreateObject<Node>();
  Ptr<Node> endNode = CreateObject<Node>();
  Ptr<Node> coord2 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> coord1NetDevice = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> endNodeNetDevice = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> coord2NetDevice = CreateObject<LrWpanNetDevice>();

  coord1NetDevice->GetMac()->SetExtendedAddress("00:00:00:00:00:00:CA:FE");
  coord1NetDevice->GetMac()->SetShortAddress(Mac16Address("00:00"));

  coord2NetDevice->GetMac()->SetExtendedAddress("00:00:00:00:00:00:BE:BE");
  coord2NetDevice->GetMac()->SetShortAddress(Mac16Address("00:00"));

  endNodeNetDevice->GetMac()->SetExtendedAddress("00:00:00:00:00:00:00:03");
  endNodeNetDevice->GetMac()->SetShortAddress(Mac16Address("ff:ff"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  coord1NetDevice->SetChannel(channel);
  endNodeNetDevice->SetChannel(channel);
  coord2NetDevice->SetChannel(channel);

  coord1->AddDevice(coord1NetDevice);
  endNode->AddDevice(endNodeNetDevice);
  coord2->AddDevice(coord2NetDevice);

  Ptr<ConstantPositionMobilityModel> coord1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  coord1Mobility->SetPosition(Vector(0, 0, 0));
  coord1NetDevice->GetPhy()->SetMobility(coord1Mobility);

  Ptr<ConstantPositionMobilityModel> endNodeMobility =
      CreateObject<ConstantPositionMobilityModel>();
  endNodeMobility->SetPosition(Vector(100, 0, 0));
  endNodeNetDevice->GetPhy()->SetMobility(endNodeMobility);

  Ptr<ConstantPositionMobilityModel> coord2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  coord2Mobility->SetPosition(Vector(206, 0, 0));
  coord2NetDevice->GetPhy()->SetMobility(coord2Mobility);

  MlmeScanConfirmCallback cb0;
  cb0 = MakeCallback(&TestActiveScanPanDescriptors::ScanConfirm, this);
  endNodeNetDevice->GetMac()->SetMlmeScanConfirmCallback(cb0);

  MlmeBeaconNotifyIndicationCallback cb1;
  cb1 =
      MakeCallback(&TestActiveScanPanDescriptors::BeaconNotifyIndication, this);
  endNodeNetDevice->GetMac()->SetMlmeBeaconNotifyIndicationCallback(cb1);

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 15;
  params.m_sfrmOrd = 15;
  params.m_logCh = 12;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 coord1NetDevice->GetMac(), params);

  Ptr<LrWpanMacPibAttributes> pibAttribute = Create<LrWpanMacPibAttributes>();
  pibAttribute->macBeaconPayload = Create<Packet>(25);
  coord2NetDevice->GetMac()->MlmeSetRequest(
      LrWpanMacPibAttributeIdentifier::macBeaconPayload, pibAttribute);

  MlmeStartRequestParams params2;
  params2.m_panCoor = true;
  params2.m_PanId = 7;
  params2.m_bcnOrd = 15;
  params2.m_sfrmOrd = 15;
  params2.m_logCh = 14;
  Simulator::ScheduleWithContext(2, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 coord2NetDevice->GetMac(), params2);

  MlmeScanRequestParams scanParams;
  scanParams.m_chPage = 0;
  scanParams.m_scanChannels = 0x7800;
  scanParams.m_scanDuration = 14;
  scanParams.m_scanType = MLMESCAN_ACTIVE;
  Simulator::ScheduleWithContext(1, Seconds(3.0), &LrWpanMac::MlmeScanRequest,
                                 endNodeNetDevice->GetMac(), scanParams);

  Simulator::Stop(Seconds(2000));
  NS_LOG_DEBUG(
      "----------- Start of TestActiveScanPanDescriptors -------------------");
  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(
      m_panDescriptorList.size(), 2,
      "Error, Beacons not received or PAN descriptors not found");

  if (m_panDescriptorList.size() == 2) {
    NS_TEST_ASSERT_MSG_LT(
        m_panDescriptorList[0].m_linkQuality, 255,
        "Error, Coordinator 1 (PAN 5) LQI value should be less than 255.");
    NS_TEST_ASSERT_MSG_LT(
        m_panDescriptorList[1].m_linkQuality, 255,
        "Error, Coordinator 2 (PAN 7) LQI value should be less than 255.");
    NS_TEST_ASSERT_MSG_GT(
        m_panDescriptorList[0].m_linkQuality, 0,
        "Error, Coordinator 1 (PAN 5) LQI value should be greater than 0.");
    NS_TEST_ASSERT_MSG_GT(
        m_panDescriptorList[1].m_linkQuality, 0,
        "Error, Coordinator 2 (PAN 7) LQI value should be greater than 0.");

    NS_TEST_ASSERT_MSG_LT(m_panDescriptorList[1].m_linkQuality,
                          m_panDescriptorList[0].m_linkQuality,
                          "Error, Coordinator 2 (PAN 7) LQI value should"
                          " be less than Coordinator 1 (PAN 5).");

    NS_TEST_EXPECT_MSG_EQ(
        g_beaconPayloadSize, 25,
        "Error, Beacon Payload not received or incorrect size (25 bytes)");
  }

  Simulator::Destroy();
}

class TestOrphanScan : public TestCase {
public:
  TestOrphanScan();
  ~TestOrphanScan() override;

private:
  void ScanConfirm(MlmeScanConfirmParams params);

  void OrphanIndicationCoord(MlmeOrphanIndicationParams params);

  void DoRun() override;

  Ptr<LrWpanNetDevice> coord1NetDevice;
  Ptr<LrWpanNetDevice> endNodeNetDevice;
  bool m_orphanScanSuccess;
};

TestOrphanScan::TestOrphanScan()
    : TestCase(
          "Test an orphan scan and the reception of the commands involved") {
  m_orphanScanSuccess = false;
}

TestOrphanScan::~TestOrphanScan() {}

void TestOrphanScan::ScanConfirm(MlmeScanConfirmParams params) {
  if (params.m_status == MLMESCAN_SUCCESS) {
    m_orphanScanSuccess = true;
  }
}

void TestOrphanScan::OrphanIndicationCoord(MlmeOrphanIndicationParams params) {

  if (params.m_orphanAddr == Mac64Address("00:00:00:00:00:00:00:02")) {
    MlmeOrphanResponseParams respParams;
    respParams.m_assocMember = true;
    respParams.m_orphanAddr = params.m_orphanAddr;
    respParams.m_shortAddr = Mac16Address("00:02");

    Simulator::ScheduleNow(&LrWpanMac::MlmeOrphanResponse,
                           coord1NetDevice->GetMac(), respParams);
  }
}

void TestOrphanScan::DoRun() {
  Ptr<Node> coord1 = CreateObject<Node>();
  Ptr<Node> endNode = CreateObject<Node>();

  coord1NetDevice = CreateObject<LrWpanNetDevice>();
  endNodeNetDevice = CreateObject<LrWpanNetDevice>();

  coord1NetDevice->GetMac()->SetExtendedAddress(
      Mac64Address("00:00:00:00:00:00:00:01"));
  coord1NetDevice->GetMac()->SetShortAddress(Mac16Address("00:01"));

  endNodeNetDevice->GetMac()->SetExtendedAddress(
      Mac64Address("00:00:00:00:00:00:00:02"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  coord1NetDevice->SetChannel(channel);
  endNodeNetDevice->SetChannel(channel);

  coord1->AddDevice(coord1NetDevice);
  endNode->AddDevice(endNodeNetDevice);

  Ptr<ConstantPositionMobilityModel> coord1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  coord1Mobility->SetPosition(Vector(0, 0, 0));
  coord1NetDevice->GetPhy()->SetMobility(coord1Mobility);

  Ptr<ConstantPositionMobilityModel> endNodeMobility =
      CreateObject<ConstantPositionMobilityModel>();
  endNodeMobility->SetPosition(Vector(100, 0, 0));
  endNodeNetDevice->GetPhy()->SetMobility(endNodeMobility);

  MlmeScanConfirmCallback cb1;
  cb1 = MakeCallback(&TestOrphanScan::ScanConfirm, this);
  endNodeNetDevice->GetMac()->SetMlmeScanConfirmCallback(cb1);

  MlmeOrphanIndicationCallback cb2;
  cb2 = MakeCallback(&TestOrphanScan::OrphanIndicationCoord, this);
  coord1NetDevice->GetMac()->SetMlmeOrphanIndicationCallback(cb2);

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 15;
  params.m_sfrmOrd = 15;
  params.m_logCh = 12;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 coord1NetDevice->GetMac(), params);

  MlmeScanRequestParams scanParams;
  scanParams.m_chPage = 0;
  scanParams.m_scanChannels = 0x7800;
  scanParams.m_scanType = MLMESCAN_ORPHAN;
  Simulator::ScheduleWithContext(1, Seconds(3.0), &LrWpanMac::MlmeScanRequest,
                                 endNodeNetDevice->GetMac(), scanParams);

  Simulator::Stop(Seconds(4000));
  NS_LOG_DEBUG("----------- Start of TestOrphanScan -------------------");
  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_orphanScanSuccess, true,
                        "Error, no coordinator realignment commands"
                        " received during orphan scan");
  if (m_orphanScanSuccess) {
    NS_TEST_EXPECT_MSG_EQ(endNodeNetDevice->GetMac()->GetShortAddress(),
                          Mac16Address("00:02"),
                          "Error, end device did not receive short address"
                          " during orphan scan");
  }

  Simulator::Destroy();
}

class LrWpanMacTestSuite : public TestSuite {
public:
  LrWpanMacTestSuite();
};

LrWpanMacTestSuite::LrWpanMacTestSuite() : TestSuite("lr-wpan-mac-test", UNIT) {
  AddTestCase(new TestRxOffWhenIdleAfterCsmaFailure, TestCase::QUICK);
  AddTestCase(new TestActiveScanPanDescriptors, TestCase::QUICK);
  AddTestCase(new TestOrphanScan, TestCase::QUICK);
}

static LrWpanMacTestSuite g_lrWpanMacTestSuite;
