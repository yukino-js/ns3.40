
#include "ns3/fcfs-wifi-queue-scheduler.h"
#include "ns3/he-configuration.h"
#include "ns3/ht-configuration.h"
#include "ns3/ht-frame-exchange-manager.h"
#include "ns3/interference-helper.h"
#include "ns3/mac-tx-middle.h"
#include "ns3/mobility-helper.h"
#include "ns3/mpdu-aggregator.h"
#include "ns3/msdu-aggregator.h"
#include "ns3/node-container.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/vht-configuration.h"
#include "ns3/wifi-default-ack-manager.h"
#include "ns3/wifi-default-protection-manager.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-phy.h"
#include <ns3/attribute-container.h>

#include <algorithm>
#include <iterator>

using namespace ns3;

class AmpduAggregationTest : public TestCase {
public:
  AmpduAggregationTest();

private:
  void MpduDiscarded(WifiMacDropReason reason, Ptr<const WifiMpdu> mpdu);

  void DoRun() override;
  Ptr<WifiNetDevice> m_device;
  Ptr<StaWifiMac> m_mac;
  Ptr<YansWifiPhy> m_phy;
  Ptr<WifiRemoteStationManager> m_manager;
  ObjectFactory m_factory;
  bool m_discarded;
};

AmpduAggregationTest::AmpduAggregationTest()
    : TestCase("Check the correctness of MPDU aggregation operations"),
      m_discarded(false) {}

void AmpduAggregationTest::MpduDiscarded(WifiMacDropReason,
                                         Ptr<const WifiMpdu>) {
  m_discarded = true;
}

