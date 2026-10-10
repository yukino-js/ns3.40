
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

  NodeContainer tvTransmitterNodes;
  NodeContainer spectrumAnalyzerNodes;
  NodeContainer allNodes;
  tvTransmitterNodes.Create(2);
  spectrumAnalyzerNodes.Create(1);
  allNodes.Add(tvTransmitterNodes);
  allNodes.Add(spectrumAnalyzerNodes);
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> nodePositionList =
      CreateObject<ListPositionAllocator>();
  nodePositionList->Add(Vector(128000.0, 0.0, 0.0));
  nodePositionList->Add(Vector(0.0, 24000.0, 0.0));
  nodePositionList->Add(Vector(0.0, 0.0, 0.0));
  mobility.SetPositionAllocator(nodePositionList);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(allNodes);

  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default();
  channelHelper.SetChannel("ns3::MultiModelSpectrumChannel");
  channelHelper.AddSpectrumPropagationLoss(
      "ns3::ConstantSpectrumPropagationLossModel");
  Ptr<SpectrumChannel> channel = channelHelper.Create();

  TvSpectrumTransmitterHelper tvTransHelper;
  tvTransHelper.SetChannel(channel);
  tvTransHelper.SetAttribute("StartFrequency", DoubleValue(524e6));
  tvTransHelper.SetAttribute("ChannelBandwidth", DoubleValue(6e6));
  tvTransHelper.SetAttribute("StartingTime", TimeValue(Seconds(0)));
  tvTransHelper.SetAttribute("TransmitDuration", TimeValue(Seconds(0.2)));
  tvTransHelper.SetAttribute("BasePsd", DoubleValue(22.22));
  tvTransHelper.SetAttribute("TvType",
                             EnumValue(TvSpectrumTransmitter::TVTYPE_8VSB));
  tvTransHelper.SetAttribute("Antenna",
                             StringValue("ns3::IsotropicAntennaModel"));
  tvTransHelper.InstallAdjacent(tvTransmitterNodes);

  std::vector<double> freqs;
  freqs.reserve(200);
  for (int i = 0; i < 200; ++i) {
    freqs.push_back((i + 5200) * 1e5);
  }
  Ptr<SpectrumModel> spectrumAnalyzerFreqModel = Create<SpectrumModel>(freqs);

  SpectrumAnalyzerHelper spectrumAnalyzerHelper;
  spectrumAnalyzerHelper.SetChannel(channel);
  spectrumAnalyzerHelper.SetRxSpectrumModel(spectrumAnalyzerFreqModel);
  spectrumAnalyzerHelper.SetPhyAttribute("NoisePowerSpectralDensity",
                                         DoubleValue(1e-15));
  spectrumAnalyzerHelper.EnableAsciiAll("spectrum-analyzer-tv-sim");
  NetDeviceContainer spectrumAnalyzerDevices =
      spectrumAnalyzerHelper.Install(spectrumAnalyzerNodes);

  Simulator::Stop(Seconds(0.4));

  Simulator::Run();

  Simulator::Destroy();

  std::cout << "simulation done!" << std::endl;
  std::cout << "see spectrum analyzer output file" << std::endl;

  return 0;
}
