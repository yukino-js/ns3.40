
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/packet.h"
#include "ns3/qos-utils.h"
#include "ns3/queue-size.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/wifi-net-device.h"

using namespace ns3;

class Issue211Test : public TestCase {
public:
  Issue211Test();
  ~Issue211Test() override;

  void DoRun() override;

private:
  void CalcThroughput(Ptr<UdpServer> server);

  std::vector<double> m_tputValues;
  uint64_t m_lastRxBytes;
  Time m_lastCheckPointTime;
  uint32_t m_payloadSize;
};

Issue211Test::Issue211Test()
    : TestCase("Test case for resuming data transmission when the recipient "
               "moves back"),
      m_lastRxBytes(0), m_lastCheckPointTime(Seconds(0)), m_payloadSize(2000) {}

Issue211Test::~Issue211Test() {}

void Issue211Test::CalcThroughput(Ptr<UdpServer> server) {
  uint64_t rxBytes = m_payloadSize * server->GetReceived();
  double tput = (rxBytes - m_lastRxBytes) * 8. /
                (Simulator::Now() - m_lastCheckPointTime).ToDouble(Time::US);
  m_tputValues.push_back(tput);
  m_lastRxBytes = rxBytes;
  m_lastCheckPointTime = Simulator::Now();
}

void Issue211Test::DoRun() {
  Time simulationTime(Seconds(6.0));
  Time moveAwayTime(Seconds(2.0));
  Time moveBackTime(Seconds(4.0));

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(40);
  int64_t streamNumber = 100;

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  NodeContainer wifiStaNode;
  wifiStaNode.Create(1);

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  SpectrumWifiPhyHelper phy;
  phy.SetChannel(spectrumChannel);

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HtMcs0"), "ControlMode",
                               StringValue("HtMcs0"));

  Config::SetDefault("ns3::WifiMacQueue::MaxSize",
                     QueueSizeValue(QueueSize("50p")));

  WifiMacHelper mac;
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(Ssid("issue211-test")));

  NetDeviceContainer staDevices = wifi.Install(phy, mac, wifiStaNode);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(Ssid("issue211-test")),
              "EnableBeaconJitter", BooleanValue(false));

  NetDeviceContainer apDevices = wifi.Install(phy, mac, wifiApNode);

  wifi.AssignStreams(apDevices, streamNumber);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(5.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNode);

  InternetStackHelper stack;
  stack.Install(wifiApNode);
  stack.Install(wifiStaNode);

  Ipv4AddressHelper address;
  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;

  staNodeInterface = address.Assign(staDevices.Get(0));
  apNodeInterface = address.Assign(apDevices.Get(0));

  ApplicationContainer serverApp;
  Time warmup(Seconds(1.0));

  uint16_t port = 9;
  UdpServerHelper server(port);
  serverApp = server.Install(wifiStaNode.Get(0));
  serverApp.Start(Seconds(0.0));
  serverApp.Stop(warmup + simulationTime);

  UdpClientHelper client(staNodeInterface.GetAddress(0), port);
  client.SetAttribute("MaxPackets", UintegerValue(4294967295U));
  client.SetAttribute("Interval", TimeValue(MilliSeconds(1)));
  client.SetAttribute("PacketSize", UintegerValue(m_payloadSize));
  ApplicationContainer clientApp = client.Install(wifiApNode.Get(0));
  clientApp.Start(warmup);
  clientApp.Stop(warmup + simulationTime);

  Ptr<MobilityModel> staMobility =
      wifiStaNode.Get(0)->GetObject<MobilityModel>();

  Simulator::Schedule(warmup + moveAwayTime, &MobilityModel::SetPosition,
                      staMobility, Vector(10000.0, 0.0, 0.0));
  Simulator::Schedule(warmup + moveAwayTime + MilliSeconds(10),
                      &Issue211Test::CalcThroughput, this,
                      DynamicCast<UdpServer>(serverApp.Get(0)));

  Simulator::Schedule(warmup + moveBackTime, &MobilityModel::SetPosition,
                      staMobility, Vector(5.0, 0.0, 0.0));
  Simulator::Schedule(warmup + moveBackTime, &Issue211Test::CalcThroughput,
                      this, DynamicCast<UdpServer>(serverApp.Get(0)));

  Simulator::Schedule(warmup + simulationTime, &Issue211Test::CalcThroughput,
                      this, DynamicCast<UdpServer>(serverApp.Get(0)));

  Simulator::Stop(warmup + simulationTime);
  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_tputValues.size(), 3,
                        "Unexpected number of throughput values");
  NS_TEST_EXPECT_MSG_GT(
      m_tputValues[0], 0,
      "Throughput must be non null before station moves away");
  NS_TEST_EXPECT_MSG_EQ(m_tputValues[1], 0,
                        "Throughput must be null while the station is away");
  NS_TEST_EXPECT_MSG_GT(m_tputValues[2], 0,
                        "Throughput must be non null when the station is back");

  for (const auto &t : m_tputValues) {
    std::cout << "Throughput = " << t << " Mb/s" << std::endl;
  }

  Simulator::Destroy();
}

class Issue211TestSuite : public TestSuite {
public:
  Issue211TestSuite();
};

Issue211TestSuite::Issue211TestSuite() : TestSuite("wifi-issue-211", UNIT) {
  AddTestCase(new Issue211Test, TestCase::QUICK);
}

static Issue211TestSuite g_issue211TestSuite;