void AmpduAggregationTest::DoRun() {
  m_device = CreateObject<WifiNetDevice>();
  Ptr<HtConfiguration> htConfiguration = CreateObject<HtConfiguration>();
  m_device->SetHtConfiguration(htConfiguration);

  m_phy = CreateObject<YansWifiPhy>();
  Ptr<InterferenceHelper> interferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phy->SetInterferenceHelper(interferenceHelper);
  m_phy->SetDevice(m_device);
  m_phy->ConfigureStandard(WIFI_STANDARD_80211n);
  m_device->SetPhy(m_phy);

  m_factory = ObjectFactory();
  m_factory.SetTypeId("ns3::ConstantRateWifiManager");
  m_factory.Set("DataMode", StringValue("HtMcs7"));
  m_manager = m_factory.Create<WifiRemoteStationManager>();
  m_manager->SetupPhy(m_phy);
  m_device->SetRemoteStationManager(m_manager);

  m_mac = CreateObjectWithAttributes<StaWifiMac>("QosSupported",
                                                 BooleanValue(true));
  m_mac->SetDevice(m_device);
  m_mac->SetWifiRemoteStationManager(m_manager);
  m_mac->SetAddress(Mac48Address("00:00:00:00:00:01"));
  m_mac->SetWifiPhys({m_phy});
  m_mac->ConfigureStandard(WIFI_STANDARD_80211n);
  Ptr<FrameExchangeManager> fem = m_mac->GetFrameExchangeManager();
  Ptr<WifiProtectionManager> protectionManager =
      CreateObject<WifiDefaultProtectionManager>();
  protectionManager->SetWifiMac(m_mac);
  fem->SetProtectionManager(protectionManager);
  Ptr<WifiAckManager> ackManager = CreateObject<WifiDefaultAckManager>();
  ackManager->SetWifiMac(m_mac);
  fem->SetAckManager(ackManager);
  m_device->SetMac(m_mac);
  m_mac->SetState(StaWifiMac::ASSOCIATED);
  m_mac->SetMacQueueScheduler(CreateObject<FcfsWifiQueueScheduler>());

  m_mac->SetAttribute("BE_MaxAmpduSize", UintegerValue(65535));
  HtCapabilities htCapabilities;
  htCapabilities.SetMaxAmpduLength(65535);
  m_manager->AddStationHtCapabilities(Mac48Address("00:00:00:00:00:02"),
                                      htCapabilities);
  m_manager->AddStationHtCapabilities(Mac48Address("00:00:00:00:00:03"),
                                      htCapabilities);

  Ptr<const Packet> pkt = Create<Packet>(1500);
  Ptr<Packet> currentAggregatedPacket = Create<Packet>();
  WifiMacHeader hdr;
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetFragmentNumber(0);
  hdr.SetNoMoreFragments();
  hdr.SetNoRetry();

  MgtAddBaRequestHeader reqHdr;
  reqHdr.SetImmediateBlockAck();
  reqHdr.SetTid(0);
  reqHdr.SetBufferSize(64);
  reqHdr.SetTimeout(0);
  reqHdr.SetStartingSequence(0);
  m_mac->GetBEQueue()->GetBaManager()->CreateOriginatorAgreement(
      reqHdr, hdr.GetAddr1());

  MgtAddBaResponseHeader respHdr;
  StatusCode code;
  code.SetSuccess();
  respHdr.SetStatusCode(code);
  respHdr.SetAmsduSupport(reqHdr.IsAmsduSupported());
  respHdr.SetImmediateBlockAck();
  respHdr.SetTid(reqHdr.GetTid());
  respHdr.SetBufferSize(64);
  respHdr.SetTimeout(reqHdr.GetTimeout());
  m_mac->GetBEQueue()->GetBaManager()->UpdateOriginatorAgreement(
      respHdr, hdr.GetAddr1(), 0);

  Ptr<HtFrameExchangeManager> htFem = DynamicCast<HtFrameExchangeManager>(fem);
  Ptr<MpduAggregator> mpduAggregator = htFem->GetMpduAggregator();

  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(Create<WifiMpdu>(pkt, hdr));

  Ptr<WifiMpdu> peeked = m_mac->GetBEQueue()->PeekNextMpdu(SINGLE_LINK_OP_ID);
  WifiTxParameters txParams;
  txParams.m_txVector = m_mac->GetWifiRemoteStationManager()->GetDataTxVector(
      peeked->GetHeader(), m_phy->GetChannelWidth());
  Ptr<WifiMpdu> item = m_mac->GetBEQueue()->GetNextMpdu(
      SINGLE_LINK_OP_ID, peeked, txParams, Time::Min(), true);

  auto mpduList = mpduAggregator->GetNextAmpdu(item, txParams, Time::Min());

  NS_TEST_EXPECT_MSG_EQ(mpduList.empty(), true,
                        "a single packet should not result in an A-MPDU");

  m_mac->m_txMiddle->SetSequenceNumberFor(&item->GetHeader());
  item->UnassignSeqNo();

  Ptr<const Packet> pkt1 = Create<Packet>(1500);
  Ptr<const Packet> pkt2 = Create<Packet>(1500);
  WifiMacHeader hdr1;
  WifiMacHeader hdr2;

  hdr1.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr1.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr1.SetType(WIFI_MAC_QOSDATA);
  hdr1.SetQosTid(0);

  hdr2.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr2.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr2.SetType(WIFI_MAC_QOSDATA);
  hdr2.SetQosTid(0);

  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(Create<WifiMpdu>(pkt1, hdr1));
  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(Create<WifiMpdu>(pkt2, hdr2));

  item = m_mac->GetBEQueue()->GetNextMpdu(SINGLE_LINK_OP_ID, peeked, txParams,
                                          Time::Min(), true);
  mpduList = mpduAggregator->GetNextAmpdu(item, txParams, Time::Min());

  NS_TEST_EXPECT_MSG_EQ(mpduList.empty(), false, "MPDU aggregation failed");

  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(mpduList);
  htFem->DequeuePsdu(psdu);

  NS_TEST_EXPECT_MSG_EQ(psdu->GetSize(), 4606, "A-MPDU size is not correct");
  NS_TEST_EXPECT_MSG_EQ(mpduList.size(), 3, "A-MPDU should contain 3 MPDUs");
  NS_TEST_EXPECT_MSG_EQ(m_mac->GetBEQueue()->GetWifiMacQueue()->GetNPackets(),
                        0, "queue should be empty");

  for (uint32_t i = 0; i < psdu->GetNMpdus(); i++) {
    NS_TEST_EXPECT_MSG_EQ(psdu->GetHeader(i).GetSequenceNumber(), i,
                          "wrong sequence number");
  }

  pkt1 = Create<Packet>(1500);
  pkt2 = Create<Packet>(1500);
  hdr1.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr1.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr1.SetType(WIFI_MAC_QOSDATA);
  hdr1.SetQosTid(0);
  hdr1.SetSequenceNumber(3);
  hdr2.SetAddr1(Mac48Address("00:00:00:00:00:03"));
  hdr2.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr2.SetType(WIFI_MAC_QOSDATA);
  hdr2.SetQosTid(0);

  Ptr<const Packet> pkt3 = Create<Packet>(1500);
  WifiMacHeader hdr3;
  hdr3.SetSequenceNumber(0);
  hdr3.SetAddr1(Mac48Address("00:00:00:00:00:03"));
  hdr3.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr3.SetType(WIFI_MAC_QOSDATA);
  hdr3.SetQosTid(0);

  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(Create<WifiMpdu>(pkt1, hdr1));
  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(Create<WifiMpdu>(pkt2, hdr2));
  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(Create<WifiMpdu>(pkt3, hdr3));

  peeked = m_mac->GetBEQueue()->PeekNextMpdu(SINGLE_LINK_OP_ID);
  txParams.Clear();
  txParams.m_txVector = m_mac->GetWifiRemoteStationManager()->GetDataTxVector(
      peeked->GetHeader(), m_phy->GetChannelWidth());
  item = m_mac->GetBEQueue()->GetNextMpdu(SINGLE_LINK_OP_ID, peeked, txParams,
                                          Time::Min(), true);

  mpduList = mpduAggregator->GetNextAmpdu(item, txParams, Time::Min());

  NS_TEST_EXPECT_MSG_EQ(
      mpduList.empty(), true,
      "a single packet for this destination should not result in an A-MPDU");
  htFem->DequeueMpdu(item);

  peeked = m_mac->GetBEQueue()->PeekNextMpdu(SINGLE_LINK_OP_ID);
  txParams.Clear();
  txParams.m_txVector = m_mac->GetWifiRemoteStationManager()->GetDataTxVector(
      peeked->GetHeader(), m_phy->GetChannelWidth());
  item = m_mac->GetBEQueue()->GetNextMpdu(SINGLE_LINK_OP_ID, peeked, txParams,
                                          Time::Min(), true);

  mpduList = mpduAggregator->GetNextAmpdu(item, txParams, Time::Min());

  NS_TEST_EXPECT_MSG_EQ(
      mpduList.empty(), true,
      "no MPDU aggregation should be performed if there is no agreement");

  m_manager->SetMaxSsrc(0);
  m_mac->TraceConnectWithoutContext(
      "DroppedMpdu", MakeCallback(&AmpduAggregationTest::MpduDiscarded, this));
  htFem->m_dcf = m_mac->GetBEQueue();
  htFem->NormalAckTimeout(item, txParams.m_txVector);

  NS_TEST_EXPECT_MSG_EQ(m_discarded, true, "packet should be discarded");
  m_mac->GetBEQueue()->GetWifiMacQueue()->Flush();

  Simulator::Destroy();

  m_manager->Dispose();
  m_manager = nullptr;

  m_device->Dispose();
  m_device = nullptr;

  htConfiguration = nullptr;
}

