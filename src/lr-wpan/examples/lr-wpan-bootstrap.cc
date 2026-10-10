

#include <ns3/core-module.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/mobility-module.h>
#include <ns3/netanim-module.h>
#include <ns3/network-module.h>
#include <ns3/propagation-module.h>
#include <ns3/spectrum-module.h>

#include <iostream>

using namespace ns3;

NodeContainer nodes;
NodeContainer coordinators;
AnimationInterface *anim = nullptr;

static void UpdateAnimation() {
  std::cout << Simulator::Now().As(Time::S)
            << " | Animation Updated, End of simulation.\n";
  for (uint32_t i = 0; i < nodes.GetN(); ++i) {
    anim->UpdateNodeSize(i, 5, 5);
    Ptr<Node> node = nodes.Get(i);
    Ptr<NetDevice> netDevice = node->GetDevice(0);
    Ptr<LrWpanNetDevice> lrwpanDevice = DynamicCast<LrWpanNetDevice>(netDevice);
    int panId = lrwpanDevice->GetMac()->GetPanId();

    switch (panId) {
    case 5:
      anim->UpdateNodeColor(node, 0, 0, 255);
      break;
    case 7:
      anim->UpdateNodeColor(node, 0, 51, 102);
      break;
    default:
      break;
    }
  }
}

static void ScanConfirm(Ptr<LrWpanNetDevice> device,
                        MlmeScanConfirmParams params) {

  if (params.m_status == MLMESCAN_SUCCESS) {
    int maxLqi = 0;
    int panDescIndex = 0;
    if (!params.m_panDescList.empty()) {
      for (uint32_t i = 0; i < params.m_panDescList.size(); i++) {
        if (params.m_panDescList[i].m_linkQuality > maxLqi) {
          maxLqi = params.m_panDescList[i].m_linkQuality;
          panDescIndex = i;
        }
      }

      if (params.m_panDescList[panDescIndex].m_superframeSpec.IsAssocPermit()) {
        std::string addressing;
        if (params.m_panDescList[panDescIndex].m_coorAddrMode == SHORT_ADDR) {
          addressing = "Short";
        } else if (params.m_panDescList[panDescIndex].m_coorAddrMode ==
                   EXT_ADDR) {
          addressing = "Ext";
        }

        std::cout << Simulator::Now().As(Time::S) << " Node "
                  << device->GetNode()->GetId() << " ["
                  << device->GetMac()->GetShortAddress() << " | "
                  << device->GetMac()->GetExtendedAddress() << "]"
                  << " MLME-scan.confirm:  Selected PAN ID "
                  << params.m_panDescList[panDescIndex].m_coorPanId
                  << "| Coord addressing mode: " << addressing << " | LQI "
                  << static_cast<int>(
                         params.m_panDescList[panDescIndex].m_linkQuality)
                  << "\n";

        if (params.m_panDescList[panDescIndex].m_linkQuality >= 127) {
          MlmeAssociateRequestParams assocParams;
          assocParams.m_chNum = params.m_panDescList[panDescIndex].m_logCh;
          assocParams.m_chPage = params.m_panDescList[panDescIndex].m_logChPage;
          assocParams.m_coordPanId =
              params.m_panDescList[panDescIndex].m_coorPanId;
          assocParams.m_coordAddrMode =
              params.m_panDescList[panDescIndex].m_coorAddrMode;

          if (params.m_panDescList[panDescIndex].m_coorAddrMode ==
              LrWpanAddressMode::SHORT_ADDR) {
            assocParams.m_coordAddrMode = LrWpanAddressMode::SHORT_ADDR;
            assocParams.m_coordShortAddr =
                params.m_panDescList[panDescIndex].m_coorShortAddr;
            assocParams.m_capabilityInfo.SetShortAddrAllocOn(true);
          } else if (assocParams.m_coordAddrMode ==
                     LrWpanAddressMode::EXT_ADDR) {
            assocParams.m_coordAddrMode = LrWpanAddressMode::EXT_ADDR;
            assocParams.m_coordExtAddr =
                params.m_panDescList[panDescIndex].m_coorExtAddr;
            assocParams.m_coordShortAddr = Mac16Address("ff:fe");
            assocParams.m_capabilityInfo.SetShortAddrAllocOn(false);
          }

          Simulator::ScheduleNow(&LrWpanMac::MlmeAssociateRequest,
                                 device->GetMac(), assocParams);
        } else {
          std::cout << Simulator::Now().As(Time::S) << " Node "
                    << device->GetNode()->GetId() << " ["
                    << device->GetMac()->GetShortAddress() << " | "
                    << device->GetMac()->GetExtendedAddress() << "]"
                    << " MLME-scan.confirm: Beacon found but link quality too "
                       "low for "
                       "association.\n";
        }
      }
    } else {
      std::cout << Simulator::Now().As(Time::S) << " Node "
                << device->GetNode()->GetId() << " ["
                << device->GetMac()->GetShortAddress() << " | "
                << device->GetMac()->GetExtendedAddress()
                << "] MLME-scan.confirm: Beacon not found.\n";
    }
  } else {
    std::cout << Simulator::Now().As(Time::S) << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress()
              << "]  error occurred, scan failed.\n";
  }
}

