
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

static void DataIndication(McpsDataIndicationParams params, Ptr<Packet> p) {
  NS_LOG_UNCOND("Received packet of size " << p->GetSize());
}

static void DataConfirm(McpsDataConfirmParams params) {
  NS_LOG_UNCOND("LrWpanMcpsDataConfirmStatus = " << params.m_status);
}

static void StateChangeNotification(std::string context, Time now,
                                    LrWpanPhyEnumeration oldState,
                                    LrWpanPhyEnumeration newState) {
  NS_LOG_UNCOND(context << " state change at " << now.As(Time::S) << " from "
                        << LrWpanHelper::LrWpanPhyEnumerationPrinter(oldState)
                        << " to "
                        << LrWpanHelper::LrWpanPhyEnumerationPrinter(newState));
}

int main(int argc, char *argv[]) {
  bool verbose = false;
  bool extended = false;

  CommandLine cmd(__FILE__);

  cmd.AddValue("verbose", "turn on all log components", verbose);
  cmd.AddValue("extended", "use extended addressing", extended);

  cmd.Parse(argc, argv);

  LrWpanHelper lrWpanHelper;
  if (verbose) {
    lrWpanHelper.EnableLogComponents();
  }

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();

  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice>();
  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice>();

  if (!extended) {
    dev0->SetAddress(Mac16Address("00:01"));
    dev1->SetAddress(Mac16Address("00:02"));
  } else {
    Ptr<LrWpanMac> mac0 = dev0->GetMac();
    Ptr<LrWpanMac> mac1 = dev1->GetMac();
    mac0->SetExtendedAddress(Mac64Address("00:00:00:00:00:00:00:01"));
    mac1->SetExtendedAddress(Mac64Address("00:00:00:00:00:00:00:02"));
  }

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

  dev0->GetPhy()->TraceConnect("TrxState", std::string("phy0"),
                               MakeCallback(&StateChangeNotification));
  dev1->GetPhy()->TraceConnect("TrxState", std::string("phy1"),
                               MakeCallback(&StateChangeNotification));

  Ptr<ConstantPositionMobilityModel> sender0Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender0Mobility->SetPosition(Vector(0, 0, 0));
  dev0->GetPhy()->SetMobility(sender0Mobility);
  Ptr<ConstantPositionMobilityModel> sender1Mobility =
      CreateObject<ConstantPositionMobilityModel>();
  sender1Mobility->SetPosition(Vector(0, 10, 0));
  dev1->GetPhy()->SetMobility(sender1Mobility);

  McpsDataConfirmCallback cb0;
  cb0 = MakeCallback(&DataConfirm);
  dev0->GetMac()->SetMcpsDataConfirmCallback(cb0);

  McpsDataIndicationCallback cb1;
  cb1 = MakeCallback(&DataIndication);
  dev0->GetMac()->SetMcpsDataIndicationCallback(cb1);

  McpsDataConfirmCallback cb2;
  cb2 = MakeCallback(&DataConfirm);
  dev1->GetMac()->SetMcpsDataConfirmCallback(cb2);

  McpsDataIndicationCallback cb3;
  cb3 = MakeCallback(&DataIndication);
  dev1->GetMac()->SetMcpsDataIndicationCallback(cb3);

  lrWpanHelper.EnablePcapAll(std::string("lr-wpan-data"), true);
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("lr-wpan-data.tr");
  lrWpanHelper.EnableAsciiAll(stream);

  Ptr<Packet> p0 = Create<Packet>(50);
  McpsDataRequestParams params;
  params.m_dstPanId = 0;
  if (!extended) {
    params.m_srcAddrMode = SHORT_ADDR;
    params.m_dstAddrMode = SHORT_ADDR;
    params.m_dstAddr = Mac16Address("00:02");
  } else {
    params.m_srcAddrMode = EXT_ADDR;
    params.m_dstAddrMode = EXT_ADDR;
    params.m_dstExtAddr = Mac64Address("00:00:00:00:00:00:00:02");
  }
  params.m_msduHandle = 0;
  params.m_txOptions = TX_OPTION_ACK;
  Simulator::ScheduleWithContext(1, Seconds(0.0), &LrWpanMac::McpsDataRequest,
                                 dev0->GetMac(), params, p0);

  Ptr<Packet> p2 = Create<Packet>(60);
  if (!extended) {
    params.m_dstAddr = Mac16Address("00:01");
  } else {
    params.m_dstExtAddr = Mac64Address("00:00:00:00:00:00:00:01");
  }
  Simulator::ScheduleWithContext(2, Seconds(2.0), &LrWpanMac::McpsDataRequest,
                                 dev1->GetMac(), params, p2);

  Simulator::Run();

  Simulator::Destroy();
  return 0;
}
