
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/double.h"
#include "ns3/energy-source-container.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/rv-battery-model-helper.h"
#include "ns3/rv-battery-model.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/yans-wifi-helper.h"

#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RvBatteryModelTestSuite");

class BatteryLifetimeTest {
public:
  BatteryLifetimeTest();
  virtual ~BatteryLifetimeTest();

  void CreateLoadProfiles();

  bool ConstantLoadTest(double load, Time expLifetime) const;

  bool VariableLoadTest(std::vector<double> loads, std::vector<Time> timeStamps,
                        Time expLifetime) const;

  struct LoadProfile {
    std::vector<double> loads;
    std::vector<Time> timeStamps;
    Time itsyLifetime;
    Time dualFoilLifeTime;
  };

  std::vector<LoadProfile> m_loadProfiles;
  double m_alpha;
  double m_beta;
};

BatteryLifetimeTest::BatteryLifetimeTest() {
  m_alpha = 35220;
  m_beta = 0.637;
}

BatteryLifetimeTest::~BatteryLifetimeTest() {}

void BatteryLifetimeTest::CreateLoadProfiles() {
  LoadProfile profile;

  std::vector<double> loads;
  std::vector<Time> timeStamps;

  loads.push_back(0.628);
  loads.push_back(0);
  loads.push_back(0.628);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(19.5 * 60));
  timeStamps.push_back(Seconds(26.0 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(55.0 * 60);
  profile.dualFoilLifeTime = Seconds(36.2 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.4947);
  loads.push_back(0);
  loads.push_back(0.4947);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(31.0 * 60));
  timeStamps.push_back(Seconds(41.3 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(73.9 * 60);
  profile.dualFoilLifeTime = Seconds(55.8 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.4256);
  loads.push_back(0);
  loads.push_back(0.4256);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(41.0 * 60));
  timeStamps.push_back(Seconds(54.6 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(88.8 * 60);
  profile.dualFoilLifeTime = Seconds(71.8 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.2923);
  loads.push_back(0);
  loads.push_back(0.2923);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(74.6 * 60));
  timeStamps.push_back(Seconds(99.5 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(137.8 * 60);
  profile.dualFoilLifeTime = Seconds(124.9 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.2227);
  loads.push_back(0);
  loads.push_back(0.2227);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(105.7 * 60));
  timeStamps.push_back(Seconds(140.9 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(185.8 * 60);
  profile.dualFoilLifeTime = Seconds(176.7 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.628);
  loads.push_back(0);
  loads.push_back(0.628);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(19.5 * 60));
  timeStamps.push_back(Seconds(29.9 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(58.9 * 60);
  profile.dualFoilLifeTime = Seconds(41.0 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.628);
  loads.push_back(0);
  loads.push_back(0.628);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(19.5 * 60));
  timeStamps.push_back(Seconds(22.1 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(51.1 * 60);
  profile.dualFoilLifeTime = Seconds(30.8 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.628);
  loads.push_back(0);
  loads.push_back(0.628);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(23.4 * 60));
  timeStamps.push_back(Seconds(29.9 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(55.0 * 60);
  profile.dualFoilLifeTime = Seconds(37.4 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.628);
  loads.push_back(0);
  loads.push_back(0.628);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(15.6 * 60));
  timeStamps.push_back(Seconds(22.1 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(55.0 * 60);
  profile.dualFoilLifeTime = Seconds(35.2 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.300);
  loads.push_back(0.628);
  loads.push_back(0.4947);
  loads.push_back(0.2523);
  loads.push_back(0.2341);
  loads.push_back(0.1379);
  loads.push_back(0.1139);
  loads.push_back(0.2656);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(0.5 * 60));
  timeStamps.push_back(Seconds(5.5 * 60));
  timeStamps.push_back(Seconds(10.5 * 60));
  timeStamps.push_back(Seconds(35.5 * 60));
  timeStamps.push_back(Seconds(60.5 * 60));
  timeStamps.push_back(Seconds(85.5 * 60));
  timeStamps.push_back(Seconds(110.5 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(144.3 * 60);
  profile.dualFoilLifeTime = Seconds(132.6 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.300);
  loads.push_back(0.1139);
  loads.push_back(0.1379);
  loads.push_back(0.2341);
  loads.push_back(0.2523);
  loads.push_back(0.4947);
  loads.push_back(0.628);
  loads.push_back(0.2656);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(0.5 * 60));
  timeStamps.push_back(Seconds(25.5 * 60));
  timeStamps.push_back(Seconds(50.5 * 60));
  timeStamps.push_back(Seconds(75.5 * 60));
  timeStamps.push_back(Seconds(100.5 * 60));
  timeStamps.push_back(Seconds(105.5 * 60));
  timeStamps.push_back(Seconds(110.5 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(144.3 * 60);
  profile.dualFoilLifeTime = Seconds(107.4 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.300);
  loads.push_back(0.1139);
  loads.push_back(0.1379);
  loads.push_back(0.2341);
  loads.push_back(0.2523);
  loads.push_back(0.4947);
  loads.push_back(0.0);
  loads.push_back(0.300);
  loads.push_back(0.628);
  loads.push_back(0.2656);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(0.5 * 60));
  timeStamps.push_back(Seconds(25.5 * 60));
  timeStamps.push_back(Seconds(50.5 * 60));
  timeStamps.push_back(Seconds(75.5 * 60));
  timeStamps.push_back(Seconds(100.5 * 60));
  timeStamps.push_back(Seconds(105.5 * 60));
  timeStamps.push_back(Seconds(130.5 * 60));
  timeStamps.push_back(Seconds(131.0 * 60));
  timeStamps.push_back(Seconds(136.0 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(169.3 * 60);
  profile.dualFoilLifeTime = Seconds(155.4 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.300);
  timeStamps.push_back(Seconds(0));

  for (int i = 0; i < 5; i++) {
    loads.push_back(0.628);
    loads.push_back(0.4947);
    loads.push_back(0.2523);
    loads.push_back(0.2341);
    loads.push_back(0.1379);
    loads.push_back(0.1139);

    timeStamps.push_back(Seconds((0.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((1.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((2.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((7.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((12.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((17.5 + i * 22.5) * 60));
  }

  loads.push_back(0.2656);
  timeStamps.push_back(Seconds(110.5 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(144.3 * 60);
  profile.dualFoilLifeTime = Seconds(131.7 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.300);
  timeStamps.push_back(Seconds(0));

  for (int i = 0; i < 5; i++) {
    loads.push_back(0.1139);
    loads.push_back(0.1379);
    loads.push_back(0.2341);
    loads.push_back(0.2523);
    loads.push_back(0.4947);
    loads.push_back(0.628);

    timeStamps.push_back(Seconds((0.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((5.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((10.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((15.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((20.5 + i * 22.5) * 60));
    timeStamps.push_back(Seconds((21.5 + i * 22.5) * 60));
  }

  loads.push_back(0.2656);
  timeStamps.push_back(Seconds(112.5 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(141.5 * 60);
  profile.dualFoilLifeTime = Seconds(126.3 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.2227);
  loads.push_back(0.2045);
  loads.push_back(0.1083);
  loads.push_back(0.0843);
  loads.push_back(0.2227);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(50.0 * 60));
  timeStamps.push_back(Seconds(100.0 * 60));
  timeStamps.push_back(Seconds(150.0 * 60));
  timeStamps.push_back(Seconds(200.0 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(211.4 * 60);
  profile.dualFoilLifeTime = Seconds(209.2 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.0843);
  loads.push_back(0.1083);
  loads.push_back(0.2045);
  loads.push_back(0.2227);
  loads.push_back(0.2227);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(50.0 * 60));
  timeStamps.push_back(Seconds(100.0 * 60));
  timeStamps.push_back(Seconds(150.0 * 60));
  timeStamps.push_back(Seconds(200.0 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(211.4 * 60);
  profile.dualFoilLifeTime = Seconds(200.7 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  loads.push_back(0.0843);
  loads.push_back(0.1083);
  loads.push_back(0.2045);
  loads.push_back(0.0);
  loads.push_back(0.2227);
  loads.push_back(0.2227);

  timeStamps.push_back(Seconds(0));
  timeStamps.push_back(Seconds(50.0 * 60));
  timeStamps.push_back(Seconds(100.0 * 60));
  timeStamps.push_back(Seconds(150.0 * 60));
  timeStamps.push_back(Seconds(200.0 * 60));
  timeStamps.push_back(Seconds(250.0 * 60));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(261.4 * 60);
  profile.dualFoilLifeTime = Seconds(251.2 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  for (int i = 0; i < 10; i++) {
    loads.push_back(0.0843);
    loads.push_back(0.1083);
    loads.push_back(0.2045);
    loads.push_back(0.2227);

    timeStamps.push_back(Seconds((0.0 + i * 20.0) * 60));
    timeStamps.push_back(Seconds((5.0 + i * 20.0) * 60));
    timeStamps.push_back(Seconds((10.0 + i * 20.0) * 60));
    timeStamps.push_back(Seconds((15.0 + i * 20.0) * 60));
  }

  loads.push_back(0.2227);
  timeStamps.push_back(Seconds(200.0));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(211.4 * 60);
  profile.dualFoilLifeTime = Seconds(204.6 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  for (int i = 0; i < 10; i++) {
    loads.push_back(0.0755);
    loads.push_back(0.0949);
    loads.push_back(0.2045);
    loads.push_back(0.2227);

    timeStamps.push_back(Seconds((0.0 + i * 20.0) * 60));
    timeStamps.push_back(Seconds((5.0 + i * 20.0) * 60));
    timeStamps.push_back(Seconds((10.0 + i * 20.0) * 60));
    timeStamps.push_back(Seconds((15.0 + i * 20.0) * 60));
  }

  loads.push_back(0.2227);
  timeStamps.push_back(Seconds(200.0));

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(216.4 * 60);
  profile.dualFoilLifeTime = Seconds(208.7 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  for (int i = 0; i < 50; i++) {
    loads.push_back(0.4947);
    loads.push_back(0.628);

    timeStamps.push_back(Seconds((0.0 + i * 2.0) * 60));
    timeStamps.push_back(Seconds((1.0 + i * 2.0) * 60));
  }

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(55.3 * 60);
  profile.dualFoilLifeTime = Seconds(33.2 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  for (int i = 0; i < 50; i++) {
    loads.push_back(0.4947);
    loads.push_back(0.628);
    loads.push_back(0.0576);

    timeStamps.push_back(Seconds((0.0 + i * 3.0) * 60));
    timeStamps.push_back(Seconds((1.0 + i * 3.0) * 60));
    timeStamps.push_back(Seconds((2.0 + i * 3.0) * 60));
  }

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(79.6 * 60);
  profile.dualFoilLifeTime = Seconds(55.9 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();

  for (int i = 0; i < 150; i++) {
    loads.push_back(0.005 + 0.005 * i);
    timeStamps.push_back(Seconds((0.0 + i * 1.0) * 60));
  }

  profile.loads = loads;
  profile.timeStamps = timeStamps;
  profile.itsyLifetime = Seconds(112.2 * 60);
  profile.dualFoilLifeTime = Seconds(94.5 * 60);

  m_loadProfiles.push_back(profile);

  loads.clear();
  timeStamps.clear();
}

int main(int argc, char **argv) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NS_LOG_DEBUG("Constant load run.");

  BatteryLifetimeTest test;
  int ret = 0;

  if (test.ConstantLoadTest(0.640, Seconds(2844.0))) {
    ret = 1;
    std::cerr << "Problems with constant load test (640mA)." << std::endl;
  }
  if (test.ConstantLoadTest(0.320, Seconds(6146.0))) {
    ret = 1;
    std::cerr << "Problems with constant load test (320mA)." << std::endl;
  }
  if (test.ConstantLoadTest(0.128, Seconds(16052.0))) {
    ret = 1;
    std::cerr << "Problems with constant load test (128mA)." << std::endl;
  }
  if (test.ConstantLoadTest(0.064, Seconds(32561.0))) {
    ret = 1;
    std::cerr << "Problems with constant load test (64mA)." << std::endl;
  }
  if (test.ConstantLoadTest(0.032, Seconds(65580.0))) {
    ret = 1;
    std::cerr << "Problems with constant load test (32mA)." << std::endl;
  }

  test.CreateLoadProfiles();

  NS_LOG_DEBUG("\n\nItsy");
  test.m_alpha = 35220;
  test.m_beta = 0.637;
  for (uint32_t i = 0; i < test.m_loadProfiles.size(); i++) {
    NS_LOG_DEBUG("========");
    NS_LOG_DEBUG("Variable load profile C" << i + 1);
    if (test.VariableLoadTest(test.m_loadProfiles[i].loads,
                              test.m_loadProfiles[i].timeStamps,
                              test.m_loadProfiles[i].itsyLifetime)) {
      ret = 1;
      std::cerr << "Problems with variable load test (Itsy)." << std::endl;
    }
  }

  NS_LOG_DEBUG("\n\nDUALFOIL");
  test.m_alpha = 40027;
  test.m_beta = 0.276;
  for (uint32_t i = 0; i < test.m_loadProfiles.size(); i++) {
    NS_LOG_DEBUG("========");
    NS_LOG_DEBUG("Variable load profile C" << i + 1);
    if (test.VariableLoadTest(test.m_loadProfiles[i].loads,
                              test.m_loadProfiles[i].timeStamps,
                              test.m_loadProfiles[i].dualFoilLifeTime)) {
      ret = 1;
      std::cerr << "Problems with variable load test (DUALFOIL)." << std::endl;
    }
  }

  return ret;
}

bool BatteryLifetimeTest::ConstantLoadTest(double load,
                                           Time expLifetime) const {
  NodeContainer c;
  c.Create(1);

  std::string phyMode("DsssRate1Mbps");

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set("RxGain", DoubleValue(0));
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

  RvBatteryModelHelper rvModelHelper;
  rvModelHelper.Set("RvBatteryModelAlphaValue", DoubleValue(m_alpha));
  rvModelHelper.Set("RvBatteryModelBetaValue", DoubleValue(m_beta));
  rvModelHelper.Set("RvBatteryModelLowBatteryThreshold", DoubleValue(0.0));
  EnergySourceContainer sources = rvModelHelper.Install(c);
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set("IdleCurrentA", DoubleValue(load));
  DeviceEnergyModelContainer deviceModels =
      radioEnergyHelper.Install(devices, sources);

  Simulator::Stop(Seconds(70000.0));
  Simulator::Run();

  Time actualLifetime;
  Ptr<RvBatteryModel> srcPtr = DynamicCast<RvBatteryModel>(sources.Get(0));
  actualLifetime = srcPtr->GetLifetime();

  NS_LOG_DEBUG("Expected lifetime = " << expLifetime.As(Time::S));
  NS_LOG_DEBUG("Actual lifetime = " << actualLifetime.As(Time::S));

  Simulator::Destroy();

  if (actualLifetime != expLifetime) {
    std::cerr << "ConstantLoadTest: Incorrect lifetime for load " << load
              << std::endl;
    return true;
  }

  return false;
}

bool BatteryLifetimeTest::VariableLoadTest(std::vector<double> loads,
                                           std::vector<Time> timeStamps,
                                           Time expLifetime) const {
  NS_ASSERT(loads.size() == timeStamps.size());

  NodeContainer c;
  c.Create(1);

  std::string phyMode("DsssRate1Mbps");

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue(phyMode));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set("RxGain", DoubleValue(0));
  wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(phyMode), "ControlMode",
                               StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

  RvBatteryModelHelper rvModelHelper;
  rvModelHelper.Set("RvBatteryModelAlphaValue", DoubleValue(m_alpha));
  rvModelHelper.Set("RvBatteryModelBetaValue", DoubleValue(m_beta));
  rvModelHelper.Set("RvBatteryModelLowBatteryThreshold", DoubleValue(0.0));
  EnergySourceContainer sources = rvModelHelper.Install(c);
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set("IdleCurrentA", DoubleValue(loads[0]));
  DeviceEnergyModelContainer deviceModels =
      radioEnergyHelper.Install(devices, sources);

  Ptr<WifiRadioEnergyModel> wifiDevicePtr =
      DynamicCast<WifiRadioEnergyModel>(deviceModels.Get(0));
  for (uint32_t i = 1; i < loads.size(); i++) {
    Simulator::Schedule(timeStamps[i], &WifiRadioEnergyModel::SetIdleCurrentA,
                        wifiDevicePtr, loads[i]);
  }

  Simulator::Stop(Seconds(70000.0));
  Simulator::Run();

  Time actualLifetime;
  Ptr<RvBatteryModel> srcPtr = DynamicCast<RvBatteryModel>(sources.Get(0));
  actualLifetime = srcPtr->GetLifetime();

  NS_LOG_DEBUG("Expected lifetime = " << expLifetime.As(Time::S));
  NS_LOG_DEBUG("Actual lifetime = " << actualLifetime.As(Time::S));
  NS_LOG_DEBUG("Difference = " << (expLifetime - actualLifetime).As(Time::S));

  Simulator::Destroy();

  if (Abs(actualLifetime - expLifetime) > Seconds(120)) {
    std::cerr << "VariableLoadTest: Incorrect lifetime." << std::endl;
    return true;
  }

  return false;
}
