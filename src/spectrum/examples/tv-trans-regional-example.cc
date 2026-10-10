
#include <ns3/core-module.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-analyzer-helper.h>
#include <ns3/spectrum-helper.h>
#include <ns3/tv-spectrum-transmitter-helper.h>

#include <iostream>
#include <stdlib.h>

using namespace ns3;

int main(int argc, char **argv) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(3);

  NodeContainer spectrumAnalyzerNodes;
  spectrumAnalyzerNodes.Create(1);
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> nodePositionList =
      CreateObject<ListPositionAllocator>();
  Vector coordinates = GeographicPositions::GeographicToCartesianCoordinates(
      48.86, 2.35, 0, GeographicPositions::SPHERE);
  nodePositionList->Add(coordinates);
  mobility.SetPositionAllocator(nodePositionList);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(spectrumAnalyzerNodes);

  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default();
  channelHelper.SetChannel("ns3::MultiModelSpectrumChannel");
  Ptr<SpectrumChannel> channel = channelHelper.Create();

  TvSpectrumTransmitterHelper tvTransHelper;
  tvTransHelper.SetChannel(channel);
  tvTransHelper.SetAttribute("StartingTime", TimeValue(Seconds(0.1)));
  tvTransHelper.SetAttribute("TransmitDuration", TimeValue(Seconds(0.1)));
  tvTransHelper.SetAttribute("BasePsd", DoubleValue(7.96));
  tvTransHelper.SetAttribute("TvType",
                             EnumValue(TvSpectrumTransmitter::TVTYPE_COFDM));
  tvTransHelper.SetAttribute("Antenna",
                             StringValue("ns3::IsotropicAntennaModel"));

  tvTransHelper.AssignStreams(300);
  tvTransHelper.CreateRegionalTvTransmitters(
      TvSpectrumTransmitterHelper::REGION_EUROPE,
      TvSpectrumTransmitterHelper::DENSITY_MEDIUM, 48.86, 2.35, 0, 250000);

  std::vector<double> freqs;
  for (int i = 0; i < 6860; i = i + 5) {
    freqs.push_back((i + 1740) * 1e5);
  }
  Ptr<SpectrumModel> spectrumAnalyzerFreqModel = Create<SpectrumModel>(freqs);

  SpectrumAnalyzerHelper spectrumAnalyzerHelper;
  spectrumAnalyzerHelper.SetChannel(channel);
  spectrumAnalyzerHelper.SetRxSpectrumModel(spectrumAnalyzerFreqModel);
  spectrumAnalyzerHelper.SetPhyAttribute("NoisePowerSpectralDensity",
                                         DoubleValue(4.14e-21));
  spectrumAnalyzerHelper.EnableAsciiAll("spectrum-analyzer-tv-sim-regional");
  NetDeviceContainer spectrumAnalyzerDevices =
      spectrumAnalyzerHelper.Install(spectrumAnalyzerNodes);

  Simulator::Stop(Seconds(0.4));

  Simulator::Run();

  Simulator::Destroy();

  std::cout << "simulation done!" << std::endl;
  std::cout << "see spectrum analyzer output file" << std::endl;

  return 0;
}
