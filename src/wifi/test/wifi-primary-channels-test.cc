
#include "ns3/ap-wifi-mac.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/ctrl-headers.h"
#include "ns3/enum.h"
#include "ns3/he-configuration.h"
#include "ns3/he-phy.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/test.h"
#include "ns3/tuple.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"

#include <algorithm>
#include <bitset>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiPrimaryChannelsTest");

class WifiPrimaryChannelsTest : public TestCase {
public:
  WifiPrimaryChannelsTest(uint16_t channelWidth, bool useDistinctBssColors);
  ~WifiPrimaryChannelsTest() override;

  void Transmit(std::string context, WifiConstPsduMap psduMap,
                WifiTxVector txVector, double txPowerW);
  void SendDlSuPpdu(uint8_t bss, uint16_t txChannelWidth);
  void SendDlMuPpdu(uint8_t bss, uint16_t txChannelWidth, HeRu::RuType ruType,
                    std::size_t nRus);
  void SendHeTbPpdu(uint8_t bss, uint16_t txChannelWidth, HeRu::RuType ruType,
                    std::size_t nRus);
  void DoSendHeTbPpdu(uint8_t bss, uint16_t txChannelWidth, HeRu::RuType ruType,
                      std::size_t nRus);
  void ReceiveDl(uint8_t bss, uint8_t station, Ptr<const WifiPsdu> psdu,
                 RxSignalInfo rxSignalInfo, WifiTxVector txVector,
                 std::vector<bool> perMpduStatus);
  void ReceiveUl(uint8_t bss, Ptr<const WifiPsdu> psdu,
                 RxSignalInfo rxSignalInfo, WifiTxVector txVector,
                 std::vector<bool> perMpduStatus);
  void CheckAssociation();
  void CheckReceivedSuPpdus(std::set<uint8_t> txBss, uint16_t txChannelWidth);
  void CheckReceivedMuPpdus(std::set<uint8_t> txBss, uint16_t txChannelWidth,
                            HeRu::RuType ruType, std::size_t nRus, bool isDlMu);
  void CheckReceivedTriggerFrames(std::set<uint8_t> txBss,
                                  uint16_t txChannelWidth);

private:
  void DoSetup() override;
  void DoRun() override;

  uint16_t m_channelWidth;
  bool m_useDistinctBssColors;
  uint8_t m_nBss;
  uint8_t m_nStationsPerBss;
  std::vector<NetDeviceContainer> m_staDevices;
  NetDeviceContainer m_apDevices;
  std::vector<std::bitset<74>> m_received;
  std::vector<std::bitset<74>> m_processed;
  Time m_time;
  Ptr<WifiPsdu> m_trigger;
  WifiTxVector m_triggerTxVector;
  Time m_triggerTxDuration;
};

WifiPrimaryChannelsTest::WifiPrimaryChannelsTest(uint16_t channelWidth,
                                                 bool useDistinctBssColors)
    : TestCase(
          "Check correct transmissions for various primary channel settings"),
      m_channelWidth(channelWidth),
      m_useDistinctBssColors(useDistinctBssColors) {}

WifiPrimaryChannelsTest::~WifiPrimaryChannelsTest() {}

void WifiPrimaryChannelsTest::Transmit(std::string context,
                                       WifiConstPsduMap psduMap,
                                       WifiTxVector txVector, double txPowerW) {
  for (const auto &psduPair : psduMap) {
    std::stringstream ss;

    if (psduPair.first != SU_STA_ID) {
      ss << " STA-ID " << psduPair.first;
    }
    ss << " " << psduPair.second->GetHeader(0).GetTypeString() << " seq "
       << psduPair.second->GetHeader(0).GetSequenceNumber() << " from "
       << psduPair.second->GetAddr2() << " to " << psduPair.second->GetAddr1();
    NS_LOG_INFO(ss.str());
  }
  NS_LOG_INFO(" TXVECTOR " << txVector);
}

void WifiPrimaryChannelsTest::ReceiveDl(uint8_t bss, uint8_t station,
                                        Ptr<const WifiPsdu> psdu,
                                        RxSignalInfo rxSignalInfo,
                                        WifiTxVector txVector,
                                        std::vector<bool> perMpduStatus) {
  if (psdu->GetNMpdus() == 1) {
    const WifiMacHeader &hdr = psdu->GetHeader(0);

    if (hdr.IsQosData() || hdr.IsTrigger()) {
      NS_LOG_INFO("RECEIVED BY BSS=" << +bss << " STA=" << +station << "  "
                                     << *psdu);
      NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(station), false,
                            "Station [" << +bss << "][" << +station
                                        << "] received a frame twice");
      m_received[bss].set(station);
      auto dev = DynamicCast<WifiNetDevice>(m_staDevices[bss].Get(station));
      if ((hdr.IsQosData() && hdr.GetAddr1() == dev->GetMac()->GetAddress()) ||
          (hdr.IsTrigger() && hdr.GetAddr1() == Mac48Address::GetBroadcast())) {
        NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(station), false,
                              "Station [" << +bss << "][" << +station
                                          << "] processed a frame twice");
        m_processed[bss].set(station);
      }
    }
  }
}