static void AssociateIndication(Ptr<LrWpanNetDevice> device,
                                MlmeAssociateIndicationParams params) {

  MlmeAssociateResponseParams assocRespParams;

  assocRespParams.m_extDevAddr = params.m_extDevAddr;
  assocRespParams.m_status = LrWpanAssociationStatus::ASSOCIATED;
  if (params.capabilityInfo.IsShortAddrAllocOn()) {
    uint8_t buffer64MacAddr[8];
    uint8_t buffer16MacAddr[2];

    params.m_extDevAddr.CopyTo(buffer64MacAddr);
    buffer16MacAddr[1] = buffer64MacAddr[7];
    buffer16MacAddr[0] = buffer64MacAddr[6];

    Mac16Address shortAddr;
    shortAddr.CopyFrom(buffer16MacAddr);
    assocRespParams.m_assocShortAddr = shortAddr;
  } else {
    assocRespParams.m_assocShortAddr = Mac16Address("ff:fe");
  }

  Simulator::ScheduleNow(&LrWpanMac::MlmeAssociateResponse, device->GetMac(),
                         assocRespParams);
}

static void CommStatusIndication(Ptr<LrWpanNetDevice> device,
                                 MlmeCommStatusIndicationParams params) {
  switch (params.m_status) {
  case LrWpanMlmeCommStatus::MLMECOMMSTATUS_TRANSACTION_EXPIRED:
    std::cout << Simulator::Now().As(Time::S) << " Coordinator "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress() << "]"
              << " MLME-comm-status.indication: Transaction for device "
              << params.m_dstExtAddr
              << " EXPIRED in pending transaction list\n";
    break;
  case LrWpanMlmeCommStatus::MLMECOMMSTATUS_NO_ACK:
    std::cout << Simulator::Now().As(Time::S) << " Coordinator "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress() << "]"
              << " MLME-comm-status.indication: NO ACK from "
              << params.m_dstExtAddr
              << " device registered in the pending transaction list\n";
    break;

  case LrWpanMlmeCommStatus::MLMECOMMSTATUS_CHANNEL_ACCESS_FAILURE:
    std::cout << Simulator::Now().As(Time::S) << " Coordinator "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress() << "]"
              << " MLME-comm-status.indication: CHANNEL ACCESS problem in "
                 "transaction for "
              << params.m_dstExtAddr
              << " registered in the pending transaction list\n";
    break;

  default:
    break;
  }
}

