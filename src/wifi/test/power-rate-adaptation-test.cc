
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/fcfs-wifi-queue-scheduler.h"
#include "ns3/frame-exchange-manager.h"
#include "ns3/interference-helper.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/wifi-default-ack-manager.h"
#include "ns3/wifi-default-protection-manager.h"
#include "ns3/wifi-net-device.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-phy.h"

using namespace ns3;

class PowerRateAdaptationTest : public TestCase {
public:
  PowerRateAdaptationTest();

  void DoRun() override;

private:
  void TestParf();
  void TestAparf();
  void TestRrpaa();
  Ptr<Node> ConfigureNode();

  ObjectFactory m_manager;
};

PowerRateAdaptationTest::PowerRateAdaptationTest()
    : TestCase("PowerRateAdaptation") {}

Ptr<Node> PowerRateAdaptationTest::ConfigureNode() {
  Ptr<WifiNetDevice> dev = CreateObject<WifiNetDevice>();
  Ptr<Node> node = CreateObject<Node>();
  node->AddDevice(dev);

  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel>();

  Ptr<ConstantPositionMobilityModel> mobility =
      CreateObject<ConstantPositionMobilityModel>();

  Ptr<YansWifiPhy> phy = CreateObject<YansWifiPhy>();
  Ptr<InterferenceHelper> interferenceHelper =
      CreateObject<InterferenceHelper>();
  phy->SetInterferenceHelper(interferenceHelper);
  dev->SetPhy(phy);
  phy->SetChannel(channel);
  phy->SetDevice(dev);
  phy->SetMobility(mobility);
  phy->ConfigureStandard(WIFI_STANDARD_80211a);

  phy->SetNTxPower(18);
  phy->SetTxPowerStart(0);
  phy->SetTxPowerEnd(17);

  Ptr<WifiRemoteStationManager> manager =
      m_manager.Create<WifiRemoteStationManager>();
  dev->SetRemoteStationManager(manager);

  Ptr<AdhocWifiMac> mac = CreateObject<AdhocWifiMac>();
  mac->SetDevice(dev);
  mac->SetAddress(Mac48Address::Allocate());
  dev->SetMac(mac);
  mac->ConfigureStandard(WIFI_STANDARD_80211a);
  mac->SetMacQueueScheduler(CreateObject<FcfsWifiQueueScheduler>());
  Ptr<FrameExchangeManager> fem = mac->GetFrameExchangeManager();

  Ptr<WifiProtectionManager> protectionManager =
      CreateObject<WifiDefaultProtectionManager>();
  protectionManager->SetWifiMac(mac);
  fem->SetProtectionManager(protectionManager);

  Ptr<WifiAckManager> ackManager = CreateObject<WifiDefaultAckManager>();
  ackManager->SetWifiMac(mac);
  fem->SetAckManager(ackManager);

  return node;
}

