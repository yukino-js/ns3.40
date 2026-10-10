
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include <ns3/buildings-helper.h>

using namespace ns3;

int main(int argc, char *argv[]) {
  Time simTime = MilliSeconds(1050);
  bool useCa = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue("useCa", "Whether to use carrier aggregation.", useCa);
  cmd.Parse(argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  cmd.Parse(argc, argv);

  if (useCa) {
    Config::SetDefault("ns3::LteHelper::UseCa", BooleanValue(useCa));
    Config::SetDefault("ns3::LteHelper::NumberOfComponentCarriers",
                       UintegerValue(2));
    Config::SetDefault("ns3::LteHelper::EnbComponentCarrierManager",
                       StringValue("ns3::RrComponentCarrierManager"));
  }

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
  lteHelper->EnableTraces();

  Simulator::Stop(simTime);
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
