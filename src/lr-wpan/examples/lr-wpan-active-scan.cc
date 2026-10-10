

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

static void ScanConfirm(Ptr<LrWpanNetDevice> device,
                        MlmeScanConfirmParams params) {
  if (params.m_status == MLMESCAN_SUCCESS) {
    std::cout << Simulator::Now().As(Time::S)
              << "| Active scan status SUCCESSFUL (Completed)"
              << "\n";
    if (!params.m_panDescList.empty()) {
      std::cout << "Device [" << device->GetMac()->GetShortAddress()
                << "] found the following PANs:\n";
      for (long unsigned int i = 0; i < params.m_panDescList.size(); i++) {
        std::cout << "PAN DESCRIPTOR " << i << ":\n"
                  << "Pan Id: " << params.m_panDescList[i].m_coorPanId
                  << "\nChannel: "
                  << static_cast<uint32_t>(params.m_panDescList[i].m_logCh)
                  << "\nLQI: "
                  << static_cast<uint32_t>(
                         params.m_panDescList[i].m_linkQuality)
                  << "\nCoordinator Short Address: "
                  << params.m_panDescList[i].m_coorShortAddr << "\n\n";
      }
    } else {
      std::cout << "No PANs found (Could not find any beacons)\n";
    }
  } else {
    std::cout << "Something went wrong, scan could not be completed\n";
  }
}

int main(int argc, char *argv[]) {
  LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC));

  Ptr<Node> coord1 = CreateObject<Node>();
  Ptr<Node> endNode = CreateObject<Node>();
  Ptr<Node> coord2 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> coord1NetDevice = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> endNodeNetDevice = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> coord2NetDevice = CreateObject<LrWpanNetDevice>();

  coord1NetDevice->SetAddress(Mac16Address("00:01"));
  endNodeNetDevice->SetAddress(Mac16Address("00:02"));
  coord2NetDevice->SetAddress(Mac16Address("00:03"));

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  coord1NetDevice->SetChannel(channel);
  endNodeNetDevice->SetChannel(channel);
  coord2NetDevice->SetChannel(channel);

  coord1->AddDevice(coord1NetDevice);
  endNode->AddDevice(endNodeNetDevice);
  coord2->AddDevice(coord2NetDevice);

  Ptr<ConstantPositionMobilityModel> coord1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  coord1Mobility->SetPosition(Vector(0, 0, 0));
  coord1NetDevice->GetPhy()->SetMobility(coord1Mobility);

  Ptr<ConstantPositionMobilityModel> endNodeMobility =
      CreateObject<ConstantPositionMobilityModel>();
  endNodeMobility->SetPosition(Vector(100, 0, 0));
  endNodeNetDevice->GetPhy()->SetMobility(endNodeMobility);

  Ptr<ConstantPositionMobilityModel> coord2Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  coord2Mobility->SetPosition(Vector(206, 0, 0));
  coord2NetDevice->GetPhy()->SetMobility(coord2Mobility);

  endNodeNetDevice->GetMac()->SetMlmeScanConfirmCallback(
      MakeBoundCallback(&ScanConfirm, endNodeNetDevice));

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 15;
  params.m_sfrmOrd = 15;
  params.m_logCh = 12;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 coord1NetDevice->GetMac(), params);

  MlmeStartRequestParams params2;
  params2.m_panCoor = true;
  params2.m_PanId = 7;
  params2.m_bcnOrd = 15;
  params2.m_sfrmOrd = 15;
  params2.m_logCh = 14;
  Simulator::ScheduleWithContext(2, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 coord2NetDevice->GetMac(), params2);

  MlmeScanRequestParams scanParams;
  scanParams.m_chPage = 0;
  scanParams.m_scanChannels = 0x7800;
  scanParams.m_scanDuration = 14;
  scanParams.m_scanType = MLMESCAN_ACTIVE;
  Simulator::ScheduleWithContext(1, Seconds(3.0), &LrWpanMac::MlmeScanRequest,
                                 endNodeNetDevice->GetMac(), scanParams);

  Simulator::Stop(Seconds(2000));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