void PowerRateAdaptationTest::TestParf() {
  m_manager.SetTypeId("ns3::ParfWifiManager");
  Ptr<Node> node = ConfigureNode();
  Ptr<WifiNetDevice> dev = DynamicCast<WifiNetDevice>(node->GetDevice(0));
  Ptr<WifiRemoteStationManager> manager = dev->GetRemoteStationManager();

  manager->SetAttribute("AttemptThreshold", UintegerValue(15));
  manager->SetAttribute("SuccessThreshold", UintegerValue(10));

  Mac48Address remoteAddress = Mac48Address::Allocate();
  WifiMacHeader packetHeader;
  packetHeader.SetAddr1(remoteAddress);
  packetHeader.SetType(WIFI_MAC_DATA);
  packetHeader.SetQosTid(0);
  Ptr<Packet> packet = Create<Packet>(10);
  Ptr<WifiMpdu> mpdu = Create<WifiMpdu>(packet, packetHeader);
  WifiMode ackMode;

  Ptr<Packet> p = Create<Packet>();
  dev->Send(p, remoteAddress, 1);

  WifiTxVector txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  WifiMode mode = txVector.GetMode();
  int power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Initial data rate wrong");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Initial power level wrong");

  for (int i = 0; i < 10; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "PARF: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Incorrect value of power level");

  for (int i = 0; i < 7; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
    manager->ReportDataFailed(mpdu);
  }
  manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "PARF: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);
  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 48000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Incorrect value of power level");

  for (int i = 0; i < 10; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 48000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Incorrect value of power level");

  for (int i = 0; i < 10; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Incorrect value of power level");

  for (int i = 0; i < 10; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "PARF: Incorrect value of power level");

  manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);

  for (int i = 0; i < 2; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "PARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "PARF: Incorrect value of power level");

  Simulator::Destroy();
}

void PowerRateAdaptationTest::TestAparf() {
  m_manager.SetTypeId("ns3::AparfWifiManager");
  Ptr<Node> node = ConfigureNode();
  Ptr<WifiNetDevice> dev = DynamicCast<WifiNetDevice>(node->GetDevice(0));
  Ptr<WifiRemoteStationManager> manager = dev->GetRemoteStationManager();

  manager->SetAttribute("SuccessThreshold1", UintegerValue(3));
  manager->SetAttribute("SuccessThreshold2", UintegerValue(10));
  manager->SetAttribute("FailThreshold", UintegerValue(1));
  manager->SetAttribute("PowerThreshold", UintegerValue(10));

  Mac48Address remoteAddress = Mac48Address::Allocate();
  WifiMacHeader packetHeader;
  packetHeader.SetAddr1(remoteAddress);
  packetHeader.SetType(WIFI_MAC_DATA);
  packetHeader.SetQosTid(0);
  Ptr<Packet> packet = Create<Packet>(10);
  Ptr<WifiMpdu> mpdu = Create<WifiMpdu>(packet, packetHeader);
  WifiMode ackMode;

  Ptr<Packet> p = Create<Packet>();
  dev->Send(p, remoteAddress, 1);

  WifiTxVector txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  WifiMode mode = txVector.GetMode();
  int power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Initial data rate wrong");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "APARF: Initial power level wrong");

  for (int i = 0; i < 3; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "APARF: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "APARF: Incorrect value of power level");

  for (int i = 0; i < 10; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "APARF: Incorrect value of power level");

  for (int i = 0; i < 3; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 15, "APARF: Incorrect value of power level");

  for (int i = 0; i < 16 * 3; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 0, "APARF: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 1, "Incorrect value of power level");

  for (int i = 0; i < 16; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "APARF: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 48000000,
                        "Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "Incorrect value of power level");

  for (int i = 0; i < 3; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 48000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "APARF: Incorrect value of power level");

  for (int i = 0; i < 9 * 3; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 48000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 7, "APARF: Incorrect value of power level");

  for (int i = 0; i < 3; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth()), 54000000,
                        "APARF: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "APARF: Incorrect value of power level");

  Simulator::Destroy();
}

void PowerRateAdaptationTest::TestRrpaa() {
  m_manager.SetTypeId("ns3::RrpaaWifiManager");
  Ptr<Node> node = ConfigureNode();
  Ptr<WifiNetDevice> dev = DynamicCast<WifiNetDevice>(node->GetDevice(0));
  Ptr<WifiRemoteStationManager> manager = dev->GetRemoteStationManager();

  manager->SetAttribute("Basic", BooleanValue(true));
  manager->SetAttribute("Alpha", DoubleValue(1.25));
  manager->SetAttribute("Beta", DoubleValue(2));
  manager->SetAttribute("Tau", DoubleValue(0.015));
  manager->SetAttribute("Gamma", DoubleValue(1));
  manager->SetAttribute("Delta", DoubleValue(1));

  Mac48Address remoteAddress = Mac48Address::Allocate();
  WifiMacHeader packetHeader;
  packetHeader.SetAddr1(remoteAddress);
  packetHeader.SetType(WIFI_MAC_DATA);
  packetHeader.SetQosTid(0);
  Ptr<Packet> packet = Create<Packet>(10);
  Ptr<WifiMpdu> mpdu = Create<WifiMpdu>(packet, packetHeader);
  WifiMode ackMode;

  Ptr<Packet> p = Create<Packet>();
  dev->Send(p, remoteAddress, 1);

  WifiTxVector txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  WifiMode mode = txVector.GetMode();
  int power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        6000000, "RRPAA: Initial data rate wrong");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Initial power level wrong");

  for (int i = 0; i < 6; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        6000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        9000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 4; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        9000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  manager->ReportDataFailed(mpdu);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        6000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 7; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        9000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 10; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        12000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 13; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        18000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 19; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        24000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 23; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        36000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 33; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        48000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 43; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        54000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 49; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        54000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        54000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 16 * 50; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        54000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 0, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 6; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        54000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 1, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 16 * 6; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        54000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 6; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        48000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 11; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        36000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 25; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        36000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 17, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 5; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        36000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 5; i++) {
    manager->ReportDataFailed(mpdu);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        36000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 16, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 25; i++) {
    manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        36000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 15, "RRPAA: Incorrect value of power level");

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 25; j++) {
      manager->ReportDataOk(mpdu, 0, ackMode, 0, txVector);
    }

    for (int j = 0; j < 5; j++) {
      manager->ReportDataFailed(mpdu);
    }
  }

  txVector =
      manager->GetDataTxVector(packetHeader, dev->GetPhy()->GetChannelWidth());
  mode = txVector.GetMode();
  power = (int)txVector.GetTxPowerLevel();

  NS_TEST_ASSERT_MSG_EQ(mode.GetDataRate(txVector.GetChannelWidth(),
                                         txVector.GetGuardInterval(), 1),
                        36000000, "RRPAA: Incorrect vale of data rate");
  NS_TEST_ASSERT_MSG_EQ(power, 0, "RRPAA: Incorrect value of power level");

  Simulator::Stop(Seconds(10.0));

  Simulator::Run();
  Simulator::Destroy();
}

void PowerRateAdaptationTest::DoRun() {
  TestParf();
  TestAparf();
  TestRrpaa();
}

class PowerRateAdaptationTestSuite : public TestSuite {
public:
  PowerRateAdaptationTestSuite();
};

PowerRateAdaptationTestSuite::PowerRateAdaptationTestSuite()
    : TestSuite("wifi-power-rate-adaptation", UNIT) {
  AddTestCase(new PowerRateAdaptationTest, TestCase::QUICK);
}

static PowerRateAdaptationTestSuite g_powerRateAdaptationTestSuite;