void WifiPrimaryChannelsTest::ReceiveUl(uint8_t bss, Ptr<const WifiPsdu> psdu,
                                        RxSignalInfo rxSignalInfo,
                                        WifiTxVector txVector,
                                        std::vector<bool> perMpduStatus) {
  if (psdu->GetNMpdus() == 1 && psdu->GetHeader(0).IsQosData() &&
      txVector.IsUlMu()) {
    auto dev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));

    uint16_t staId = txVector.GetHeMuUserInfoMap().begin()->first;
    uint8_t station = staId - 1;
    NS_LOG_INFO("RECEIVED FROM BSSID=" << psdu->GetHeader(0).GetAddr3()
                                       << " STA=" << +station << "  " << *psdu);
    NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(station), false,
                          "AP of BSS " << +bss
                                       << " received a frame from station "
                                       << +station << " twice");
    m_received[bss].set(station);
    if (psdu->GetHeader(0).GetAddr1() == dev->GetMac()->GetAddress()) {
      NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(station), false,
                            "AP of BSS " << +bss
                                         << " received a frame from station "
                                         << +station << " twice");
      m_processed[bss].set(station);
    }
  }
}

void WifiPrimaryChannelsTest::DoSetup() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(40);
  int64_t streamNumber = 100;
  uint8_t channelNum;

  switch (m_channelWidth) {
  case 20:
    m_nStationsPerBss = 9;
    channelNum = 36;
    break;
  case 40:
    m_nStationsPerBss = 18;
    channelNum = 38;
    break;
  case 80:
    m_nStationsPerBss = 37;
    channelNum = 42;
    break;
  case 160:
    m_nStationsPerBss = 74;
    channelNum = 50;
    break;
  default:
    NS_ABORT_MSG("Channel width (" << m_channelWidth << ") not allowed");
  }

  m_nBss = m_channelWidth / 20;

  NodeContainer wifiApNodes;
  wifiApNodes.Create(m_nBss);

  std::vector<NodeContainer> wifiStaNodes(m_nBss);
  for (auto &container : wifiStaNodes) {
    container.Create(m_nStationsPerBss);
  }

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  SpectrumWifiPhyHelper phy;
  phy.SetChannel(spectrumChannel);

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ax);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager");

  WifiMacHelper mac;
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(Ssid("non-existent-ssid")),
              "MaxMissedBeacons", UintegerValue(20), "WaitBeaconTimeout",
              TimeValue(MicroSeconds(102400)));

  TupleValue<UintegerValue, UintegerValue, EnumValue, UintegerValue>
      channelValue;

  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    channelValue.Set(WifiPhy::ChannelTuple{channelNum, m_channelWidth,
                                           WIFI_PHY_BAND_5GHZ, bss});
    phy.Set("ChannelSettings", channelValue);

    m_staDevices.push_back(wifi.Install(phy, mac, wifiStaNodes[bss]));
  }

  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    channelValue.Set(WifiPhy::ChannelTuple{channelNum, m_channelWidth,
                                           WIFI_PHY_BAND_5GHZ, bss});
    phy.Set("ChannelSettings", channelValue);

    mac.SetType("ns3::ApWifiMac", "Ssid",
                SsidValue(Ssid("wifi-ssid-" + std::to_string(bss))),
                "BeaconInterval", TimeValue(MicroSeconds(102400)),
                "EnableBeaconJitter", BooleanValue(false));

    m_apDevices.Add(wifi.Install(phy, mac, wifiApNodes.Get(bss)));
  }

  streamNumber = wifi.AssignStreams(m_apDevices, streamNumber);
  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    streamNumber = wifi.AssignStreams(m_staDevices[bss], streamNumber);
  }

  if (m_useDistinctBssColors) {
    for (uint8_t bss = 0; bss < m_nBss; bss++) {
      auto dev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));
      dev->GetHeConfiguration()->SetBssColor(bss + 1);
    }
  }

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNodes);
  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    mobility.Install(wifiStaNodes[bss]);
  }

  m_received.resize(m_nBss);
  m_processed.resize(m_nBss);

  auto apDev = DynamicCast<WifiNetDevice>(m_apDevices.Get(0));

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_CTL_TRIGGER);
  hdr.SetAddr1(Mac48Address::GetBroadcast());
  hdr.SetSequenceNumber(1);

  Ptr<Packet> pkt = Create<Packet>();
  CtrlTriggerHeader trigger;
  trigger.SetType(TriggerFrameType::BASIC_TRIGGER);
  pkt->AddHeader(trigger);

  m_triggerTxVector =
      WifiTxVector(OfdmPhy::GetOfdmRate6Mbps(), 0, WIFI_PREAMBLE_LONG, 800, 1,
                   1, 0, 20, false, false, false);
  m_trigger = Create<WifiPsdu>(pkt, hdr);

  m_triggerTxDuration =
      WifiPhy::CalculateTxDuration(m_trigger->GetSize(), m_triggerTxVector,
                                   apDev->GetMac()->GetWifiPhy()->GetPhyBand());
}

