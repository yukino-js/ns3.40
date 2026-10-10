

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
    std::cout << Simulator::Now().As(Time::S) << " Node "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress()
              << "] MLME-SCAN.confirm: Active scan status SUCCESSFUL "
              << "(Coordinator found and address assigned) \n";
  } else if (params.m_status == MLMESCAN_NO_BEACON) {
    std::cout << Simulator::Now().As(Time::S) << " Node "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress()
              << "] MLME-SCAN.confirm: Could not locate coordinator "
              << "(Coord realignment command not received) "
              << "status: " << params.m_status << "\n";
  } else {
    std::cout << Simulator::Now().As(Time::S) << " Node "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress()
              << "] MLME-SCAN.confirm: An error occurred during scanning, "
              << "status: " << params.m_status << "\n";
  }
}

static void OrphanIndication(Ptr<LrWpanNetDevice> device,
                             MlmeOrphanIndicationParams params) {

  std::cout << Simulator::Now().As(Time::S) << " Node "
            << device->GetNode()->GetId() << " ["
            << device->GetMac()->GetShortAddress() << " | "
            << device->GetMac()->GetExtendedAddress()
            << "] MLME-ORPHAN.indication: Orphan Notification received, "
               "processing...\n";

  MlmeOrphanResponseParams respParams;
  respParams.m_assocMember = true;
  respParams.m_orphanAddr = params.m_orphanAddr;
  respParams.m_shortAddr = Mac16Address("DE:AF");

  Simulator::ScheduleNow(&LrWpanMac::MlmeOrphanResponse, device->GetMac(),
                         respParams);
}

int main(int argc, char *argv[]) {
  LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC));

  Ptr<Node> coord1 = CreateObject<Node>();
  Ptr<Node> endNode = CreateObject<Node>();

  Ptr<LrWpanNetDevice> coord1NetDevice = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> endNodeNetDevice = CreateObject<LrWpanNetDevice>();

  coord1NetDevice->GetMac()->SetExtendedAddress(
      Mac64Address("00:00:00:00:00:00:00:01"));
  coord1NetDevice->GetMac()->SetShortAddress(Mac16Address("00:01"));

  endNodeNetDevice->GetMac()->SetExtendedAddress(
      Mac64Address("00:00:00:00:00:00:00:02"));

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

  coord1->AddDevice(coord1NetDevice);
  endNode->AddDevice(endNodeNetDevice);

  Ptr<ConstantPositionMobilityModel> coord1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  coord1Mobility->SetPosition(Vector(0, 0, 0));
  coord1NetDevice->GetPhy()->SetMobility(coord1Mobility);

  Ptr<ConstantPositionMobilityModel> endNodeMobility =
      CreateObject<ConstantPositionMobilityModel>();
  endNodeMobility->SetPosition(Vector(100, 0, 0));
  endNodeNetDevice->GetPhy()->SetMobility(endNodeMobility);

  endNodeNetDevice->GetMac()->SetMlmeScanConfirmCallback(
      MakeBoundCallback(&ScanConfirm, endNodeNetDevice));

  coord1NetDevice->GetMac()->SetMlmeOrphanIndicationCallback(
      MakeBoundCallback(&OrphanIndication, coord1NetDevice));

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 15;
  params.m_sfrmOrd = 15;
  params.m_logCh = 12;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 coord1NetDevice->GetMac(), params);

  MlmeScanRequestParams scanParams;
  scanParams.m_chPage = 0;
  scanParams.m_scanChannels = 0x7800;
  scanParams.m_scanType = MLMESCAN_ORPHAN;
  Simulator::ScheduleWithContext(1, Seconds(3.0), &LrWpanMac::MlmeScanRequest,
                                 endNodeNetDevice->GetMac(), scanParams);

  Simulator::Stop(Seconds(2000));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
