
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include <ns3/buildings-helper.h>
#include <ns3/string.h>

#include <fstream>

using namespace ns3;

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

  lteHelper->SetAttribute("FadingModel",
                          StringValue("ns3::TraceFadingLossModel"));

  std::ifstream ifTraceFile;
  ifTraceFile.open(
      "../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad",
      std::ifstream::in);
  if (ifTraceFile.good()) {
    lteHelper->SetFadingModelAttribute(
        "TraceFilename",
        StringValue(
            "../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
  } else {
    lteHelper->SetFadingModelAttribute(
        "TraceFilename",
        StringValue("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
  }

  lteHelper->SetFadingModelAttribute("TraceLength", TimeValue(Seconds(10.0)));
  lteHelper->SetFadingModelAttribute("SamplesNum", UintegerValue(10000));
  lteHelper->SetFadingModelAttribute("WindowSize", TimeValue(Seconds(0.5)));
  lteHelper->SetFadingModelAttribute("RbNum", UintegerValue(100));

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

  Simulator::Stop(Seconds(0.005));

  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