void WifiPrimaryChannelsTest::DoRun() {
  Ptr<WifiNetDevice> dev;

  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    dev = DynamicCast<WifiNetDevice>(m_staDevices[bss].Get(0));
    dev->GetMac()->SetSsid(Ssid("wifi-ssid-" + std::to_string(bss)));

    for (uint16_t i = 1; i < m_nStationsPerBss; i++) {
      dev = DynamicCast<WifiNetDevice>(m_staDevices[bss].Get(i));
      Simulator::Schedule(i * MicroSeconds(102400), &WifiMac::SetSsid,
                          dev->GetMac(),
                          Ssid("wifi-ssid-" + std::to_string(bss)));
    }
  }

  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    dev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));
    auto mac = DynamicCast<ApWifiMac>(dev->GetMac());

    Simulator::Schedule((m_nStationsPerBss - 1) * MicroSeconds(102400),
                        &ApWifiMac::SetBeaconInterval, mac,
                        MicroSeconds(1024 * 65535));
  }

  m_time = (m_nStationsPerBss + 1) * MicroSeconds(102400);

  Simulator::Schedule(m_time, &WifiPrimaryChannelsTest::CheckAssociation, this);

  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    for (uint8_t i = 0; i < m_nStationsPerBss; i++) {
      auto dev = DynamicCast<WifiNetDevice>(m_staDevices[bss].Get(i));
      Simulator::Schedule(
          m_time, &WifiPhy::SetReceiveOkCallback, dev->GetPhy(),
          MakeCallback(&WifiPrimaryChannelsTest::ReceiveDl, this).Bind(bss, i));
    }
    auto dev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));
    Simulator::Schedule(
        m_time, &WifiPhy::SetReceiveOkCallback, dev->GetPhy(),
        MakeCallback(&WifiPrimaryChannelsTest::ReceiveUl, this).Bind(bss));
  }

  Time roundDuration = MilliSeconds(5);

  for (uint16_t txChannelWidth = 20, nRounds = 2,
                nApsPerRound = m_channelWidth / 20 / 2;
       txChannelWidth <= m_channelWidth;
       txChannelWidth *= 2, nRounds *= 2, nApsPerRound /= 2) {
    nRounds = std::min<uint16_t>(nRounds, m_nBss);
    nApsPerRound = std::max<uint16_t>(nApsPerRound, 1);

    for (uint16_t round = 0; round < nRounds; round++) {
      std::set<uint8_t> txBss;

      for (uint16_t i = 0; i < nApsPerRound; i++) {
        uint16_t ap = round + i * nRounds;
        txBss.insert(ap);
        Simulator::Schedule(m_time, &WifiPrimaryChannelsTest::SendDlSuPpdu,
                            this, ap, txChannelWidth);
      }
      Simulator::Schedule(m_time + roundDuration,
                          &WifiPrimaryChannelsTest::CheckReceivedSuPpdus, this,
                          txBss, txChannelWidth);
      m_time += roundDuration;
    }
  }

  for (uint16_t txChannelWidth = 20, nRounds = 2,
                nApsPerRound = m_channelWidth / 20 / 2;
       txChannelWidth <= m_channelWidth;
       txChannelWidth *= 2, nRounds *= 2, nApsPerRound /= 2) {
    nRounds = std::min<uint16_t>(nRounds, m_nBss);
    nApsPerRound = std::max<uint16_t>(nApsPerRound, 1);

    for (uint16_t round = 0; round < nRounds; round++) {
      for (unsigned int type = 0; type < 7; type++) {
        auto ruType = static_cast<HeRu::RuType>(type);
        std::size_t nRus = HeRu::GetNRus(txChannelWidth, ruType);
        std::set<uint8_t> txBss;
        if (nRus > 0) {
          for (uint16_t i = 0; i < nApsPerRound; i++) {
            uint16_t ap = round + i * nRounds;
            txBss.insert(ap);
            Simulator::Schedule(m_time, &WifiPrimaryChannelsTest::SendDlMuPpdu,
                                this, ap, txChannelWidth, ruType, nRus);
          }
          Simulator::Schedule(m_time + roundDuration,
                              &WifiPrimaryChannelsTest::CheckReceivedMuPpdus,
                              this, txBss, txChannelWidth, ruType, nRus, true);
          m_time += roundDuration;
        }
      }
    }
  }

  for (uint16_t txChannelWidth = 20, nRounds = 2,
                nApsPerRound = m_channelWidth / 20 / 2;
       txChannelWidth <= m_channelWidth;
       txChannelWidth *= 2, nRounds *= 2, nApsPerRound /= 2) {
    nRounds = std::min<uint16_t>(nRounds, m_nBss);
    nApsPerRound = std::max<uint16_t>(nApsPerRound, 1);

    for (uint16_t round = 0; round < nRounds; round++) {
      for (unsigned int type = 0; type < 7; type++) {
        auto ruType = static_cast<HeRu::RuType>(type);
        std::size_t nRus = HeRu::GetNRus(txChannelWidth, ruType);
        std::set<uint8_t> txBss;
        if (nRus > 0) {
          for (uint16_t i = 0; i < nApsPerRound; i++) {
            uint16_t ap = round + i * nRounds;
            txBss.insert(ap);
            Simulator::Schedule(m_time, &WifiPrimaryChannelsTest::SendHeTbPpdu,
                                this, ap, txChannelWidth, ruType, nRus);
          }
          Simulator::Schedule(
              m_time + m_triggerTxDuration + MicroSeconds(10),
              &WifiPrimaryChannelsTest::CheckReceivedTriggerFrames, this, txBss,
              txChannelWidth);
          Simulator::Schedule(m_time + roundDuration,
                              &WifiPrimaryChannelsTest::CheckReceivedMuPpdus,
                              this, txBss, txChannelWidth, ruType, nRus, false);
          m_time += roundDuration;
        }
      }
    }
  }

  Config::Connect(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
      MakeCallback(&WifiPrimaryChannelsTest::Transmit, this));

  Simulator::Stop(m_time);
  Simulator::Run();

  Simulator::Destroy();
}