class TwoLevelAggregationTest : public TestCase {
public:
  TwoLevelAggregationTest();

private:
  void DoRun() override;
  Ptr<WifiNetDevice> m_device;
  Ptr<StaWifiMac> m_mac;
  Ptr<YansWifiPhy> m_phy;
  Ptr<WifiRemoteStationManager> m_manager;
  ObjectFactory m_factory;
};

TwoLevelAggregationTest::TwoLevelAggregationTest()
    : TestCase("Check the correctness of two-level aggregation operations") {}

void TwoLevelAggregationTest::DoRun() {
  m_device = CreateObject<WifiNetDevice>();
  m_device->SetStandard(WIFI_STANDARD_80211n);
  Ptr<HtConfiguration> htConfiguration = CreateObject<HtConfiguration>();
  m_device->SetHtConfiguration(htConfiguration);

  m_phy = CreateObject<YansWifiPhy>();
  Ptr<InterferenceHelper> interferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phy->SetInterferenceHelper(interferenceHelper);
  m_phy->SetDevice(m_device);
  m_phy->ConfigureStandard(WIFI_STANDARD_80211n);
  m_device->SetPhy(m_phy);

  m_factory = ObjectFactory();
  m_factory.SetTypeId("ns3::ConstantRateWifiManager");
  m_factory.Set("DataMode", StringValue("HtMcs7"));
  m_manager = m_factory.Create<WifiRemoteStationManager>();
  m_manager->SetupPhy(m_phy);
  m_device->SetRemoteStationManager(m_manager);

  m_mac = CreateObjectWithAttributes<StaWifiMac>("QosSupported",
                                                 BooleanValue(true));
  m_mac->SetDevice(m_device);
  m_mac->SetWifiRemoteStationManager(m_manager);
  m_mac->SetAddress(Mac48Address("00:00:00:00:00:01"));
  m_mac->SetWifiPhys({m_phy});
  m_mac->ConfigureStandard(WIFI_STANDARD_80211n);
  Ptr<FrameExchangeManager> fem = m_mac->GetFrameExchangeManager();
  Ptr<WifiProtectionManager> protectionManager =
      CreateObject<WifiDefaultProtectionManager>();
  protectionManager->SetWifiMac(m_mac);
  fem->SetProtectionManager(protectionManager);
  Ptr<WifiAckManager> ackManager = CreateObject<WifiDefaultAckManager>();
  ackManager->SetWifiMac(m_mac);
  fem->SetAckManager(ackManager);
  m_device->SetMac(m_mac);
  m_mac->SetState(StaWifiMac::ASSOCIATED);
  m_mac->SetMacQueueScheduler(CreateObject<FcfsWifiQueueScheduler>());

  m_mac->SetAttribute("BE_MaxAmsduSize", UintegerValue(4095));
  m_mac->SetAttribute("BE_MaxAmpduSize", UintegerValue(65535));
  HtCapabilities htCapabilities;
  htCapabilities.SetMaxAmsduLength(7935);
  htCapabilities.SetMaxAmpduLength(65535);
  m_manager->AddStationHtCapabilities(Mac48Address("00:00:00:00:00:02"),
                                      htCapabilities);

  Ptr<const Packet> pkt = Create<Packet>(1500);
  WifiMacHeader hdr;
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);

  Ptr<HtFrameExchangeManager> htFem = DynamicCast<HtFrameExchangeManager>(fem);
  Ptr<MsduAggregator> msduAggregator = htFem->GetMsduAggregator();
  Ptr<MpduAggregator> mpduAggregator = htFem->GetMpduAggregator();

  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(
      Create<WifiMpdu>(Create<Packet>(1500), hdr));
  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(
      Create<WifiMpdu>(Create<Packet>(1500), hdr));
  m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(
      Create<WifiMpdu>(Create<Packet>(1500), hdr));

  Ptr<WifiMpdu> peeked = m_mac->GetBEQueue()->PeekNextMpdu(SINGLE_LINK_OP_ID);
  WifiTxParameters txParams;
  txParams.m_txVector = m_mac->GetWifiRemoteStationManager()->GetDataTxVector(
      peeked->GetHeader(), m_phy->GetChannelWidth());
  htFem->TryAddMpdu(peeked, txParams, Time::Min());
  Ptr<WifiMpdu> item =
      msduAggregator->GetNextAmsdu(peeked, txParams, Time::Min());

  bool result{item};
  NS_TEST_EXPECT_MSG_EQ(result, true, "aggregation failed");
  NS_TEST_EXPECT_MSG_EQ(item->GetPacketSize(), 3030, "wrong packet size");

  htFem->DequeueMpdu(item);

  NS_TEST_EXPECT_MSG_EQ(m_mac->GetBEQueue()->GetWifiMacQueue()->GetNPackets(),
                        1, "Unexpected number of MSDUs left in the EDCA queue");

  peeked = m_mac->GetBEQueue()->PeekNextMpdu(SINGLE_LINK_OP_ID);
  txParams.Clear();
  txParams.m_txVector = m_mac->GetWifiRemoteStationManager()->GetDataTxVector(
      peeked->GetHeader(), m_phy->GetChannelWidth());
  htFem->TryAddMpdu(peeked, txParams, Time::Min());
  item = msduAggregator->GetNextAmsdu(peeked, txParams, Time::Min());

  NS_TEST_EXPECT_MSG_EQ(item, nullptr, "A-MSDU aggregation did not fail");

  htFem->DequeueMpdu(peeked);

  NS_TEST_EXPECT_MSG_EQ(m_mac->GetBEQueue()->GetWifiMacQueue()->GetNPackets(),
                        0, "queue should be empty");

  uint8_t tid = 5;
  MgtAddBaRequestHeader reqHdr;
  reqHdr.SetImmediateBlockAck();
  reqHdr.SetTid(tid);
  reqHdr.SetBufferSize(64);
  reqHdr.SetTimeout(0);
  reqHdr.SetStartingSequence(0);
  m_mac->GetVIQueue()->GetBaManager()->CreateOriginatorAgreement(
      reqHdr, hdr.GetAddr1());

  MgtAddBaResponseHeader respHdr;
  StatusCode code;
  code.SetSuccess();
  respHdr.SetStatusCode(code);
  respHdr.SetAmsduSupport(reqHdr.IsAmsduSupported());
  respHdr.SetImmediateBlockAck();
  respHdr.SetTid(reqHdr.GetTid());
  respHdr.SetBufferSize(64);
  respHdr.SetTimeout(reqHdr.GetTimeout());
  m_mac->GetVIQueue()->GetBaManager()->UpdateOriginatorAgreement(
      respHdr, hdr.GetAddr1(), 0);

  m_mac->SetAttribute("VI_MaxAmsduSize", UintegerValue(3050));
  m_mac->SetAttribute("VI_MaxAmpduSize", UintegerValue(65535));
  m_mac->GetVIQueue()->SetAttribute("TxopLimits",
                                    AttributeContainerValue<TimeValue>(
                                        std::vector<Time>{MicroSeconds(3008)}));
  m_manager->SetAttribute("DataMode", StringValue("HtMcs2"));

  hdr.SetQosTid(tid);

  for (uint8_t i = 0; i < 10; i++) {
    m_mac->GetVIQueue()->GetWifiMacQueue()->Enqueue(
        Create<WifiMpdu>(Create<Packet>(1300), hdr));
  }

  peeked = m_mac->GetVIQueue()->PeekNextMpdu(SINGLE_LINK_OP_ID);
  txParams.Clear();
  txParams.m_txVector = m_mac->GetWifiRemoteStationManager()->GetDataTxVector(
      peeked->GetHeader(), m_phy->GetChannelWidth());
  Time txopLimit = m_mac->GetVIQueue()->GetTxopLimit();

  item = m_mac->GetVIQueue()->GetNextMpdu(SINGLE_LINK_OP_ID, peeked, txParams,
                                          txopLimit, true);

  NS_TEST_EXPECT_MSG_EQ(std::distance(item->begin(), item->end()), 2,
                        "There must be 2 MSDUs in the A-MSDU");

  auto mpduList = mpduAggregator->GetNextAmpdu(item, txParams, txopLimit);

  NS_TEST_EXPECT_MSG_EQ(mpduList.empty(), false, "aggregation failed");
  NS_TEST_EXPECT_MSG_EQ(mpduList.size(), 3,
                        "Unexpected number of MPDUs in the A-MPDU");
  NS_TEST_EXPECT_MSG_EQ(mpduList.at(0)->GetSize(), 2660,
                        "Unexpected size of the first MPDU");
  NS_TEST_EXPECT_MSG_EQ(mpduList.at(1)->GetSize(), 2660,
                        "Unexpected size of the second MPDU");
  NS_TEST_EXPECT_MSG_EQ(mpduList.at(2)->GetSize(), 1330,
                        "Unexpected size of the first MPDU");

  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(mpduList);
  htFem->DequeuePsdu(psdu);

  NS_TEST_EXPECT_MSG_EQ(m_mac->GetVIQueue()->GetWifiMacQueue()->GetNPackets(),
                        5, "Unexpected number of MSDUs left in the EDCA queue");

  NS_TEST_EXPECT_MSG_EQ(psdu->GetSize(), 6662, "Unexpected size of the A-MPDU");

  Simulator::Destroy();

  m_device->Dispose();
  m_device = nullptr;
  htConfiguration = nullptr;
}

