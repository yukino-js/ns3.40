

#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config-store.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-net-device.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-phy.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiPhyConfigurationExample");

Ptr<YansWifiPhy> GetYansWifiPhyPtr(const NetDeviceContainer &nc) {
  Ptr<WifiNetDevice> wnd = nc.Get(0)->GetObject<WifiNetDevice>();
  Ptr<WifiPhy> wp = wnd->GetPhy();
  return wp->GetObject<YansWifiPhy>();
}

void PrintAttributesIfEnabled(bool enabled) {
  if (enabled) {
    ConfigStore outputConfig;
    outputConfig.ConfigureAttributes();
  }
}

int main(int argc, char *argv[]) {
  uint32_t testCase = 0;
  bool printAttributes = false;
  bool exceptionThrown = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("testCase", "Test case", testCase);
  cmd.AddValue("printAttributes", "If true, print out attributes",
               printAttributes);
  cmd.Parse(argc, argv);

  NodeContainer wifiStaNode;
  wifiStaNode.Create(1);
  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());
  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::IdealWifiManager");

  Ssid ssid;
  ssid = Ssid("wifi-phy-configuration");
  WifiMacHelper macSta;
  macSta.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
                 BooleanValue(false));
  WifiMacHelper macAp;
  macAp.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "BeaconInterval",
                TimeValue(MicroSeconds(102400)), "BeaconGeneration",
                BooleanValue(true));
  NetDeviceContainer staDevice;
  NetDeviceContainer apDevice;
  Ptr<YansWifiPhy> phySta;
  Config::SetDefault(
      "ns3::ConfigStore::Filename",
      StringValue("output-attributes-" + std::to_string(testCase) + ".txt"));
  Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("RawText"));
  Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Save"));

  switch (testCase) {
  case 0:
    phySta = CreateObject<YansWifiPhy>();
    NS_ASSERT(!phySta->GetOperatingChannel().IsSet());
    PrintAttributesIfEnabled(printAttributes);
    break;

  case 1:
    wifi.SetStandard(WIFI_STANDARD_80211a);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 36);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5180);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 2:
    wifi.SetStandard(WIFI_STANDARD_80211b);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 1);
    NS_ASSERT(phySta->GetChannelWidth() == 22);
    NS_ASSERT(phySta->GetFrequency() == 2412);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 3:
    wifi.SetStandard(WIFI_STANDARD_80211g);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 1);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 2412);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 4:
    wifi.SetStandard(WIFI_STANDARD_80211n);
    phy.Set("ChannelSettings", StringValue("{0, 0, BAND_5GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 36);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5180);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 5:
    wifi.SetStandard(WIFI_STANDARD_80211n);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 1);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 2412);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 6:
    wifi.SetStandard(WIFI_STANDARD_80211ac);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 42);
    NS_ASSERT(phySta->GetChannelWidth() == 80);
    NS_ASSERT(phySta->GetFrequency() == 5210);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 7:
    wifi.SetStandard(WIFI_STANDARD_80211ax);
    phy.Set("ChannelSettings", StringValue("{0, 0, BAND_2_4GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 1);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 2412);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 8:
    wifi.SetStandard(WIFI_STANDARD_80211ax);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 42);
    NS_ASSERT(phySta->GetChannelWidth() == 80);
    NS_ASSERT(phySta->GetFrequency() == 5210);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 9:
    wifi.SetStandard(WIFI_STANDARD_80211ax);
    phy.Set("ChannelSettings", StringValue("{0, 0, BAND_6GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 7);
    NS_ASSERT(phySta->GetChannelWidth() == 80);
    NS_ASSERT(phySta->GetFrequency() == 5985);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 10:
    wifi.SetStandard(WIFI_STANDARD_80211p);
    phy.Set("ChannelSettings", StringValue("{0, 10, BAND_5GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 172);
    NS_ASSERT(phySta->GetChannelWidth() == 10);
    NS_ASSERT(phySta->GetFrequency() == 5860);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 11:
    wifi.SetStandard(WIFI_STANDARD_80211p);
    phy.Set("ChannelSettings", StringValue("{0, 5, BAND_5GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 171);
    NS_ASSERT(phySta->GetChannelWidth() == 5);
    NS_ASSERT(phySta->GetFrequency() == 5860);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 12:
    wifi.SetStandard(WIFI_STANDARD_80211n);
    phy.Set("ChannelSettings", StringValue("{44, 20, BAND_5GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 44);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5220);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 13:
    wifi.SetStandard(WIFI_STANDARD_80211n);
    phy.Set("ChannelSettings", StringValue("{44, 0, BAND_5GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    Config::Set("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/"
                "$ns3::YansWifiPhy/ChannelSettings",
                StringValue("{40, 0, BAND_5GHZ, 0}"));
    Config::Set("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Phy/"
                "$ns3::YansWifiPhy/ChannelSettings",
                StringValue("{40, 0, BAND_5GHZ, 0}"));
    NS_ASSERT(phySta->GetChannelNumber() == 40);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5200);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 14:
    wifi.SetStandard(WIFI_STANDARD_80211n);
    phy.Set("ChannelSettings", StringValue("{44, 0, BAND_5GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    Config::Set("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/"
                "$ns3::YansWifiPhy/ChannelSettings",
                StringValue("{46, 0, BAND_5GHZ, 0}"));
    Config::Set("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Phy/"
                "$ns3::YansWifiPhy/ChannelSettings",
                StringValue("{46, 0, BAND_5GHZ, 0}"));
    NS_ASSERT(phySta->GetChannelNumber() == 46);
    NS_ASSERT(phySta->GetChannelWidth() == 40);
    NS_ASSERT(phySta->GetFrequency() == 5230);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 15:
    Config::SetDefault("ns3::WifiPhy::ChannelSettings",
                       StringValue("{44, 0, BAND_5GHZ, 0}"));
    wifi.SetStandard(WIFI_STANDARD_80211n);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    Config::Set("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/"
                "$ns3::YansWifiPhy/ChannelSettings",
                StringValue("{46, 0, BAND_5GHZ, 0}"));
    Config::Set("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Phy/"
                "$ns3::YansWifiPhy/ChannelSettings",
                StringValue("{46, 0, BAND_5GHZ, 0}"));
    NS_ASSERT(phySta->GetChannelNumber() == 46);
    NS_ASSERT(phySta->GetChannelWidth() == 40);
    NS_ASSERT(phySta->GetFrequency() == 5230);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 16:
    Config::SetDefault("ns3::WifiPhy::ChannelSettings",
                       StringValue("{45, 0, BAND_5GHZ, 0}"));
    wifi.SetStandard(WIFI_STANDARD_80211n);
    exceptionThrown = false;
    try {
      staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    } catch (const std::runtime_error &) {
      exceptionThrown = true;
    }
    NS_ASSERT(exceptionThrown);
    exceptionThrown = false;
    try {
      apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    } catch (const std::runtime_error &) {
      exceptionThrown = true;
    }
    NS_ASSERT(exceptionThrown);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 17:
    Config::SetDefault("ns3::WifiPhy::ChannelSettings",
                       StringValue("{100, 0, BAND_5GHZ, 0}"));
    wifi.SetStandard(WIFI_STANDARD_80211n);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 100);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5500);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 18:
    wifi.SetStandard(WIFI_STANDARD_80211n);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    exceptionThrown = false;
    try {
      phySta->SetOperatingChannel(
          WifiPhy::ChannelTuple{99, 40, WIFI_PHY_BAND_5GHZ, 0});
    } catch (const std::runtime_error &) {
      exceptionThrown = true;
    }
    NS_ASSERT(exceptionThrown);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 19:
    wifi.SetStandard(WIFI_STANDARD_80211n);
    phy.Set("ChannelSettings", StringValue("{44, 0, BAND_5GHZ, 0}"));
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    exceptionThrown = false;
    try {
      phySta->SetAttribute("ChannelSettings",
                           StringValue("{45, 0, BAND_5GHZ, 0}"));
    } catch (const std::runtime_error &) {
      exceptionThrown = true;
    }
    NS_ASSERT(exceptionThrown);
    phySta->SetAttribute("ChannelSettings",
                         StringValue("{36, 0, BAND_5GHZ, 0}"));
    NS_ASSERT(phySta->GetChannelNumber() == 36);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5180);
    exceptionThrown = false;
    try {
      phySta->SetAttribute("ChannelSettings",
                           StringValue("{43, 0, BAND_5GHZ, 0}"));
    } catch (const std::runtime_error &) {
      exceptionThrown = true;
    }
    NS_ASSERT(exceptionThrown);
    phySta->SetAttribute("ChannelSettings",
                         StringValue("{36, 0, BAND_5GHZ, 0}"));
    NS_ASSERT(phySta->GetChannelNumber() == 36);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5180);
    PrintAttributesIfEnabled(printAttributes);
    break;
  case 20:
    Config::SetDefault("ns3::WifiPhy::ChannelSettings",
                       StringValue("{40, 0, BAND_5GHZ, 0}"));
    wifi.SetStandard(WIFI_STANDARD_80211n);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    NS_ASSERT(phySta->GetChannelNumber() == 40);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5200);
    wifi.SetStandard(WIFI_STANDARD_80211n);
    staDevice = wifi.Install(phy, macSta, wifiStaNode.Get(0));
    apDevice = wifi.Install(phy, macAp, wifiApNode.Get(0));
    phySta = GetYansWifiPhyPtr(staDevice);
    phySta->SetAttribute("ChannelSettings",
                         StringValue("{40, 0, BAND_5GHZ, 0}"));
    NS_ASSERT(phySta->GetChannelNumber() == 40);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5200);
    exceptionThrown = false;
    try {
      phySta->SetAttribute("ChannelSettings",
                           StringValue("{45, 0, BAND_5GHZ, 0}"));
    } catch (const std::runtime_error &) {
      exceptionThrown = true;
    }
    phySta->SetAttribute("ChannelSettings",
                         StringValue("{36, 0, BAND_5GHZ, 0}"));
    NS_ASSERT(phySta->GetChannelNumber() == 36);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5180);
    NS_ASSERT(exceptionThrown);
    phySta->SetAttribute("ChannelSettings",
                         StringValue("{36, 0, BAND_5GHZ, 0}"));
    exceptionThrown = false;
    try {
      phySta->SetAttribute("ChannelSettings",
                           StringValue("{43, 0, BAND_5GHZ, 0}"));
    } catch (const std::runtime_error &) {
      exceptionThrown = true;
    }
    NS_ASSERT(phySta->GetChannelNumber() == 36);
    NS_ASSERT(phySta->GetChannelWidth() == 20);
    NS_ASSERT(phySta->GetFrequency() == 5180);
    NS_ASSERT(exceptionThrown);
    PrintAttributesIfEnabled(printAttributes);
    break;
  default:
    std::cerr << "Invalid testcase number " << testCase << std::endl;
    exit(1);
    break;
  }

  Simulator::Destroy();
}