void WifiPrimaryChannelsTest::SendDlSuPpdu(uint8_t bss,
                                           uint16_t txChannelWidth) {
  NS_LOG_INFO("*** BSS " << +bss << " transmits on primary " << txChannelWidth
                         << " MHz channel");

  auto apDev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));
  auto staDev = DynamicCast<WifiNetDevice>(m_staDevices[bss].Get(0));

  uint8_t bssColor = apDev->GetHeConfiguration()->GetBssColor();
  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs8(), 0, WIFI_PREAMBLE_HE_SU, 800, 1, 1, 0,
                   txChannelWidth, false, false, false, bssColor);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(staDev->GetMac()->GetAddress());
  hdr.SetAddr2(apDev->GetMac()->GetAddress());
  hdr.SetAddr3(apDev->GetMac()->GetBssid(0));
  hdr.SetSequenceNumber(1);
  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(Create<Packet>(1000), hdr);
  apDev->GetPhy()->Send(WifiConstPsduMap({std::make_pair(SU_STA_ID, psdu)}),
                        txVector);
}

void WifiPrimaryChannelsTest::SendDlMuPpdu(uint8_t bss, uint16_t txChannelWidth,
                                           HeRu::RuType ruType,
                                           std::size_t nRus) {
  NS_LOG_INFO("*** BSS " << +bss << " transmits on primary " << txChannelWidth
                         << " MHz channel a DL MU PPDU "
                         << "addressed to " << nRus
                         << " stations (RU type: " << ruType << ")");

  auto apDev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));
  uint8_t bssColor = apDev->GetHeConfiguration()->GetBssColor();

  WifiTxVector txVector =
      WifiTxVector(HePhy::GetHeMcs8(), 0, WIFI_PREAMBLE_HE_MU, 800, 1, 1, 0,
                   txChannelWidth, false, false, false, bssColor);
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr2(apDev->GetMac()->GetAddress());
  hdr.SetAddr3(apDev->GetMac()->GetBssid(0));
  hdr.SetSequenceNumber(1);

  WifiConstPsduMap psduMap;

  for (std::size_t i = 1; i <= nRus; i++) {
    bool primary80 = !(txChannelWidth == 160 && i > nRus / 2);
    std::size_t index = (primary80 ? i : i - nRus / 2);

    auto staDev = DynamicCast<WifiNetDevice>(m_staDevices[bss].Get(i - 1));
    uint16_t staId =
        DynamicCast<StaWifiMac>(staDev->GetMac())->GetAssociationId();
    txVector.SetHeMuUserInfo(staId, {{ruType, index, primary80}, 8, 1});
    hdr.SetAddr1(staDev->GetMac()->GetAddress());
    psduMap[staId] = Create<const WifiPsdu>(Create<Packet>(1000), hdr);
  }
  txVector.SetSigBMode(VhtPhy::GetVhtMcs5());
  RuAllocation ruAllocations;
  auto numRuAllocs = txChannelWidth / 20;
  ruAllocations.resize(numRuAllocs);
  auto IsOddNum = (nRus / numRuAllocs) % 2 == 1;
  auto ruAlloc = HeRu::GetEqualizedRuAllocation(ruType, IsOddNum);
  std::fill_n(ruAllocations.begin(), numRuAllocs, ruAlloc);
  txVector.SetRuAllocation(ruAllocations, 0);

  apDev->GetPhy()->Send(psduMap, txVector);
}

