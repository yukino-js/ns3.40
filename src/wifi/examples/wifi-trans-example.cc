
#include "ns3/command-line.h"
#include "ns3/gnuplot.h"
#include "ns3/mobility-helper.h"
#include "ns3/spectrum-analyzer-helper.h"
#include "ns3/spectrum-channel.h"
#include "ns3/spectrum-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"

using namespace ns3;

void SendPacket(Ptr<NetDevice> sourceDevice, Address &destination) {
  Ptr<Packet> pkt = Create<Packet>(100);
  sourceDevice->Send(pkt, destination, 0);
}

int main(int argc, char **argv) {
  std::string standard = "11a";
  int bw = 20;
  double pow = 23;
  bool verbose = false;
  CommandLine cmd(__FILE__);
  cmd.AddValue("standard",
               "OFDM-based Wi-Fi standard [11a, 11p_10MHZ, 11p_5MHZ, "
               "11n_2_4GHZ, 11n_5GHZ, 11ac, "
               "11ax_2_4GHZ, 11ax_5GHZ]",
               standard);
  cmd.AddValue("bw", "Bandwidth (consistent with standard, in MHz)", bw);
  cmd.AddValue("txPower", "Transmit power (dBm)", pow);
  cmd.AddValue(
      "verbose",
      "Display log messages for WifiSpectrumValueHelper and SpectrumWifiPhy",
      verbose);
  cmd.Parse(argc, argv);

  WifiHelper wifi;
  Ssid ssid;
  std::string dataRate;
  int freq;
  Time dataStartTime = MicroSeconds(800);
  Time dataDuration = MicroSeconds(300);
  if (standard == "11a") {
    wifi.SetStandard(WIFI_STANDARD_80211a);
    ssid = Ssid("ns380211a");
    dataRate = "OfdmRate6Mbps";
    freq = 5180;
    if (bw != 20) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else if (standard == "11p_10MHZ") {
    wifi.SetStandard(WIFI_STANDARD_80211p);
    ssid = Ssid("ns380211p_10MHZ");
    dataRate = "OfdmRate3MbpsBW10MHz";
    freq = 5860;
    dataStartTime = MicroSeconds(1400);
    dataDuration = MicroSeconds(600);
    if (bw != 10) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else if (standard == "11p_5MHZ") {
    wifi.SetStandard(WIFI_STANDARD_80211p);
    ssid = Ssid("ns380211p_5MHZ");
    dataRate = "OfdmRate1_5MbpsBW5MHz";
    freq = 5860;
    dataStartTime = MicroSeconds(2500);
    dataDuration = MicroSeconds(1200);
    if (bw != 5) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else if (standard == "11n_2_4GHZ") {
    wifi.SetStandard(WIFI_STANDARD_80211n);
    ssid = Ssid("ns380211n_2_4GHZ");
    dataRate = "HtMcs0";
    freq = 2402 + (bw / 2);
    dataStartTime = MicroSeconds(4700);
    dataDuration = MicroSeconds(400);
    if (bw != 20 && bw != 40) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else if (standard == "11n_5GHZ") {
    wifi.SetStandard(WIFI_STANDARD_80211n);
    ssid = Ssid("ns380211n_5GHZ");
    dataRate = "HtMcs0";
    freq = 5170 + (bw / 2);
    dataStartTime = MicroSeconds(1000);
    if (bw != 20 && bw != 40) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else if (standard == "11ac") {
    wifi.SetStandard(WIFI_STANDARD_80211ac);
    ssid = Ssid("ns380211ac");
    dataRate = "VhtMcs0";
    freq = 5170 + (bw / 2);
    dataStartTime = MicroSeconds(1100);
    dataDuration += MicroSeconds(400);
    if (bw != 20 && bw != 40 && bw != 80 && bw != 160) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else if (standard == "11ax_2_4GHZ") {
    wifi.SetStandard(WIFI_STANDARD_80211ax);
    ssid = Ssid("ns380211ax_2_4GHZ");
    dataRate = "HeMcs0";
    freq = 2402 + (bw / 2);
    dataStartTime = MicroSeconds(5500);
    dataDuration += MicroSeconds(2000);
    if (bw != 20 && bw != 40 && bw != 80) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else if (standard == "11ax_5GHZ") {
    wifi.SetStandard(WIFI_STANDARD_80211ax);
    ssid = Ssid("ns380211ax_5GHZ");
    dataRate = "HeMcs0";
    freq = 5170 + (bw / 2);
    dataStartTime = MicroSeconds(1200);
    dataDuration += MicroSeconds(500);
    if (bw != 20 && bw != 40 && bw != 80 && bw != 160) {
      std::cout << "Bandwidth is not compatible with standard" << std::endl;
      return 1;
    }
  } else {
    std::cout
        << "Unknown OFDM standard (please refer to the listed possible values)"
        << std::endl;
    return 1;
  }

  if (verbose) {
    LogComponentEnableAll(LOG_PREFIX_ALL);
    LogComponentEnable("WifiSpectrumValueHelper", LOG_LEVEL_ALL);
    LogComponentEnable("SpectrumWifiPhy", LOG_LEVEL_ALL);
  }

  NodeContainer wifiNodes;
  NodeContainer spectrumAnalyzerNodes;
  NodeContainer allNodes;
  wifiNodes.Create(2);
  spectrumAnalyzerNodes.Create(1);
  allNodes.Add(wifiNodes);
  allNodes.Add(spectrumAnalyzerNodes);
  NodeContainer wifiStaNode;
  NodeContainer wifiApNode;
  wifiApNode.Add(wifiNodes.Get(0));
  wifiStaNode.Add(wifiNodes.Get(1));

  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default();
  channelHelper.SetChannel("ns3::MultiModelSpectrumChannel");
  channelHelper.AddSpectrumPropagationLoss(
      "ns3::ConstantSpectrumPropagationLossModel");
  Ptr<SpectrumChannel> channel = channelHelper.Create();

  SpectrumWifiPhyHelper spectrumPhy;
  spectrumPhy.SetChannel(channel);
  spectrumPhy.SetErrorRateModel("ns3::NistErrorRateModel");
  spectrumPhy.Set("Frequency", UintegerValue(freq));
  spectrumPhy.Set("ChannelWidth", UintegerValue(bw));
  spectrumPhy.Set("TxPowerStart", DoubleValue(pow));
  spectrumPhy.Set("TxPowerEnd", DoubleValue(pow));

  WifiMacHelper mac;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue(dataRate), "ControlMode",
                               StringValue(dataRate));

  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
              BooleanValue(false));
  NetDeviceContainer staDevice = wifi.Install(spectrumPhy, mac, wifiStaNode);
  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "EnableBeaconJitter",
              BooleanValue(false));
  NetDeviceContainer apDevice = wifi.Install(spectrumPhy, mac, wifiApNode);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> nodePositionList =
      CreateObject<ListPositionAllocator>();
  nodePositionList->Add(Vector(0.0, 1.0, 0.0));
  nodePositionList->Add(Vector(1.0, 0.0, 0.0));
  nodePositionList->Add(Vector(0.0, 0.0, 0.0));
  mobility.SetPositionAllocator(nodePositionList);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(allNodes);

  Simulator::Schedule(dataStartTime, &SendPacket, apDevice.Get(0),
                      staDevice.Get(0)->GetAddress());

  std::vector<double> freqs;
  int margin = 2;
  int band = (bw + margin);
  freqs.reserve((4 * 10 * band));
  for (int i = 0; i < (4 * 10 * band); ++i) {
    freqs.push_back(i * 1e5 + (freq - 2 * band) * 1e6);
  }
  Ptr<SpectrumModel> spectrumAnalyzerFreqModel = Create<SpectrumModel>(freqs);

  SpectrumAnalyzerHelper spectrumAnalyzerHelper;
  spectrumAnalyzerHelper.SetChannel(channel);
  spectrumAnalyzerHelper.SetRxSpectrumModel(spectrumAnalyzerFreqModel);
  spectrumAnalyzerHelper.SetPhyAttribute("Resolution",
                                         TimeValue(MicroSeconds(4)));
  std::ostringstream ossFileName;
  ossFileName << "spectrum-analyzer-wifi-" << standard << "-" << bw << "MHz";
  spectrumAnalyzerHelper.EnableAsciiAll(ossFileName.str());
  NetDeviceContainer spectrumAnalyzerDevices =
      spectrumAnalyzerHelper.Install(spectrumAnalyzerNodes);

  Simulator::Stop(dataStartTime + dataDuration);

  Simulator::Run();

  ossFileName << "-2-0";
  std::ostringstream ossPlt;
  ossPlt << ossFileName.str() << ".plt";
  std::ofstream plotFile(ossPlt.str());
  std::ostringstream ossPng;
  ossPng << ossFileName.str() << ".png";
  Gnuplot plot = Gnuplot(ossPng.str());
  std::ostringstream ossExtra;
  ossExtra << "file = '" << ossFileName.str() << "'";
  plot.SetExtra(ossExtra.str());
  plot.AppendExtra("unset surface");
  plot.AppendExtra("set key off");
  plot.AppendExtra("set term png");
  plot.AppendExtra("set output file . '.png'");
  plot.AppendExtra("set pm3d at s");
  plot.AppendExtra("set palette");
  plot.AppendExtra("set view 50,50");
  plot.AppendExtra("set xlabel \"time (ms)\"");
  plot.AppendExtra("set ylabel \"freq (MHz)\" offset 15,0,0");
  plot.AppendExtra("set zlabel \"PSD (dBW/Hz)\" offset 15,0,0");
  plot.AppendExtra("set ytics");
  plot.AppendExtra("set mytics 2");
  plot.AppendExtra("set ztics");
  plot.AppendExtra("set mztics 5");
  plot.AppendExtra("set grid ytics mytics ztics mztics");
  plot.AppendExtra("filename = file . '.tr'");
  plot.AppendExtra("stats filename using 3");
  plot.AppendExtra("refW = STATS_max");
  plot.AppendExtra(
      "splot filename using ($1*1000.0):($2/1e6):(10*log10($3/refW))");
  plot.GenerateOutput(plotFile);
  plotFile.close();

  Simulator::Destroy();

  std::cout << "Simulation done!" << std::endl;
  std::cout << "See spectrum analyzer output file: " << ossFileName.str()
            << ".tr" << std::endl;
  std::cout << "To generate plot simply execute the following command: gnuplot "
            << ossFileName.str() << ".plt" << std::endl;

  return 0;
}
