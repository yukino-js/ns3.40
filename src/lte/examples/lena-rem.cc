
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/spectrum-module.h"
#include <ns3/buildings-helper.h>

using namespace ns3;

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  BuildingsHelper::Install(enbNodes);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(ueNodes);
  BuildingsHelper::Install(ueNodes);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;

  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer(q);
  lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  Ptr<RadioEnvironmentMapHelper> remHelper =
      CreateObject<RadioEnvironmentMapHelper>();
  remHelper->SetAttribute("ChannelPath", StringValue("/ChannelList/0"));
  remHelper->SetAttribute("OutputFile", StringValue("rem.out"));
  remHelper->SetAttribute("XMin", DoubleValue(-400.0));
  remHelper->SetAttribute("XMax", DoubleValue(400.0));
  remHelper->SetAttribute("YMin", DoubleValue(-300.0));
  remHelper->SetAttribute("YMax", DoubleValue(300.0));
  remHelper->SetAttribute("Z", DoubleValue(0.0));
  remHelper->Install();

  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