void WifiPrimaryChannelsTest::SendHeTbPpdu(uint8_t bss, uint16_t txChannelWidth,
                                           HeRu::RuType ruType,
                                           std::size_t nRus) {
  NS_LOG_INFO("*** BSS " << +bss << " transmits a Basic Trigger Frame");

  auto apDev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));

  m_trigger->GetHeader(0).SetAddr2(apDev->GetMac()->GetAddress());

  apDev->GetPhy()->Send(m_trigger, m_triggerTxVector);

  Simulator::Schedule(m_triggerTxDuration + apDev->GetPhy()->GetSifs(),
                      &WifiPrimaryChannelsTest::DoSendHeTbPpdu, this, bss,
                      txChannelWidth, ruType, nRus);
}

void WifiPrimaryChannelsTest::DoSendHeTbPpdu(uint8_t bss,
                                             uint16_t txChannelWidth,
                                             HeRu::RuType ruType,
                                             std::size_t nRus) {
  auto apDev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));
  uint8_t bssColor = apDev->GetHeConfiguration()->GetBssColor();

  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetQosTid(0);
  hdr.SetAddr1(apDev->GetMac()->GetAddress());
  hdr.SetAddr3(apDev->GetMac()->GetBssid(0));
  hdr.SetSequenceNumber(1);

  Time duration = Seconds(0);
  uint16_t length = 0;
  WifiTxVector trigVector(HePhy::GetHeMcs8(), 0, WIFI_PREAMBLE_HE_TB, 3200, 1,
                          1, 0, txChannelWidth, false, false, false, bssColor);

  for (std::size_t i = 1; i <= nRus; i++) {
    NS_LOG_INFO("*** BSS " << +bss << " STA " << i - 1
                           << " transmits on primary " << txChannelWidth
                           << " MHz channel an HE TB PPDU (RU type: " << ruType
                           << ")");

    bool primary80 = !(txChannelWidth == 160 && i > nRus / 2);
    std::size_t index = (primary80 ? i : i - nRus / 2);

    auto staDev = DynamicCast<WifiNetDevice>(m_staDevices[bss].Get(i - 1));
    uint16_t staId =
        DynamicCast<StaWifiMac>(staDev->GetMac())->GetAssociationId();

    WifiTxVector txVector(HePhy::GetHeMcs8(), 0, WIFI_PREAMBLE_HE_TB, 3200, 1,
                          1, 0, txChannelWidth, false, false, false, bssColor);
    txVector.SetHeMuUserInfo(staId, {{ruType, index, primary80}, 8, 1});
    trigVector.SetHeMuUserInfo(staId, {{ruType, index, primary80}, 8, 1});

    hdr.SetAddr2(staDev->GetMac()->GetAddress());
    Ptr<const WifiPsdu> psdu =
        Create<const WifiPsdu>(Create<Packet>(1000), hdr);

    if (duration.IsZero()) {
      duration = WifiPhy::CalculateTxDuration(
          psdu->GetSize(), txVector,
          staDev->GetMac()->GetWifiPhy()->GetPhyBand(), staId);
      std::tie(length, duration) = HePhy::ConvertHeTbPpduDurationToLSigLength(
          duration, txVector, staDev->GetMac()->GetWifiPhy()->GetPhyBand());
    }
    txVector.SetLength(length);

    staDev->GetPhy()->Send(WifiConstPsduMap{{staId, psdu}}, txVector);
  }

  trigVector.SetLength(length);
  auto apHePhy = StaticCast<HePhy>(apDev->GetPhy()->GetLatestPhyEntity());
  apHePhy->SetTrigVector(trigVector, duration);
}

void WifiPrimaryChannelsTest::CheckAssociation() {
  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    auto dev = DynamicCast<WifiNetDevice>(m_apDevices.Get(bss));
    auto mac = DynamicCast<ApWifiMac>(dev->GetMac());
    NS_TEST_EXPECT_MSG_EQ(mac->GetStaList(SINGLE_LINK_OP_ID).size(),
                          m_nStationsPerBss,
                          "Not all the stations completed association");
  }
}

