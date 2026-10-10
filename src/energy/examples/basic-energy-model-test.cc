
#include "ns3/basic-energy-source-helper.h"
#include "ns3/basic-energy-source.h"
#include "ns3/config.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/double.h"
#include "ns3/energy-source-container.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/yans-wifi-helper.h"

#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BasicEnergyModelTestSuite");

class BasicEnergyUpdateTest {
public:
  BasicEnergyUpdateTest();
  virtual ~BasicEnergyUpdateTest();

  bool DoRun();

private:
  bool StateSwitchTest(WifiPhyState state);

private:
  double m_timeS;
  double m_tolerance;

  ObjectFactory m_energySource;
  ObjectFactory m_deviceEnergyModel;
};

BasicEnergyUpdateTest::BasicEnergyUpdateTest() {
  m_timeS = 15.5;
  m_tolerance = 1.0e-5;
}

BasicEnergyUpdateTest::~BasicEnergyUpdateTest() {}

bool BasicEnergyUpdateTest::DoRun() {
  m_energySource.SetTypeId("ns3::BasicEnergySource");
  m_deviceEnergyModel.SetTypeId("ns3::WifiRadioEnergyModel");

  if (StateSwitchTest(WifiPhyState::IDLE)) {
    return true;
    std::cerr << "Problem with state switch test (WifiPhy idle)." << std::endl;
  }
  if (StateSwitchTest(WifiPhyState::CCA_BUSY)) {
    return true;
    std::cerr << "Problem with state switch test (WifiPhy cca busy)."
              << std::endl;
  }
  if (StateSwitchTest(WifiPhyState::TX)) {
    return true;
    std::cerr << "Problem with state switch test (WifiPhy tx)." << std::endl;
  }
  if (StateSwitchTest(WifiPhyState::RX)) {
    return true;
    std::cerr << "Problem with state switch test (WifiPhy rx)." << std::endl;
  }
  if (StateSwitchTest(WifiPhyState::SWITCHING)) {
    return true;
    std::cerr << "Problem with state switch test (WifiPhy switching)."
              << std::endl;
  }
  if (StateSwitchTest(WifiPhyState::SLEEP)) {
    return true;
    std::cerr << "Problem with state switch test (WifiPhy sleep)." << std::endl;
  }
  return false;
}

bool BasicEnergyUpdateTest::StateSwitchTest(WifiPhyState state) {
  Ptr<Node> node = CreateObject<Node>();

  Ptr<BasicEnergySource> source = m_energySource.Create<BasicEnergySource>();
  source->SetInitialEnergy(50);
  node->AggregateObject(source);
  source->SetNode(node);

  Ptr<WifiRadioEnergyModel> model =
      m_deviceEnergyModel.Create<WifiRadioEnergyModel>();
  model->SetEnergySource(source);
  source->AppendDeviceEnergyModel(model);

  DeviceEnergyModelContainer models =
      source->FindDeviceEnergyModels("ns3::WifiRadioEnergyModel");
  if ((models.GetN() == 0)) {
    std::cerr << "Model list is empty!." << std::endl;
    return true;
  }
  Ptr<WifiRadioEnergyModel> devModel =
      DynamicCast<WifiRadioEnergyModel>(models.Get(0));
  if (!devModel) {
    std::cerr << "NULL pointer to device model!." << std::endl;
    return true;
  }

  Simulator::Schedule(Seconds(m_timeS), &WifiRadioEnergyModel::ChangeState,
                      devModel, state);

  Simulator::Schedule(Seconds(m_timeS * 2),
                      &BasicEnergySource::UpdateEnergySource, source);

  double timeDelta = 0.000000001;
  Simulator::Stop(Seconds(m_timeS * 2 + timeDelta));
  Simulator::Run();

  double estRemainingEnergy = source->GetInitialEnergy();
  double voltage = source->GetSupplyVoltage();
  estRemainingEnergy -= devModel->GetIdleCurrentA() * voltage * m_timeS;

  double current = 0.0;
  switch (state) {
  case WifiPhyState::IDLE:
    current = devModel->GetIdleCurrentA();
    break;
  case WifiPhyState::CCA_BUSY:
    current = devModel->GetCcaBusyCurrentA();
    break;
  case WifiPhyState::TX:
    current = devModel->GetTxCurrentA();
    break;
  case WifiPhyState::RX:
    current = devModel->GetRxCurrentA();
    break;
  case WifiPhyState::SWITCHING:
    current = devModel->GetSwitchingCurrentA();
    break;
  case WifiPhyState::SLEEP:
    current = devModel->GetSleepCurrentA();
    break;
  case WifiPhyState::OFF:
    current = 0;
    break;
  default:
    NS_FATAL_ERROR("Undefined radio state: " << state);
    break;
  }
  estRemainingEnergy -= current * voltage * m_timeS;
  estRemainingEnergy = std::max(0.0, estRemainingEnergy);

  double remainingEnergy = source->GetRemainingEnergy();
  NS_LOG_DEBUG("Remaining energy is " << remainingEnergy);
  NS_LOG_DEBUG("Estimated remaining energy is " << estRemainingEnergy);
  NS_LOG_DEBUG("Difference is " << estRemainingEnergy - remainingEnergy);

  if ((remainingEnergy > (estRemainingEnergy + m_tolerance)) ||
      (remainingEnergy < (estRemainingEnergy - m_tolerance))) {
    std::cerr << "Incorrect remaining energy!" << std::endl;
    return true;
  }

  WifiPhyState endState = devModel->GetCurrentState();
  NS_LOG_DEBUG("Radio state is " << endState);
  if (endState != state) {
    std::cerr << "Incorrect end state!" << std::endl;
    return true;
  }
  Simulator::Destroy();

  return false;
}