class HeAggregationTest : public TestCase {
public:
  HeAggregationTest();

private:
  void DoRun() override;
  void DoRunSubTest(uint16_t bufferSize);
  Ptr<WifiNetDevice> m_device;
  Ptr<StaWifiMac> m_mac;
  Ptr<YansWifiPhy> m_phy;
  Ptr<WifiRemoteStationManager> m_manager;
  ObjectFactory m_factory;
};

HeAggregationTest::HeAggregationTest()
    : TestCase("Check the correctness of 802.11ax aggregation operations") {}

void HeAggregationTest::DoRunSubTest(uint16_t bufferSize) {
  m_device = CreateObject<WifiNetDevice>();
  Ptr<HtConfiguration> htConfiguration = CreateObject<HtConfiguration>();
  m_device->SetHtConfiguration(htConfiguration);
  Ptr<VhtConfiguration> vhtConfiguration = CreateObject<VhtConfiguration>();
  m_device->SetVhtConfiguration(vhtConfiguration);
  Ptr<HeConfiguration> heConfiguration = CreateObject<HeConfiguration>();
  m_device->SetHeConfiguration(heConfiguration);

  m_phy = CreateObject<YansWifiPhy>();
  Ptr<InterferenceHelper> interferenceHelper =
      CreateObject<InterferenceHelper>();
  m_phy->SetInterferenceHelper(interferenceHelper);
  m_phy->SetDevice(m_device);
  m_phy->ConfigureStandard(WIFI_STANDARD_80211ax);
  m_device->SetPhy(m_phy);

  m_factory = ObjectFactory();
  m_factory.SetTypeId("ns3::ConstantRateWifiManager");
  m_factory.Set("DataMode", StringValue("HeMcs11"));
  m_manager = m_factory.Create<WifiRemoteStationManager>();
  m_manager->SetupPhy(m_phy);
  m_device->SetRemoteStationManager(m_manager);

  m_mac = CreateObjectWithAttributes<StaWifiMac>("QosSupported",
                                                 BooleanValue(true));
  m_mac->SetDevice(m_device);
  m_mac->SetWifiRemoteStationManager(m_manager);
  m_mac->SetAddress(Mac48Address("00:00:00:00:00:01"));
  m_mac->SetWifiPhys({m_phy});
  m_mac->ConfigureStandard(WIFI_STANDARD_80211ax);
  Ptr<FrameExchangeManager> fem = m_mac->GetFrameExchangeManager();
  Ptr<WifiProtectionManager> protectionManager =
      CreateObject<WifiDefaultProtectionManager>();
  protectionManager->SetWifiMac(m_mac);
  fem->SetProtectionManager(protectionManager);
  Ptr<WifiAckManager> ackManager = CreateObject<WifiDefaultAckManager>();
  ackManager->SetWifiMac(m_mac);
  fem->SetAckManager(ackManager);
  m_device->SetMac(m_mac);
  m_mac->SetState(StaWifiMac::ASSOCIATED);
  m_mac->SetMacQueueScheduler(CreateObject<FcfsWifiQueueScheduler>());

  HeCapabilities heCapabilities;
  m_manager->AddStationHeCapabilities(Mac48Address("00:00:00:00:00:02"),
                                      heCapabilities);

  WifiMacHeader hdr;
  hdr.SetAddr1(Mac48Address("00:00:00:00:00:02"));
  hdr.SetAddr2(Mac48Address("00:00:00:00:00:01"));
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  uint16_t sequence = m_mac->m_txMiddle->PeekNextSequenceNumberFor(&hdr);
  hdr.SetSequenceNumber(sequence);
  hdr.SetFragmentNumber(0);
  hdr.SetNoMoreFragments();
  hdr.SetNoRetry();

  MgtAddBaRequestHeader reqHdr;
  reqHdr.SetImmediateBlockAck();
  reqHdr.SetTid(0);
  reqHdr.SetBufferSize(bufferSize);
  reqHdr.SetTimeout(0);
  reqHdr.SetStartingSequence(0);
  m_mac->GetBEQueue()->GetBaManager()->CreateOriginatorAgreement(
      reqHdr, hdr.GetAddr1());

  MgtAddBaResponseHeader respHdr;
  StatusCode code;
  code.SetSuccess();
  respHdr.SetStatusCode(code);
  respHdr.SetAmsduSupport(reqHdr.IsAmsduSupported());
  respHdr.SetImmediateBlockAck();
  respHdr.SetTid(reqHdr.GetTid());
  respHdr.SetBufferSize(bufferSize);
  respHdr.SetTimeout(reqHdr.GetTimeout());
  m_mac->GetBEQueue()->GetBaManager()->UpdateOriginatorAgreement(
      respHdr, hdr.GetAddr1(), 0);

  Ptr<HtFrameExchangeManager> htFem = DynamicCast<HtFrameExchangeManager>(fem);
  Ptr<MpduAggregator> mpduAggregator = htFem->GetMpduAggregator();

  for (uint16_t i = 0; i < 300; i++) {
    Ptr<const Packet> pkt = Create<Packet>(100);
    WifiMacHeader hdr;

    hdr.SetAddr1(Mac48Address("00:00:00:00:00:02"));
    hdr.SetAddr2(Mac48Address("00:00:00:00:00:01"));
    hdr.SetType(WIFI_MAC_QOSDATA);
    hdr.SetQosTid(0);

    m_mac->GetBEQueue()->GetWifiMacQueue()->Enqueue(Create<WifiMpdu>(pkt, hdr));
  }

  Ptr<WifiMpdu> peeked = m_mac->GetBEQueue()->PeekNextMpdu(SINGLE_LINK_OP_ID);
  WifiTxParameters txParams;
  txParams.m_txVector = m_mac->GetWifiRemoteStationManager()->GetDataTxVector(
      peeked->GetHeader(), m_phy->GetChannelWidth());
  Ptr<WifiMpdu> item = m_mac->GetBEQueue()->GetNextMpdu(
      SINGLE_LINK_OP_ID, peeked, txParams, Time::Min(), true);

  auto mpduList = mpduAggregator->GetNextAmpdu(item, txParams, Time::Min());
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(mpduList);
  htFem->DequeuePsdu(psdu);

  NS_TEST_EXPECT_MSG_EQ(mpduList.empty(), false, "MPDU aggregation failed");
  NS_TEST_EXPECT_MSG_EQ(mpduList.size(), bufferSize,
                        "A-MPDU should contain " << bufferSize << " MPDUs");
  uint16_t expectedRemainingPacketsInQueue = 300 - bufferSize;
  NS_TEST_EXPECT_MSG_EQ(m_mac->GetBEQueue()->GetWifiMacQueue()->GetNPackets(),
                        expectedRemainingPacketsInQueue,
                        "queue should contain 300 - "
                            << bufferSize << " = "
                            << expectedRemainingPacketsInQueue << " packets");

  Simulator::Destroy();

  m_manager->Dispose();
  m_manager = nullptr;

  m_device->Dispose();
  m_device = nullptr;

  htConfiguration = nullptr;
  vhtConfiguration = nullptr;
  heConfiguration = nullptr;
}