void WifiPrimaryChannelsTest::CheckReceivedSuPpdus(std::set<uint8_t> txBss,
                                                   uint16_t txChannelWidth) {
  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    if (txBss.find(bss) != txBss.end()) {
      for (uint8_t sta = 0; sta < m_nStationsPerBss; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), true,
                              "Station ["
                                  << +bss << "][" << +sta
                                  << "] did not receive the SU frame on primary"
                                  << txChannelWidth << " channel");
      }
      NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(0), true,
                            "Station ["
                                << +bss << "][0]"
                                << " did not process the SU frame on primary"
                                << txChannelWidth << " channel");
      for (uint8_t sta = 1; sta < m_nStationsPerBss; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(sta), false,
                              "Station ["
                                  << +bss << "][" << +sta
                                  << "] processed the SU frame on primary"
                                  << txChannelWidth << " channel");
      }
    } else {
      if (m_useDistinctBssColors ||
          std::none_of(txBss.begin(), txBss.end(), [&](const uint8_t &txAp) {
            auto txApPhy =
                DynamicCast<WifiNetDevice>(m_apDevices.Get(txAp))->GetPhy();
            auto thisApPhy =
                DynamicCast<WifiNetDevice>(m_apDevices.Get(bss))->GetPhy();
            return txApPhy->GetOperatingChannel().GetPrimaryChannelIndex(
                       txChannelWidth) ==
                   thisApPhy->GetOperatingChannel().GetPrimaryChannelIndex(
                       txChannelWidth);
          })) {
        for (uint8_t sta = 0; sta < m_nStationsPerBss; sta++) {
          NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), false,
                                "Station ["
                                    << +bss << "][" << +sta
                                    << "] received the SU frame on primary"
                                    << txChannelWidth << " channel");
        }
      } else {
        for (uint8_t sta = 0; sta < m_nStationsPerBss; sta++) {
          NS_TEST_EXPECT_MSG_EQ(
              m_received[bss].test(sta), true,
              "Station [" << +bss << "][" << +sta
                          << "] did not receive the SU frame on primary"
                          << txChannelWidth << " channel");
          NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(sta), false,
                                "Station ["
                                    << +bss << "][" << +sta
                                    << "] processed the SU frame on primary"
                                    << txChannelWidth << " channel");
        }
      }
    }
    m_received[bss].reset();
    m_processed[bss].reset();
  }
}

void WifiPrimaryChannelsTest::CheckReceivedMuPpdus(std::set<uint8_t> txBss,
                                                   uint16_t txChannelWidth,
                                                   HeRu::RuType ruType,
                                                   std::size_t nRus,
                                                   bool isDlMu) {
  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    if (txBss.find(bss) != txBss.end()) {
      for (std::size_t sta = 0; sta < nRus; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), true,
                              (isDlMu ? "A DL MU PPDU transmitted to"
                                      : "An HE TB PPDU transmitted by")
                                  << " station [" << +bss << "][" << +sta
                                  << "] on primary" << txChannelWidth
                                  << " channel, RU type " << ruType
                                  << " was not received");
      }
      for (uint8_t sta = nRus; sta < m_nStationsPerBss; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), false,
                              (isDlMu ? "A DL MU PPDU" : "An HE TB PPDU")
                                  << " transmitted on primary" << txChannelWidth
                                  << " channel, RU type " << ruType
                                  << " was received "
                                  << (isDlMu ? "by" : "from") << " station ["
                                  << +bss << "][" << +sta << "]");
      }
      for (std::size_t sta = 0; sta < nRus; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(sta), true,
                              (isDlMu ? "A DL MU PPDU transmitted to"
                                      : "An HE TB PPDU transmitted by")
                                  << " station [" << +bss << "][" << +sta
                                  << "] on primary" << txChannelWidth
                                  << " channel, RU type " << ruType
                                  << " was not processed");
      }
      for (uint8_t sta = nRus; sta < m_nStationsPerBss; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(sta), false,
                              (isDlMu ? "A DL MU PPDU" : "An HE TB PPDU")
                                  << " transmitted on primary" << txChannelWidth
                                  << " channel, RU type " << ruType
                                  << " was received "
                                  << (isDlMu ? "by" : "from") << " station ["
                                  << +bss << "][" << +sta << "] and processed");
      }
    } else {
      if (!isDlMu || m_useDistinctBssColors ||
          std::none_of(txBss.begin(), txBss.end(), [&](const uint8_t &txAp) {
            auto txApPhy =
                DynamicCast<WifiNetDevice>(m_apDevices.Get(txAp))->GetPhy();
            auto thisApPhy =
                DynamicCast<WifiNetDevice>(m_apDevices.Get(bss))->GetPhy();
            return txApPhy->GetOperatingChannel().GetPrimaryChannelIndex(
                       txChannelWidth) ==
                   thisApPhy->GetOperatingChannel().GetPrimaryChannelIndex(
                       txChannelWidth);
          })) {
        for (uint8_t sta = 0; sta < m_nStationsPerBss; sta++) {
          NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), false,
                                (isDlMu ? "A DL MU PPDU" : "An HE TB PPDU")
                                    << " transmitted on primary"
                                    << txChannelWidth << " channel, RU type "
                                    << ruType << " was received "
                                    << (isDlMu ? "by" : "from") << " station ["
                                    << +bss << "][" << +sta << "]");
        }
      } else {
        for (std::size_t sta = 0; sta < nRus; sta++) {
          NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), true,
                                (isDlMu ? "A DL MU PPDU transmitted to"
                                        : "An HE TB PPDU transmitted by")
                                    << " station [" << +bss << "][" << +sta
                                    << "] on primary" << txChannelWidth
                                    << " channel, RU type " << ruType
                                    << " was not received");
        }
        for (uint8_t sta = nRus; sta < m_nStationsPerBss; sta++) {
          NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), false,
                                (isDlMu ? "A DL MU PPDU" : "An HE TB PPDU")
                                    << " transmitted on primary"
                                    << txChannelWidth << " channel, RU type "
                                    << ruType << " was received "
                                    << (isDlMu ? "by" : "from") << " station ["
                                    << +bss << "][" << +sta << "]");
        }
        for (uint8_t sta = 0; sta < m_nStationsPerBss; sta++) {
          NS_TEST_EXPECT_MSG_EQ(
              m_processed[bss].test(sta), false,
              (isDlMu ? "A DL MU PPDU" : "An HE TB PPDU")
                  << " transmitted on primary" << txChannelWidth
                  << " channel, RU type " << ruType << " was received "
                  << (isDlMu ? "by" : "from") << " station [" << +bss << "]["
                  << +sta << "] and processed");
        }
      }
    }
    m_received[bss].reset();
    m_processed[bss].reset();
  }
}

