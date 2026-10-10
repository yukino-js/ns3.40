

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

void BeaconIndication(MlmeBeaconNotifyIndicationParams params) {
  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << " secs | Received BEACON packet of size ");
}

void DataIndication(McpsDataIndicationParams params, Ptr<Packet> p) {
  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << " secs | Received DATA packet of size " << p->GetSize());
}

void TransEndIndication(McpsDataConfirmParams params) {
  if (params.m_status == LrWpanMcpsDataConfirmStatus::IEEE_802_15_4_SUCCESS) {
    NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                  << " secs | Transmission successfully sent");
  }
}

void DataIndicationCoordinator(McpsDataIndicationParams params, Ptr<Packet> p) {
  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                << "s Coordinator Received DATA packet (size " << p->GetSize()
                << " bytes)");
}

void StartConfirm(MlmeStartConfirmParams params) {
  if (params.m_status == MLMESTART_SUCCESS) {
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "Beacon status SUCCESSFUL");
  }
}

int main(int argc, char *argv[]) {
  LogComponentEnableAll(LOG_PREFIX_TIME);
  LogComponentEnableAll(LOG_PREFIX_FUNC);
  LogComponentEnable("LrWpanMac", LOG_LEVEL_INFO);
  LogComponentEnable("LrWpanCsmaCa", LOG_LEVEL_INFO);

  LrWpanHelper lrWpanHelper;

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();

  dev0->SetAddress(Mac16Address("00:01"));
  dev1->SetAddress(Mac16Address("00:02"));

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

  n0->AddDevice(dev0);
  n1->AddDevice(dev1);

  Ptr<ConstantPositionMobilityModel> sender0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender0Mobility->SetPosition(Vector(0, 0, 0));
  dev0->GetPhy()->SetMobility(sender0Mobility);
  Ptr<ConstantPositionMobilityModel> sender1Mobility =
      CreateObject<ConstantPositionMobilityModel>();

  sender1Mobility->SetPosition(Vector(0, 10, 0));
  dev1->GetPhy()->SetMobility(sender1Mobility);

  MlmeStartConfirmCallback cb0;
  cb0 = MakeCallback(&StartConfirm);
  dev0->GetMac()->SetMlmeStartConfirmCallback(cb0);

  McpsDataConfirmCallback cb1;
  cb1 = MakeCallback(&TransEndIndication);
  dev1->GetMac()->SetMcpsDataConfirmCallback(cb1);

  MlmeBeaconNotifyIndicationCallback cb3;
  cb3 = MakeCallback(&BeaconIndication);
  dev1->GetMac()->SetMlmeBeaconNotifyIndicationCallback(cb3);

  McpsDataIndicationCallback cb4;
  cb4 = MakeCallback(&DataIndication);
  dev1->GetMac()->SetMcpsDataIndicationCallback(cb4);

  McpsDataIndicationCallback cb5;
  cb5 = MakeCallback(&DataIndicationCoordinator);
  dev0->GetMac()->SetMcpsDataIndicationCallback(cb5);

  dev1->GetMac()->SetPanId(5);
  dev1->GetMac()->SetAssociatedCoor(Mac16Address("00:01"));

  MlmeStartRequestParams params;
  params.m_panCoor = true;
  params.m_PanId = 5;
  params.m_bcnOrd = 14;
  params.m_sfrmOrd = 6;
  Simulator::ScheduleWithContext(1, Seconds(2.0), &LrWpanMac::MlmeStartRequest,
                                 dev0->GetMac(), params);

  Ptr<Packet> p1 = Create<Packet>(5);
  McpsDataRequestParams params2;
  params2.m_dstPanId = 5;
  params2.m_srcAddrMode = SHORT_ADDR;
  params2.m_dstAddrMode = SHORT_ADDR;
  params2.m_dstAddr = Mac16Address("00:01");
  params2.m_msduHandle = 0;

  Simulator::ScheduleWithContext(1, Seconds(2.93), &LrWpanMac::McpsDataRequest,
                                 dev1->GetMac(), params2, p1);

  Simulator::Stop(Seconds(600));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