static void AssociateConfirm(Ptr<LrWpanNetDevice> device,
                             MlmeAssociateConfirmParams params) {
  if (params.m_status == LrWpanMlmeAssociateConfirmStatus::MLMEASSOC_SUCCESS) {
    std::cout
        << Simulator::Now().As(Time::S) << " Node "
        << device->GetNode()->GetId() << " ["
        << device->GetMac()->GetShortAddress() << " | "
        << device->GetMac()->GetExtendedAddress() << "]"
        << " MLME-associate.confirm: Association with coordinator successful."
        << " (PAN: " << device->GetMac()->GetPanId()
        << " | CoordShort: " << device->GetMac()->GetCoordShortAddress()
        << " | CoordExt: " << device->GetMac()->GetCoordExtAddress() << ")\n";
  } else if (params.m_status ==
             LrWpanMlmeAssociateConfirmStatus::MLMEASSOC_NO_ACK) {
    std::cout << Simulator::Now().As(Time::S) << " Node "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress() << "]"
              << " MLME-associate.confirm: Association with coordinator FAILED "
                 "(NO ACK).\n";
  } else {
    std::cout
        << Simulator::Now().As(Time::S) << " Node "
        << device->GetNode()->GetId() << " ["
        << device->GetMac()->GetShortAddress() << " | "
        << device->GetMac()->GetExtendedAddress() << "]"
        << " MLME-associate.confirm: Association with coordinator FAILED.\n";
  }
}

static void PollConfirm(Ptr<LrWpanNetDevice> device,
                        MlmePollConfirmParams params) {
  if (params.m_status ==
      LrWpanMlmePollConfirmStatus::MLMEPOLL_CHANNEL_ACCESS_FAILURE) {
    std::cout << Simulator::Now().As(Time::S) << " Node "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress() << "]"
              << " MLME-poll.confirm:  CHANNEL ACCESS problem when sending a "
                 "data request command.\n";
  } else if (params.m_status == LrWpanMlmePollConfirmStatus::MLMEPOLL_NO_ACK) {
    std::cout << Simulator::Now().As(Time::S) << " Node "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress() << "]"
              << " MLME-poll.confirm: Data Request Command FAILED (NO ACK).\n";
  } else if (params.m_status != LrWpanMlmePollConfirmStatus::MLMEPOLL_SUCCESS) {
    std::cout << Simulator::Now().As(Time::S) << " Node "
              << device->GetNode()->GetId() << " ["
              << device->GetMac()->GetShortAddress() << " | "
              << device->GetMac()->GetExtendedAddress() << "]"
              << " MLME-poll.confirm: Data Request command FAILED.\n";
  }
}