void HeAggregationTest::DoRun() {
  DoRunSubTest(64);
  DoRunSubTest(256);
}

class PreservePacketsInAmpdus : public TestCase {
public:
  PreservePacketsInAmpdus();
  ~PreservePacketsInAmpdus() override;

  void DoRun() override;

private:
  std::list<Ptr<const Packet>> m_packetList;
  std::vector<std::size_t> m_nMpdus;
  std::vector<std::size_t> m_nMsdus;

  void NotifyMacTransmit(Ptr<const Packet> packet);
  void NotifyPsduForwardedDown(WifiConstPsduMap psduMap, WifiTxVector txVector,
                               double txPowerW);
  void NotifyMacForwardUp(Ptr<const Packet> p);
};

PreservePacketsInAmpdus::PreservePacketsInAmpdus()
    : TestCase("Test case to check that the Wifi Mac forwards up the same "
               "packets received at "
               "sender side.") {}

PreservePacketsInAmpdus::~PreservePacketsInAmpdus() {}

void PreservePacketsInAmpdus::NotifyMacTransmit(Ptr<const Packet> packet) {
  m_packetList.push_back(packet);
}

void PreservePacketsInAmpdus::NotifyPsduForwardedDown(WifiConstPsduMap psduMap,
                                                      WifiTxVector txVector,
                                                      double txPowerW) {
  NS_TEST_EXPECT_MSG_EQ(
      (psduMap.size() == 1 && psduMap.begin()->first == SU_STA_ID), true,
      "No DL MU PPDU expected");

  if (!psduMap[SU_STA_ID]->GetHeader(0).IsQosData()) {
    return;
  }

  m_nMpdus.push_back(psduMap[SU_STA_ID]->GetNMpdus());

  for (auto &mpdu : *PeekPointer(psduMap[SU_STA_ID])) {
    std::size_t dist = std::distance(mpdu->begin(), mpdu->end());
    m_nMsdus.push_back(dist > 0 ? dist : 1);
  }
}

