
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include <ns3/buildings-helper.h>

using namespace ns3;

static void ChangePosition(Ptr<Node> node) {
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
  Vector pos = mobility->GetPosition();

  if (pos.x <= 10.0) {
    pos.x = 100000.0;
  } else {
    pos.x = 5.0;
  }
  mobility->SetPosition(pos);
}

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  lteHelper->SetAttribute(
      "PathlossModel", StringValue("ns3::FriisSpectrumPropagationLossModel"));

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
  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
  lteHelper->SetSchedulerAttribute("CqiTimerThreshold", UintegerValue(3));
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->EnableRlcTraces();
  lteHelper->EnableMacTraces();

  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  Simulator::Schedule(Seconds(0.010), &ChangePosition, ueNodes.Get(0));
  Simulator::Schedule(Seconds(0.020), &ChangePosition, ueNodes.Get(0));

  EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer(q);
  lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  Simulator::Stop(Seconds(0.030));

  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