int main(int argc, char *argv[]) {
  LogComponentEnableAll(
      LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));

  nodes.Create(100);
  coordinators.Create(2);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
      DoubleValue(0.0), "DeltaX", DoubleValue(30.0), "DeltaY",
      DoubleValue(30.0), "GridWidth", UintegerValue(20), "LayoutType",
      StringValue("RowFirst"));

  mobility.Install(nodes);

  Ptr<ListPositionAllocator> listPositionAlloc =
      CreateObject<ListPositionAllocator>();
  listPositionAlloc->Add(Vector(210, 50, 0));
  listPositionAlloc->Add(Vector(360, 50, 0));

  mobility.SetPositionAllocator(listPositionAlloc);
  mobility.Install(coordinators);

  Ptr<SingleModelSpectrumChannel> channel =
      CreateObject<SingleModelSpectrumChannel>();
  Ptr<LogDistancePropagationLossModel> propModel =
      CreateObject<LogDistancePropagationLossModel>();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();

  channel->AddPropagationLossModel(propModel);
  channel->SetPropagationDelayModel(delayModel);

  LrWpanHelper lrWpanHelper;
  lrWpanHelper.SetChannel(channel);

  NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
  lrwpanDevices.Add(lrWpanHelper.Install(coordinators));

  lrWpanHelper.SetExtendedAddresses(lrwpanDevices);

  for (auto i = nodes.Begin(); i != nodes.End(); i++) {
    Ptr<Node> node = *i;
    Ptr<NetDevice> netDevice = node->GetDevice(0);
    Ptr<LrWpanNetDevice> lrwpanDevice = DynamicCast<LrWpanNetDevice>(netDevice);
    lrwpanDevice->GetMac()->SetMlmeScanConfirmCallback(
        MakeBoundCallback(&ScanConfirm, lrwpanDevice));
    lrwpanDevice->GetMac()->SetMlmeAssociateConfirmCallback(
        MakeBoundCallback(&AssociateConfirm, lrwpanDevice));
    lrwpanDevice->GetMac()->SetMlmePollConfirmCallback(
        MakeBoundCallback(&PollConfirm, lrwpanDevice));

    MlmeScanRequestParams scanParams;
    scanParams.m_chPage = 0;
    scanParams.m_scanChannels = 0x7800;
    scanParams.m_scanDuration = 14;
    scanParams.m_scanType = MLMESCAN_PASSIVE;

    Time jitter =
        Seconds(2) + MilliSeconds(std::distance(nodes.Begin(), i) * 100);
    Simulator::ScheduleWithContext(node->GetId(), jitter,
                                   &LrWpanMac::MlmeScanRequest,
                                   lrwpanDevice->GetMac(), scanParams);
  }

  for (auto i = coordinators.Begin(); i != coordinators.End(); i++) {
    Ptr<Node> coor = *i;
    Ptr<NetDevice> netDevice = coor->GetDevice(0);
    Ptr<LrWpanNetDevice> lrwpanDevice = DynamicCast<LrWpanNetDevice>(netDevice);
    lrwpanDevice->GetMac()->SetMlmeAssociateIndicationCallback(
        MakeBoundCallback(&AssociateIndication, lrwpanDevice));
    lrwpanDevice->GetMac()->SetMlmeCommStatusIndicationCallback(
        MakeBoundCallback(&CommStatusIndication, lrwpanDevice));
  }

  Ptr<Node> coor1 = coordinators.Get(0);
  Ptr<NetDevice> netDeviceCoor1 = coor1->GetDevice(0);
  Ptr<LrWpanNetDevice> coor1Device =
      DynamicCast<LrWpanNetDevice>(netDeviceCoor1);

  Ptr<Node> coor2 = coordinators.Get(1);
  Ptr<NetDevice> netDeviceCoor2 = coor2->GetDevice(0);
  Ptr<LrWpanNetDevice> coor2Device =
      DynamicCast<LrWpanNetDevice>(netDeviceCoor2);

  coor1Device->GetMac()->SetShortAddress(Mac16Address("FF:FE"));
  coor2Device->GetMac()->SetShortAddress(Mac16Address("CA:FE"));

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 3;
  params.m_sfrmOrd = 3;
  params.m_logCh = 12;

  Simulator::ScheduleWithContext(coor1Device->GetNode()->GetId(), Seconds(2.0),
                                 &LrWpanMac::MlmeStartRequest,
                                 coor1Device->GetMac(), params);

  MlmeStartRequestParams params2;
  params2.m_panCoor = true;
  params2.m_PanId = 7;
  params2.m_bcnOrd = 3;
  params2.m_sfrmOrd = 3;
  params2.m_logCh = 14;

  Simulator::ScheduleWithContext(coor2Device->GetNode()->GetId(), Seconds(2.0),
                                 &LrWpanMac::MlmeStartRequest,
                                 coor2Device->GetMac(), params2);

  anim = new AnimationInterface("lrwpan-bootstrap.xml");
  anim->SkipPacketTracing();
  anim->UpdateNodeDescription(coordinators.Get(0), "Coordinator (PAN 5)");
  anim->UpdateNodeDescription(coordinators.Get(1), "Coordinator (PAN 7)");
  anim->UpdateNodeColor(coordinators.Get(0), 0, 0, 255);
  anim->UpdateNodeColor(coordinators.Get(1), 0, 51, 102);
  anim->UpdateNodeSize(nodes.GetN(), 9, 9);
  anim->UpdateNodeSize(nodes.GetN() + 1, 9, 9);

  Simulator::Schedule(Seconds(1499), &UpdateAnimation);
  Simulator::Stop(Seconds(1500));
  Simulator::Run();

  Simulator::Destroy();
  delete anim;
  return 0;
}