void PreservePacketsInAmpdus::NotifyMacForwardUp(Ptr<const Packet> p) {
  auto it = std::find(m_packetList.begin(), m_packetList.end(), p);
  NS_TEST_EXPECT_MSG_EQ((it != m_packetList.end()), true,
                        "Packet being forwarded up not found");
  m_packetList.erase(it);
}

void PreservePacketsInAmpdus::DoRun() {
  NodeContainer wifiStaNode;
  wifiStaNode.Create(1);

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);
  wifi.SetRemoteStationManager("ns3::IdealWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType("ns3::StaWifiMac", "BE_MaxAmsduSize", UintegerValue(4500),
              "BE_MaxAmpduSize", UintegerValue(7500), "Ssid", SsidValue(ssid),
              "BE_BlockAckThreshold", UintegerValue(2), "ActiveProbing",
              BooleanValue(false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install(phy, mac, wifiStaNode);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "BeaconGeneration",
              BooleanValue(true));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install(phy, mac, wifiApNode);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNode);

  Ptr<WifiNetDevice> ap_device = DynamicCast<WifiNetDevice>(apDevices.Get(0));
  Ptr<WifiNetDevice> sta_device = DynamicCast<WifiNetDevice>(staDevices.Get(0));

  PacketSocketAddress socket;
  socket.SetSingleDevice(sta_device->GetIfIndex());
  socket.SetPhysicalAddress(ap_device->GetAddress());
  socket.SetProtocol(1);

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiStaNode);
  packetSocket.Install(wifiApNode);

  Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
  client->SetAttribute("PacketSize", UintegerValue(1000));
  client->SetAttribute("MaxPackets", UintegerValue(8));
  client->SetAttribute("Interval", TimeValue(Seconds(1)));
  client->SetRemote(socket);
  wifiStaNode.Get(0)->AddApplication(client);
  client->SetStartTime(Seconds(1));
  client->SetStopTime(Seconds(3.0));
  Simulator::Schedule(Seconds(1.5), &PacketSocketClient::SetAttribute, client,
                      "Interval", TimeValue(MicroSeconds(0)));

  Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
  server->SetLocal(socket);
  wifiApNode.Get(0)->AddApplication(server);
  server->SetStartTime(Seconds(0.0));
  server->SetStopTime(Seconds(4.0));

  sta_device->GetMac()->TraceConnectWithoutContext(
      "MacTx", MakeCallback(&PreservePacketsInAmpdus::NotifyMacTransmit, this));
  sta_device->GetPhy()->TraceConnectWithoutContext(
      "PhyTxPsduBegin",
      MakeCallback(&PreservePacketsInAmpdus::NotifyPsduForwardedDown, this));
  ap_device->GetMac()->TraceConnectWithoutContext(
      "MacRx",
      MakeCallback(&PreservePacketsInAmpdus::NotifyMacForwardUp, this));

  Simulator::Stop(Seconds(5));
  Simulator::Run();

  Simulator::Destroy();

  NS_TEST_EXPECT_MSG_EQ(m_nMpdus.size(), 2,
                        "Unexpected number of transmitted packets");
  NS_TEST_EXPECT_MSG_EQ(m_nMsdus.size(), 3,
                        "Unexpected number of transmitted MPDUs");
  NS_TEST_EXPECT_MSG_EQ(m_nMpdus[0], 1,
                        "Unexpected number of MPDUs in the first A-MPDU");
  NS_TEST_EXPECT_MSG_EQ(m_nMsdus[0], 1,
                        "Unexpected number of MSDUs in the first MPDU");
  NS_TEST_EXPECT_MSG_EQ(m_nMpdus[1], 2,
                        "Unexpected number of MPDUs in the second A-MPDU");
  NS_TEST_EXPECT_MSG_EQ(m_nMsdus[1], 4,
                        "Unexpected number of MSDUs in the second MPDU");
  NS_TEST_EXPECT_MSG_EQ(m_nMsdus[2], 3,
                        "Unexpected number of MSDUs in the third MPDU");
  NS_TEST_EXPECT_MSG_EQ(m_packetList.empty(), true,
                        "Some packets have not been forwarded up");
}

class WifiAggregationTestSuite : public TestSuite {
public:
  WifiAggregationTestSuite();
};

WifiAggregationTestSuite::WifiAggregationTestSuite()
    : TestSuite("wifi-aggregation", UNIT) {
  AddTestCase(new AmpduAggregationTest, TestCase::QUICK);
  AddTestCase(new TwoLevelAggregationTest, TestCase::QUICK);
  AddTestCase(new HeAggregationTest, TestCase::QUICK);
  AddTestCase(new PreservePacketsInAmpdus, TestCase::QUICK);
}

static WifiAggregationTestSuite g_wifiAggregationTestSuite;