void WifiPrimaryChannelsTest::CheckReceivedTriggerFrames(
    std::set<uint8_t> txBss, uint16_t txChannelWidth) {
  for (uint8_t bss = 0; bss < m_nBss; bss++) {
    if (txBss.find(bss) != txBss.end()) {
      for (uint8_t sta = 0; sta < m_nStationsPerBss; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), true,
                              "Station ["
                                  << +bss << "][" << +sta
                                  << "] did not receive the Trigger Frame "
                                     "soliciting a transmission on primary"
                                  << txChannelWidth << " channel");
        NS_TEST_EXPECT_MSG_EQ(m_processed[bss].test(sta), true,
                              "Station ["
                                  << +bss << "][" << +sta
                                  << "] did not process the Trigger Frame "
                                     "soliciting a transmission on primary"
                                  << txChannelWidth << " channel");
      }
    } else {
      for (uint8_t sta = 0; sta < m_nStationsPerBss; sta++) {
        NS_TEST_EXPECT_MSG_EQ(m_received[bss].test(sta), false,
                              "Station ["
                                  << +bss << "][" << +sta
                                  << "] received the Trigger Frame soliciting "
                                     "a transmission on primary"
                                  << txChannelWidth << " channel");
      }
    }
    m_received[bss].reset();
    m_processed[bss].reset();
  }
}

class Wifi20MHzChannelIndicesTest : public TestCase {
public:
  Wifi20MHzChannelIndicesTest();
  ~Wifi20MHzChannelIndicesTest() override = default;

  void RunOne(uint8_t primary20, const std::set<uint8_t> &secondary20,
              const std::set<uint8_t> &primary40,
              const std::set<uint8_t> &secondary40,
              const std::set<uint8_t> &primary80,
              const std::set<uint8_t> &secondary80);

private:
  void DoRun() override;

  WifiPhyOperatingChannel m_channel;
};

Wifi20MHzChannelIndicesTest::Wifi20MHzChannelIndicesTest()
    : TestCase("Check computation of primary and secondary channel indices") {}