class BasicEnergyDepletionTest {
public:
  BasicEnergyDepletionTest();
  virtual ~BasicEnergyDepletionTest();

  bool DoRun();

private:
  void DepletionHandler();

  bool DepletionTestCase(double simTimeS, double updateIntervalS);

private:
  int m_numOfNodes;
  int m_callbackCount;
  double m_simTimeS;
  double m_timeStepS;
  double m_updateIntervalS;
};

BasicEnergyDepletionTest::BasicEnergyDepletionTest() {
  m_numOfNodes = 10;
  m_callbackCount = 0;
  m_simTimeS = 4.5;
  m_timeStepS = 0.5;
  m_updateIntervalS = 1.5;
}

BasicEnergyDepletionTest::~BasicEnergyDepletionTest() {}

bool BasicEnergyDepletionTest::DoRun() {
  bool ret = false;

  for (double simTimeS = 0.0; simTimeS <= m_simTimeS; simTimeS += m_timeStepS) {
    for (double updateIntervalS = 0.5; updateIntervalS <= m_updateIntervalS;
         updateIntervalS += m_timeStepS) {
      if (DepletionTestCase(simTimeS, updateIntervalS)) {
        ret = true;
        std::cerr << "Depletion test case problem." << std::endl;
      }
      m_callbackCount = 0;
    }
  }
  return ret;
}

void BasicEnergyDepletionTest::DepletionHandler() { m_callbackCount++; }

bool BasicEnergyDepletionTest::DepletionTestCase(double simTimeS,
                                                 double updateIntervalS) {
  NodeContainer c;
  c.Create(m_numOfNodes);

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

  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(0.0));
  basicSourceHelper.Set("PeriodicEnergyUpdateInterval",
                        TimeValue(Seconds(updateIntervalS)));
  EnergySourceContainer sources = basicSourceHelper.Install(c);

  WifiRadioEnergyModelHelper radioEnergyHelper;
  WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback =
      MakeCallback(&BasicEnergyDepletionTest::DepletionHandler, this);
  radioEnergyHelper.SetDepletionCallback(callback);
  DeviceEnergyModelContainer deviceModels =
      radioEnergyHelper.Install(devices, sources);

  Simulator::Stop(Seconds(simTimeS));
  Simulator::Run();
  Simulator::Destroy();

  NS_LOG_DEBUG("Simulation time = " << simTimeS << "s");
  NS_LOG_DEBUG("Update interval = " << updateIntervalS << "s");
  NS_LOG_DEBUG("Expected callback count is " << m_numOfNodes);
  NS_LOG_DEBUG("Actual callback count is " << m_callbackCount);

  if (m_numOfNodes != m_callbackCount) {
    std::cerr << "Not all callbacks are invoked!" << std::endl;
    return true;
  }

  return false;
}

int main(int argc, char **argv) {
  BasicEnergyUpdateTest testEnergyUpdate;
  if (testEnergyUpdate.DoRun()) {
    return 1;
  }

  BasicEnergyDepletionTest testEnergyDepletion;
  if (testEnergyDepletion.DoRun()) {
    return 1;
  }

  return 0;
}
