
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include <ns3/buildings-helper.h>

using namespace ns3;

int main(int argc, char *argv[]) {
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(false));

  double eNbTxPower = 30;
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(eNbTxPower));
  Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue(10.0));
  Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl",
                     BooleanValue(true));

  Config::SetDefault("ns3::LteUePowerControl::ClosedLoop", BooleanValue(true));
  Config::SetDefault("ns3::LteUePowerControl::AccumulationEnabled",
                     BooleanValue(true));
  Config::SetDefault("ns3::LteUePowerControl::Alpha", DoubleValue(1.0));

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

  uint16_t bandwidth = 25;
  double d1 = 0;

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);
  NodeContainer allNodes = NodeContainer(enbNodes, ueNodes);

  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(d1, 0.0, 0.0));

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(allNodes);

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(bandwidth));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(bandwidth));

  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer(q);
  lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  Simulator::Stop(Seconds(0.500));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