void Wifi20MHzChannelIndicesTest::RunOne(uint8_t primary20,
                                         const std::set<uint8_t> &secondary20,
                                         const std::set<uint8_t> &primary40,
                                         const std::set<uint8_t> &secondary40,
                                         const std::set<uint8_t> &primary80,
                                         const std::set<uint8_t> &secondary80) {
  auto printToStr = [](const std::set<uint8_t> &s) {
    std::stringstream ss;
    ss << "{";
    for (const auto &index : s) {
      ss << +index << " ";
    }
    ss << "}";
    return ss.str();
  };

  m_channel.SetPrimary20Index(primary20);

  auto actualPrimary20 = m_channel.GetAll20MHzChannelIndicesInPrimary(20);
  NS_TEST_ASSERT_MSG_EQ((actualPrimary20 == std::set<uint8_t>{primary20}), true,
                        "Expected Primary20 {" << +primary20 << "}"
                                               << " differs from actual "
                                               << printToStr(actualPrimary20));

  auto actualSecondary20 =
      m_channel.GetAll20MHzChannelIndicesInSecondary(actualPrimary20);
  NS_TEST_ASSERT_MSG_EQ((actualSecondary20 == secondary20), true,
                        "Expected Secondary20 "
                            << printToStr(secondary20)
                            << " differs from actual "
                            << printToStr(actualSecondary20));

  auto actualPrimary40 = m_channel.GetAll20MHzChannelIndicesInPrimary(40);
  NS_TEST_ASSERT_MSG_EQ((actualPrimary40 == primary40), true,
                        "Expected Primary40 " << printToStr(primary40)
                                              << " differs from actual "
                                              << printToStr(actualPrimary40));

  auto actualSecondary40 =
      m_channel.GetAll20MHzChannelIndicesInSecondary(primary40);
  NS_TEST_ASSERT_MSG_EQ((actualSecondary40 == secondary40), true,
                        "Expected Secondary40 "
                            << printToStr(secondary40)
                            << " differs from actual "
                            << printToStr(actualSecondary40));

  auto actualPrimary80 = m_channel.GetAll20MHzChannelIndicesInPrimary(80);
  NS_TEST_ASSERT_MSG_EQ((actualPrimary80 == primary80), true,
                        "Expected Primary80 " << printToStr(primary80)
                                              << " differs from actual "
                                              << printToStr(actualPrimary80));

  auto actualSecondary80 =
      m_channel.GetAll20MHzChannelIndicesInSecondary(primary80);
  NS_TEST_ASSERT_MSG_EQ((actualSecondary80 == secondary80), true,
                        "Expected Secondary80 "
                            << printToStr(secondary80)
                            << " differs from actual "
                            << printToStr(actualSecondary80));
}

void Wifi20MHzChannelIndicesTest::DoRun() {
  m_channel.SetDefault(20, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);
  RunOne(0, {}, {}, {}, {}, {});

  m_channel.SetDefault(40, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);
  RunOne(0, {1}, {0, 1}, {}, {}, {});
  RunOne(1, {0}, {0, 1}, {}, {}, {});

  m_channel.SetDefault(80, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);
  RunOne(0, {1}, {0, 1}, {2, 3}, {0, 1, 2, 3}, {});
  RunOne(1, {0}, {0, 1}, {2, 3}, {0, 1, 2, 3}, {});
  RunOne(2, {3}, {2, 3}, {0, 1}, {0, 1, 2, 3}, {});
  RunOne(3, {2}, {2, 3}, {0, 1}, {0, 1, 2, 3}, {});

  m_channel.SetDefault(160, WIFI_STANDARD_80211ax, WIFI_PHY_BAND_5GHZ);
  RunOne(0, {1}, {0, 1}, {2, 3}, {0, 1, 2, 3}, {4, 5, 6, 7});
  RunOne(1, {0}, {0, 1}, {2, 3}, {0, 1, 2, 3}, {4, 5, 6, 7});
  RunOne(2, {3}, {2, 3}, {0, 1}, {0, 1, 2, 3}, {4, 5, 6, 7});
  RunOne(3, {2}, {2, 3}, {0, 1}, {0, 1, 2, 3}, {4, 5, 6, 7});
  RunOne(4, {5}, {4, 5}, {6, 7}, {4, 5, 6, 7}, {0, 1, 2, 3});
  RunOne(5, {4}, {4, 5}, {6, 7}, {4, 5, 6, 7}, {0, 1, 2, 3});
  RunOne(6, {7}, {6, 7}, {4, 5}, {4, 5, 6, 7}, {0, 1, 2, 3});
  RunOne(7, {6}, {6, 7}, {4, 5}, {4, 5, 6, 7}, {0, 1, 2, 3});
}

class WifiPrimaryChannelsTestSuite : public TestSuite {
public:
  WifiPrimaryChannelsTestSuite();
};

WifiPrimaryChannelsTestSuite::WifiPrimaryChannelsTestSuite()
    : TestSuite("wifi-primary-channels", UNIT) {
  AddTestCase(new WifiPrimaryChannelsTest(40, true), TestCase::QUICK);
  AddTestCase(new WifiPrimaryChannelsTest(40, false), TestCase::QUICK);
#if 0
    AddTestCase(new WifiPrimaryChannelsTest(80, true), TestCase::EXTENSIVE);
    AddTestCase(new WifiPrimaryChannelsTest(80, false), TestCase::EXTENSIVE);
    AddTestCase(new WifiPrimaryChannelsTest(160, true), TestCase::TAKES_FOREVER);
    AddTestCase(new WifiPrimaryChannelsTest(160, false), TestCase::TAKES_FOREVER);
#endif
  AddTestCase(new Wifi20MHzChannelIndicesTest(), TestCase::QUICK);
}

static WifiPrimaryChannelsTestSuite g_wifiPrimaryChannelsTestSuite;
