

#include <ns3/constant-position-mobility-model.h>
#include <ns3/core-module.h>
#include <ns3/log.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/packet.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>

#include <iostream>

using namespace ns3;

static void ScanConfirm(MlmeScanConfirmParams params) {
  if (params.m_status == MLMESCAN_SUCCESS && params.m_scanType == MLMESCAN_ED) {
    std::cout << Simulator::Now().As(Time::S) << "| Scan status SUCCESSFUL\n";
    std::cout << "Results for Energy Scan:"
              << "\nPage: " << params.m_chPage << "\n";
    for (std::size_t i = 0; i < params.m_energyDetList.size(); i++) {
      std::cout << "Channel " << static_cast<uint32_t>(i + 11) << ": "
                << +params.m_energyDetList[i] << "\n";
    }
  }
}

int main(int argc, char *argv[]) {
  LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC));

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();
  Ptr<Node> n2 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev2 = CreateObject<LrWpanNetDevice>();

  dev0->SetAddress(Mac16Address("00:01"));
  dev1->SetAddress(Mac16Address("00:02"));
  dev2->SetAddress(Mac16Address("00:03"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  dev0->SetChannel(channel);
  dev1->SetChannel(channel);
  dev2->SetChannel(channel);

  n0->AddDevice(dev0);
  n1->AddDevice(dev1);
  n2->AddDevice(dev2);

  MlmeScanConfirmCallback scb;
  scb = MakeCallback(&ScanConfirm);
  dev1->GetMac()->SetMlmeScanConfirmCallback(scb);

  Ptr<ConstantPositionMobilityModel> PanCoordinatorN0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  PanCoordinatorN0Mobility->SetPosition(Vector(0, 0, 0));
  dev0->GetPhy()->SetMobility(PanCoordinatorN0Mobility);

  Ptr<ConstantPositionMobilityModel> endDeviceN1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  endDeviceN1Mobility->SetPosition(Vector(10, 0, 0));
  dev1->GetPhy()->SetMobility(endDeviceN1Mobility);

  Ptr<ConstantPositionMobilityModel> PanCoordinatorN2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  PanCoordinatorN2Mobility->SetPosition(Vector(40, 0, 0));
  dev2->GetPhy()->SetMobility(PanCoordinatorN2Mobility);

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 3;
  params.m_sfrmOrd = 3;
  params.m_logCh = 12;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 dev0->GetMac(), params);

  MlmeStartRequestParams params2;
  params2.m_panCoor = true;
  params2.m_PanId = 7;
  params2.m_bcnOrd = 3;
  params2.m_sfrmOrd = 3;
  params2.m_logCh = 14;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 dev2->GetMac(), params2);

  MlmeScanRequestParams scanParams;
  scanParams.m_chPage = 0;
  scanParams.m_scanChannels = 0x7800;
  scanParams.m_scanDuration = 14;
  scanParams.m_scanType = MLMESCAN_ED;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeScanRequest,
                                 dev1->GetMac(), scanParams);

  Simulator::Stop(Seconds(2000));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
