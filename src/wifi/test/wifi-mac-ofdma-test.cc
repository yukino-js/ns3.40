
#include "ns3/config.h"
#include "ns3/he-configuration.h"
#include "ns3/he-frame-exchange-manager.h"
#include "ns3/he-phy.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/multi-user-scheduler.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/packet.h"
#include "ns3/qos-utils.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/wifi-acknowledgment.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-protection.h"
#include "ns3/wifi-psdu.h"

#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiMacOfdmaTestSuite");

class TestMultiUserScheduler : public MultiUserScheduler {
public:
  static TypeId GetTypeId();
  TestMultiUserScheduler();
  ~TestMultiUserScheduler() override;

private:
  TxFormat SelectTxFormat() override;
  DlMuInfo ComputeDlMuInfo() override;
  UlMuInfo ComputeUlMuInfo() override;

  void ComputeWifiTxVector();

  TxFormat m_txFormat;
  TriggerFrameType m_ulTriggerType;
  CtrlTriggerHeader m_trigger;
  WifiMacHeader m_triggerHdr;
  WifiTxVector m_txVector;
  WifiTxParameters m_txParams;
  WifiPsduMap m_psduMap;
  WifiModulationClass m_modClass;
};

NS_OBJECT_ENSURE_REGISTERED(TestMultiUserScheduler);

TypeId TestMultiUserScheduler::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TestMultiUserScheduler")
          .SetParent<MultiUserScheduler>()
          .SetGroupName("Wifi")
          .AddConstructor<TestMultiUserScheduler>()
          .AddAttribute("ModulationClass",
                        "Modulation class for DL MU PPDUs and TB PPDUs.",
                        EnumValue(WIFI_MOD_CLASS_HE),
                        MakeEnumAccessor(&TestMultiUserScheduler::m_modClass),
                        MakeEnumChecker(WIFI_MOD_CLASS_HE, "HE",
                                        WIFI_MOD_CLASS_EHT, "EHT"));
  return tid;
}

TestMultiUserScheduler::TestMultiUserScheduler()
    : m_txFormat(SU_TX), m_ulTriggerType(TriggerFrameType::BSRP_TRIGGER) {
  NS_LOG_FUNCTION(this);
}

TestMultiUserScheduler::~TestMultiUserScheduler() { NS_LOG_FUNCTION_NOARGS(); }

MultiUserScheduler::TxFormat TestMultiUserScheduler::SelectTxFormat() {
  NS_LOG_FUNCTION(this);

  if (Simulator::Now() < Seconds(1.5)) {
    NS_LOG_DEBUG("Return SU_TX");
    return SU_TX;
  }

  ComputeWifiTxVector();

  if (m_txFormat == SU_TX || m_txFormat == DL_MU_TX ||
      (m_txFormat == UL_MU_TX &&
       m_ulTriggerType == TriggerFrameType::BSRP_TRIGGER)) {
    TriggerFrameType ulTriggerType =
        (m_txFormat == SU_TX || m_txFormat == DL_MU_TX
             ? TriggerFrameType::BSRP_TRIGGER
             : TriggerFrameType::BASIC_TRIGGER);

    WifiTxVector txVector = m_txVector;
    txVector.SetPreambleType(m_modClass == WIFI_MOD_CLASS_HE
                                 ? WIFI_PREAMBLE_HE_TB
                                 : WIFI_PREAMBLE_EHT_TB);
    m_trigger = CtrlTriggerHeader(ulTriggerType, txVector);

    txVector.SetGuardInterval(m_trigger.GetGuardInterval());

    uint32_t ampduSize = (ulTriggerType == TriggerFrameType::BSRP_TRIGGER)
                             ? GetMaxSizeOfQosNullAmpdu(m_trigger)
                             : 3500;

    auto staList = m_apMac->GetStaList(SINGLE_LINK_OP_ID);
    for (auto it = staList.begin(); it != staList.end();) {
      it = m_apMac->GetHeSupported(it->second) ? std::next(it)
                                               : staList.erase(it);
    }

    Time duration = WifiPhy::CalculateTxDuration(
        ampduSize, txVector, m_apMac->GetWifiPhy()->GetPhyBand(),
        staList.begin()->first);

    uint16_t length;
    std::tie(length, duration) = HePhy::ConvertHeTbPpduDurationToLSigLength(
        duration, m_trigger.GetHeTbTxVector(m_trigger.begin()->GetAid12()),
        m_apMac->GetWifiPhy()->GetPhyBand());
    m_trigger.SetUlLength(length);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(m_trigger);

    m_triggerHdr = WifiMacHeader(WIFI_MAC_CTL_TRIGGER);
    m_triggerHdr.SetAddr1(Mac48Address::GetBroadcast());
    m_triggerHdr.SetAddr2(m_apMac->GetAddress());
    m_triggerHdr.SetDsNotTo();
    m_triggerHdr.SetDsNotFrom();

    auto item = Create<WifiMpdu>(packet, m_triggerHdr);

    m_txParams.Clear();
    m_txParams.m_txVector =
        m_apMac->GetWifiRemoteStationManager()->GetRtsTxVector(
            m_triggerHdr.GetAddr1());

    if (!GetHeFem(SINGLE_LINK_OP_ID)
             ->TryAddMpdu(item, m_txParams, m_availableTime) ||
        (m_availableTime != Time::Min() &&
         m_txParams.m_protection->protectionTime + m_txParams.m_txDuration +
                 m_apMac->GetWifiPhy()->GetSifs() + duration +
                 m_txParams.m_acknowledgment->acknowledgmentTime >
             m_availableTime)) {
      NS_LOG_DEBUG(
          "Remaining TXOP duration is not enough for BSRP TF exchange");
      return SU_TX;
    }

    m_txFormat = UL_MU_TX;
    m_ulTriggerType = ulTriggerType;
  } else if (m_txFormat == UL_MU_TX) {
    m_psduMap.clear();
    auto staList = m_apMac->GetStaList(SINGLE_LINK_OP_ID);
    for (auto it = staList.cbegin(); it != staList.cend();) {
      it = m_apMac->GetHeSupported(it->second) ? std::next(it)
                                               : staList.erase(it);
    }
    NS_ABORT_MSG_IF(staList.size() != 4, "There must be 4 associated stations");

    m_txParams.Clear();
    m_txParams.m_txVector = m_txVector;

    for (auto &sta : staList) {
      Ptr<WifiMpdu> peeked;
      uint8_t tid;

      for (tid = 0; tid < 8; tid++) {
        peeked = m_apMac->GetQosTxop(tid)->PeekNextMpdu(SINGLE_LINK_OP_ID, tid,
                                                        sta.second);
        if (peeked) {
          break;
        }
      }

      if (!peeked) {
        NS_LOG_DEBUG("No frame to send to " << sta.second);
        continue;
      }

      Ptr<WifiMpdu> mpdu = m_apMac->GetQosTxop(tid)->GetNextMpdu(
          SINGLE_LINK_OP_ID, peeked, m_txParams, m_availableTime,
          m_initialFrame);
      if (!mpdu) {
        NS_LOG_DEBUG("Not enough time to send frames to all the stations");
        return SU_TX;
      }

      std::vector<Ptr<WifiMpdu>> mpduList;
      mpduList = GetHeFem(SINGLE_LINK_OP_ID)
                     ->GetMpduAggregator()
                     ->GetNextAmpdu(mpdu, m_txParams, m_availableTime);

      if (mpduList.size() > 1) {
        m_psduMap[sta.first] = Create<WifiPsdu>(std::move(mpduList));
      } else {
        m_psduMap[sta.first] = Create<WifiPsdu>(mpdu, true);
      }
    }

    if (m_psduMap.empty()) {
      NS_LOG_DEBUG("No frame to send");
      return SU_TX;
    }

    m_txFormat = DL_MU_TX;
  } else {
    NS_ABORT_MSG("Cannot get here.");
  }

  NS_LOG_DEBUG("Return " << m_txFormat);
  return m_txFormat;
}

void TestMultiUserScheduler::ComputeWifiTxVector() {
  if (m_txVector.IsDlMu()) {
    return;
  }

  uint16_t bw = m_apMac->GetWifiPhy()->GetChannelWidth();

  m_txVector.SetPreambleType(m_modClass == WIFI_MOD_CLASS_HE
                                 ? WIFI_PREAMBLE_HE_MU
                                 : WIFI_PREAMBLE_EHT_MU);
  if (IsEht(m_txVector.GetPreambleType())) {
    m_txVector.SetEhtPpduType(0);
  }
  m_txVector.SetChannelWidth(bw);
  m_txVector.SetGuardInterval(
      m_apMac->GetHeConfiguration()->GetGuardInterval().GetNanoSeconds());
  m_txVector.SetTxPowerLevel(
      GetWifiRemoteStationManager(SINGLE_LINK_OP_ID)->GetDefaultTxPowerLevel());

  auto staList = m_apMac->GetStaList(SINGLE_LINK_OP_ID);
  for (auto it = staList.cbegin(); it != staList.cend();) {
    it =
        m_apMac->GetHeSupported(it->second) ? std::next(it) : staList.erase(it);
  }
  NS_ABORT_MSG_IF(staList.size() != 4, "There must be 4 associated stations");

  HeRu::RuType ruType;
  switch (bw) {
  case 20:
    ruType = HeRu::RU_52_TONE;
    m_txVector.SetRuAllocation({112}, 0);
    break;
  case 40:
    ruType = HeRu::RU_106_TONE;
    m_txVector.SetRuAllocation({96, 96}, 0);
    break;
  case 80:
    ruType = HeRu::RU_242_TONE;
    m_txVector.SetRuAllocation({192, 192, 192, 192}, 0);
    break;
  case 160:
    ruType = HeRu::RU_484_TONE;
    m_txVector.SetRuAllocation({200, 200, 200, 200, 200, 200, 200, 200}, 0);
    break;
  default:
    NS_ABORT_MSG("Unsupported channel width");
  }

  bool primary80 = true;
  std::size_t ruIndex = 1;

  for (auto &sta : staList) {
    if (bw == 160 && ruIndex == 3) {
      ruIndex = 1;
      primary80 = false;
    }
    m_txVector.SetHeMuUserInfo(sta.first,
                               {{ruType, ruIndex++, primary80}, 11, 1});
  }
  m_txVector.SetSigBMode(VhtPhy::GetVhtMcs5());
}

MultiUserScheduler::DlMuInfo TestMultiUserScheduler::ComputeDlMuInfo() {
  NS_LOG_FUNCTION(this);
  return DlMuInfo{m_psduMap, std::move(m_txParams)};
}

MultiUserScheduler::UlMuInfo TestMultiUserScheduler::ComputeUlMuInfo() {
  NS_LOG_FUNCTION(this);
  return UlMuInfo{m_trigger, m_triggerHdr, std::move(m_txParams)};
}

enum class WifiOfdmaScenario : uint8_t { HE = 0, HE_EHT, EHT };

class OfdmaAckSequenceTest : public TestCase {
public:
  struct MuEdcaParameterSet {
    uint8_t muAifsn;
    uint16_t muCwMin;
    uint16_t muCwMax;
    uint8_t muTimer;
  };

  OfdmaAckSequenceTest(uint16_t width, WifiAcknowledgment::Method dlType,
                       uint32_t maxAmpduSize, uint16_t txopLimit,
                       uint16_t nPktsPerSta,
                       MuEdcaParameterSet muEdcaParameterSet,
                       WifiOfdmaScenario scenario);
  ~OfdmaAckSequenceTest() override;

  void L7Receive(std::string context, Ptr<const Packet> p, const Address &addr);
  void TraceCw(uint32_t staIndex, uint32_t cw, uint8_t);
  void Transmit(std::string context, WifiConstPsduMap psduMap,
                WifiTxVector txVector, double txPowerW);
  void CheckResults(Time sifs, Time slotTime, uint8_t aifsn);

private:
  void DoRun() override;

  static constexpr uint16_t m_muTimerRes = 8192;

  struct FrameInfo {
    Time startTx;
    Time endTx;
    WifiConstPsduMap psduMap;
    WifiTxVector txVector;
  };

  uint16_t m_nStations;
  NetDeviceContainer m_staDevices;
  Ptr<WifiNetDevice> m_apDevice;
  std::vector<PacketSocketAddress> m_sockets;
  uint16_t m_channelWidth;
  uint8_t m_muRtsRuAllocation;
  std::vector<FrameInfo> m_txPsdus;
  WifiAcknowledgment::Method m_dlMuAckType;
  uint32_t m_maxAmpduSize;
  uint16_t m_txopLimit;
  uint16_t m_nPktsPerSta;
  MuEdcaParameterSet m_muEdcaParameterSet;
  WifiOfdmaScenario m_scenario;
  WifiPreamble m_dlMuPreamble;
  WifiPreamble m_tbPreamble;
  bool m_ulPktsGenerated;
  uint16_t m_received;
  uint16_t m_flushed;
  Time m_edcaDisabledStartTime;
  std::vector<uint32_t> m_cwValues;
};

OfdmaAckSequenceTest::OfdmaAckSequenceTest(
    uint16_t width, WifiAcknowledgment::Method dlType, uint32_t maxAmpduSize,
    uint16_t txopLimit, uint16_t nPktsPerSta,
    MuEdcaParameterSet muEdcaParameterSet, WifiOfdmaScenario scenario)
    : TestCase("Check correct operation of DL OFDMA acknowledgment sequences"),
      m_nStations(4), m_sockets(m_nStations), m_channelWidth(width),
      m_dlMuAckType(dlType), m_maxAmpduSize(maxAmpduSize),
      m_txopLimit(txopLimit), m_nPktsPerSta(nPktsPerSta),
      m_muEdcaParameterSet(muEdcaParameterSet), m_scenario(scenario),
      m_ulPktsGenerated(false), m_received(0), m_flushed(0),
      m_edcaDisabledStartTime(Seconds(0)),
      m_cwValues(std::vector<uint32_t>(m_nStations, 2)) {
  switch (m_scenario) {
  case WifiOfdmaScenario::HE:
  case WifiOfdmaScenario::HE_EHT:
    m_dlMuPreamble = WIFI_PREAMBLE_HE_MU;
    m_tbPreamble = WIFI_PREAMBLE_HE_TB;
    break;
  case WifiOfdmaScenario::EHT:
    m_dlMuPreamble = WIFI_PREAMBLE_EHT_MU;
    m_tbPreamble = WIFI_PREAMBLE_EHT_TB;
    break;
  }

  switch (m_channelWidth) {
  case 20:
    m_muRtsRuAllocation = 61;
    break;
  case 40:
    m_muRtsRuAllocation = 65;
    break;
  case 80:
    m_muRtsRuAllocation = 67;
    break;
  case 160:
    m_muRtsRuAllocation = 68;
    break;
  default:
    NS_ABORT_MSG("Unhandled channel width (" << m_channelWidth << " MHz)");
  }

  m_txPsdus.reserve(35);
}

OfdmaAckSequenceTest::~OfdmaAckSequenceTest() {}

void OfdmaAckSequenceTest::L7Receive(std::string context, Ptr<const Packet> p,
                                     const Address &addr) {
  if (p->GetSize() >= 1400 && Simulator::Now() > Seconds(1.5)) {
    m_received++;
  }
}

void OfdmaAckSequenceTest::TraceCw(uint32_t staIndex, uint32_t cw, uint8_t) {
  if (m_cwValues.at(staIndex) == 2) {
    m_cwValues[staIndex] = cw;
  }
}

void OfdmaAckSequenceTest::Transmit(std::string context,
                                    WifiConstPsduMap psduMap,
                                    WifiTxVector txVector, double txPowerW) {
  if (!psduMap.begin()->second->GetHeader(0).IsBeacon() &&
      Simulator::Now() >= Seconds(1.5)) {
    Time txDuration =
        WifiPhy::CalculateTxDuration(psduMap, txVector, WIFI_PHY_BAND_5GHZ);
    m_txPsdus.push_back(
        {Simulator::Now(), Simulator::Now() + txDuration, psduMap, txVector});

    for (const auto &[staId, psdu] : psduMap) {
      NS_LOG_INFO("Sending "
                  << psdu->GetHeader(0).GetTypeString() << " #MPDUs "
                  << psdu->GetNMpdus()
                  << (psdu->GetHeader(0).IsQosData()
                          ? " TID " + std::to_string(*psdu->GetTids().begin())
                          : "")
                  << std::setprecision(10) << " txDuration " << txDuration
                  << " duration/ID " << psdu->GetHeader(0).GetDuration()
                  << " #TX PSDUs = " << m_txPsdus.size()
                  << " size=" << (*psdu->begin())->GetSize() << "\n"
                  << "TXVECTOR: " << txVector << "\n");
    }
  }

  if (txVector.GetPreambleType() == m_dlMuPreamble) {
    m_flushed = 0;
    for (uint32_t i = 0; i < m_staDevices.GetN(); i++) {
      auto queue = m_apDevice->GetMac()
                       ->GetQosTxop(static_cast<AcIndex>(i))
                       ->GetWifiMacQueue();
      auto staDev = DynamicCast<WifiNetDevice>(m_staDevices.Get(i));
      Ptr<const WifiMpdu> lastInFlight = nullptr;
      Ptr<const WifiMpdu> mpdu;

      while ((mpdu = queue->PeekByTidAndAddress(i * 2,
                                                staDev->GetMac()->GetAddress(),
                                                lastInFlight)) != nullptr) {
        if (mpdu->IsInFlight()) {
          lastInFlight = mpdu;
        } else {
          queue->Remove(mpdu);
          m_flushed++;
        }
      }
    }
  } else if (txVector.GetPreambleType() == m_tbPreamble &&
             psduMap.begin()->second->GetHeader(0).HasData()) {
    Mac48Address sender = psduMap.begin()->second->GetAddr2();

    for (uint32_t i = 0; i < m_staDevices.GetN(); i++) {
      auto dev = DynamicCast<WifiNetDevice>(m_staDevices.Get(i));

      if (dev->GetAddress() == sender) {
        Ptr<QosTxop> qosTxop =
            dev->GetMac()->GetQosTxop(static_cast<AcIndex>(i));

        if (m_muEdcaParameterSet.muTimer > 0 &&
            m_muEdcaParameterSet.muAifsn > 0) {
          qosTxop->TraceConnectWithoutContext(
              "CwTrace",
              MakeCallback(&OfdmaAckSequenceTest::TraceCw, this).Bind(i));
        } else {
          qosTxop->GetWifiMacQueue()->Flush();
        }
        break;
      }
    }
  } else if (!txVector.IsMu() &&
             psduMap.begin()->second->GetHeader(0).IsBlockAck() &&
             psduMap.begin()->second->GetHeader(0).GetAddr2() ==
                 m_apDevice->GetAddress() &&
             m_muEdcaParameterSet.muTimer > 0 &&
             m_muEdcaParameterSet.muAifsn == 0) {
    CtrlBAckResponseHeader blockAck;
    psduMap.begin()->second->GetPayload(0)->PeekHeader(blockAck);

    if (blockAck.IsMultiSta()) {
      m_edcaDisabledStartTime =
          Simulator::Now() + m_txPsdus.back().endTx - m_txPsdus.back().startTx;
    }
  } else if (!txVector.IsMu() &&
             psduMap.begin()->second->GetHeader(0).IsTrigger() &&
             !m_ulPktsGenerated) {
    CtrlTriggerHeader trigger;
    psduMap.begin()->second->GetPayload(0)->PeekHeader(trigger);
    if (trigger.IsBasic()) {
      Time txDuration =
          WifiPhy::CalculateTxDuration(psduMap, txVector, WIFI_PHY_BAND_5GHZ);
      for (uint16_t i = 0; i < m_nStations; i++) {
        Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
        client->SetAttribute("PacketSize", UintegerValue(1400 + i * 100));
        client->SetAttribute("MaxPackets", UintegerValue(m_nPktsPerSta));
        client->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
        client->SetAttribute("Priority", UintegerValue(i * 2));
        client->SetRemote(m_sockets[i]);
        m_staDevices.Get(i)->GetNode()->AddApplication(client);
        client->SetStartTime(txDuration);
        client->SetStopTime(Seconds(1.0));
        client->Initialize();
      }
      m_ulPktsGenerated = true;
    }
  }
}

void OfdmaAckSequenceTest::CheckResults(Time sifs, Time slotTime,
                                        uint8_t aifsn) {
  CtrlTriggerHeader trigger;
  CtrlBAckResponseHeader blockAck;
  Time tEnd;
  Time tStart;
  Time tolerance = NanoSeconds(500);
  Time ifs = (m_txopLimit > 0 ? sifs : sifs + aifsn * slotTime);
  Time navEnd;

  NS_TEST_ASSERT_MSG_GT_OR_EQ(m_txPsdus.size(), 5,
                              "Expected at least 5 transmitted packet");
  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[0].psduMap.size() == 1 &&
       m_txPsdus[0].psduMap[SU_STA_ID]->GetHeader(0).IsTrigger() &&
       m_txPsdus[0].psduMap[SU_STA_ID]->GetHeader(0).GetAddr1().IsBroadcast()),
      true, "Expected a Trigger Frame");
  m_txPsdus[0].psduMap[SU_STA_ID]->GetPayload(0)->PeekHeader(trigger);
  NS_TEST_EXPECT_MSG_EQ(trigger.IsMuRts(), true,
                        "Expected an MU-RTS Trigger Frame");
  NS_TEST_EXPECT_MSG_EQ(trigger.GetNUserInfoFields(), 4,
                        "Expected one User Info field per station");
  NS_TEST_EXPECT_MSG_EQ(
      m_txPsdus[0].txVector.GetChannelWidth(), m_channelWidth,
      "Expected the MU-RTS to occupy the entire channel width");
  for (const auto &userInfo : trigger) {
    NS_TEST_EXPECT_MSG_EQ(+userInfo.GetMuRtsRuAllocation(),
                          +m_muRtsRuAllocation,
                          "Unexpected RU Allocation value in MU-RTS");
  }
  tEnd = m_txPsdus[0].endTx;
  navEnd = tEnd + m_txPsdus[0].psduMap[SU_STA_ID]->GetDuration();

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[1].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
       m_txPsdus[1].psduMap.size() == 1 &&
       m_txPsdus[1].psduMap.begin()->second->GetNMpdus() == 1 &&
       m_txPsdus[1].psduMap.begin()->second->GetHeader(0).GetType() ==
           WIFI_MAC_CTL_CTS),
      true, "Expected a CTS frame");
  NS_TEST_EXPECT_MSG_EQ(m_txPsdus[1].txVector.GetChannelWidth(), m_channelWidth,
                        "Expected the CTS to occupy the entire channel width");

  tStart = m_txPsdus[1].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CTS frame sent too late");
  Time ctsNavEnd =
      m_txPsdus[1].endTx + m_txPsdus[1].psduMap[SU_STA_ID]->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                              "Duration/ID in CTS frame is too short");
  NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                        "Duration/ID in CTS frame is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[2].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
       m_txPsdus[2].psduMap.size() == 1 &&
       m_txPsdus[2].psduMap.begin()->second->GetNMpdus() == 1 &&
       m_txPsdus[2].psduMap.begin()->second->GetHeader(0).GetType() ==
           WIFI_MAC_CTL_CTS),
      true, "Expected a CTS frame");
  NS_TEST_EXPECT_MSG_EQ(m_txPsdus[2].txVector.GetChannelWidth(), m_channelWidth,
                        "Expected the CTS to occupy the entire channel width");

  tStart = m_txPsdus[2].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CTS frame sent too late");
  ctsNavEnd =
      m_txPsdus[2].endTx + m_txPsdus[2].psduMap[SU_STA_ID]->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                              "Duration/ID in CTS frame is too short");
  NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                        "Duration/ID in CTS frame is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[3].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
       m_txPsdus[3].psduMap.size() == 1 &&
       m_txPsdus[3].psduMap.begin()->second->GetNMpdus() == 1 &&
       m_txPsdus[3].psduMap.begin()->second->GetHeader(0).GetType() ==
           WIFI_MAC_CTL_CTS),
      true, "Expected a CTS frame");
  NS_TEST_EXPECT_MSG_EQ(m_txPsdus[3].txVector.GetChannelWidth(), m_channelWidth,
                        "Expected the CTS to occupy the entire channel width");

  tStart = m_txPsdus[3].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CTS frame sent too late");
  ctsNavEnd =
      m_txPsdus[3].endTx + m_txPsdus[3].psduMap[SU_STA_ID]->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                              "Duration/ID in CTS frame is too short");
  NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                        "Duration/ID in CTS frame is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[4].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
       m_txPsdus[4].psduMap.size() == 1 &&
       m_txPsdus[4].psduMap.begin()->second->GetNMpdus() == 1 &&
       m_txPsdus[4].psduMap.begin()->second->GetHeader(0).GetType() ==
           WIFI_MAC_CTL_CTS),
      true, "Expected a CTS frame");
  NS_TEST_EXPECT_MSG_EQ(m_txPsdus[4].txVector.GetChannelWidth(), m_channelWidth,
                        "Expected the CTS to occupy the entire channel width");

  tStart = m_txPsdus[4].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "CTS frame sent too late");
  ctsNavEnd =
      m_txPsdus[4].endTx + m_txPsdus[4].psduMap[SU_STA_ID]->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                              "Duration/ID in CTS frame is too short");
  NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                        "Duration/ID in CTS frame is too long");

  NS_TEST_ASSERT_MSG_GT_OR_EQ(m_txPsdus.size(), 10,
                              "Expected at least 10 transmitted packet");
  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[5].psduMap.size() == 1 &&
       m_txPsdus[5].psduMap[SU_STA_ID]->GetHeader(0).IsTrigger() &&
       m_txPsdus[5].psduMap[SU_STA_ID]->GetHeader(0).GetAddr1().IsBroadcast()),
      true, "Expected a Trigger Frame");
  m_txPsdus[5].psduMap[SU_STA_ID]->GetPayload(0)->PeekHeader(trigger);
  NS_TEST_EXPECT_MSG_EQ(trigger.IsBsrp(), true,
                        "Expected a BSRP Trigger Frame");
  NS_TEST_EXPECT_MSG_EQ(trigger.GetNUserInfoFields(), 4,
                        "Expected one User Info field per station");
  tEnd = m_txPsdus[4].endTx;
  tStart = m_txPsdus[5].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "BSRP Trigger Frame sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "BSRP Trigger Frame sent too late");
  Time bsrpNavEnd =
      m_txPsdus[5].endTx + m_txPsdus[5].psduMap[SU_STA_ID]->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, bsrpNavEnd,
                              "Duration/ID in BSRP TF is too short");
  NS_TEST_EXPECT_MSG_LT(bsrpNavEnd, navEnd + tolerance,
                        "Duration/ID in BSRP TF is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[6].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[6].psduMap.size() == 1 &&
       m_txPsdus[6].psduMap.begin()->second->GetNMpdus() == 1),
      true, "Expected a QoS Null frame in a TB PPDU");
  {
    const WifiMacHeader &hdr =
        m_txPsdus[6].psduMap.begin()->second->GetHeader(0);
    NS_TEST_EXPECT_MSG_EQ(hdr.GetType(), WIFI_MAC_QOSDATA_NULL,
                          "Expected a QoS Null frame");
    uint16_t staId;
    for (staId = 0; staId < m_nStations; staId++) {
      if (DynamicCast<WifiNetDevice>(m_staDevices.Get(staId))->GetAddress() ==
          hdr.GetAddr2()) {
        break;
      }
    }
    NS_TEST_EXPECT_MSG_NE(+staId, m_nStations,
                          "Sender not found among stations");
    uint8_t tid = staId * 2;
    NS_TEST_EXPECT_MSG_EQ(+hdr.GetQosTid(), +tid,
                          "Expected a TID equal to " << +tid);
  }
  tEnd = m_txPsdus[5].endTx;
  tStart = m_txPsdus[6].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS Null frame in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS Null frame in HE TB PPDU sent too late");
  Time qosNullNavEnd =
      m_txPsdus[6].endTx + m_txPsdus[6].psduMap.begin()->second->GetDuration();
  if (m_txopLimit == 0) {
    NS_TEST_EXPECT_MSG_EQ(
        qosNullNavEnd, m_txPsdus[6].endTx,
        "Expected null Duration/ID for QoS Null frame in HE TB PPDU");
  }
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosNullNavEnd,
                              "Duration/ID in QoS Null is too short");
  NS_TEST_EXPECT_MSG_LT(qosNullNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Null is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[7].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[7].psduMap.size() == 1 &&
       m_txPsdus[7].psduMap.begin()->second->GetNMpdus() == 1),
      true, "Expected a QoS Null frame in a TB PPDU");
  {
    const WifiMacHeader &hdr =
        m_txPsdus[7].psduMap.begin()->second->GetHeader(0);
    NS_TEST_EXPECT_MSG_EQ(hdr.GetType(), WIFI_MAC_QOSDATA_NULL,
                          "Expected a QoS Null frame");
    uint16_t staId;
    for (staId = 0; staId < m_nStations; staId++) {
      if (DynamicCast<WifiNetDevice>(m_staDevices.Get(staId))->GetAddress() ==
          hdr.GetAddr2()) {
        break;
      }
    }
    NS_TEST_EXPECT_MSG_NE(+staId, m_nStations,
                          "Sender not found among stations");
    uint8_t tid = staId * 2;
    NS_TEST_EXPECT_MSG_EQ(+hdr.GetQosTid(), +tid,
                          "Expected a TID equal to " << +tid);
  }
  tStart = m_txPsdus[7].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS Null frame in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS Null frame in HE TB PPDU sent too late");
  qosNullNavEnd =
      m_txPsdus[7].endTx + m_txPsdus[7].psduMap.begin()->second->GetDuration();
  if (m_txopLimit == 0) {
    NS_TEST_EXPECT_MSG_EQ(
        qosNullNavEnd, m_txPsdus[7].endTx,
        "Expected null Duration/ID for QoS Null frame in HE TB PPDU");
  }
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosNullNavEnd,
                              "Duration/ID in QoS Null is too short");
  NS_TEST_EXPECT_MSG_LT(qosNullNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Null is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[8].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[8].psduMap.size() == 1 &&
       m_txPsdus[8].psduMap.begin()->second->GetNMpdus() == 1),
      true, "Expected a QoS Null frame in an HE TB PPDU");
  {
    const WifiMacHeader &hdr =
        m_txPsdus[8].psduMap.begin()->second->GetHeader(0);
    NS_TEST_EXPECT_MSG_EQ(hdr.GetType(), WIFI_MAC_QOSDATA_NULL,
                          "Expected a QoS Null frame");
    uint16_t staId;
    for (staId = 0; staId < m_nStations; staId++) {
      if (DynamicCast<WifiNetDevice>(m_staDevices.Get(staId))->GetAddress() ==
          hdr.GetAddr2()) {
        break;
      }
    }
    NS_TEST_EXPECT_MSG_NE(+staId, m_nStations,
                          "Sender not found among stations");
    uint8_t tid = staId * 2;
    NS_TEST_EXPECT_MSG_EQ(+hdr.GetQosTid(), +tid,
                          "Expected a TID equal to " << +tid);
  }
  tStart = m_txPsdus[8].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS Null frame in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS Null frame in HE TB PPDU sent too late");
  qosNullNavEnd =
      m_txPsdus[8].endTx + m_txPsdus[8].psduMap.begin()->second->GetDuration();
  if (m_txopLimit == 0) {
    NS_TEST_EXPECT_MSG_EQ(
        qosNullNavEnd, m_txPsdus[8].endTx,
        "Expected null Duration/ID for QoS Null frame in HE TB PPDU");
  }
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosNullNavEnd,
                              "Duration/ID in QoS Null is too short");
  NS_TEST_EXPECT_MSG_LT(qosNullNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Null is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[9].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[9].psduMap.size() == 1 &&
       m_txPsdus[9].psduMap.begin()->second->GetNMpdus() == 1),
      true, "Expected a QoS Null frame in an HE TB PPDU");
  {
    const WifiMacHeader &hdr =
        m_txPsdus[9].psduMap.begin()->second->GetHeader(0);
    NS_TEST_EXPECT_MSG_EQ(hdr.GetType(), WIFI_MAC_QOSDATA_NULL,
                          "Expected a QoS Null frame");
    uint16_t staId;
    for (staId = 0; staId < m_nStations; staId++) {
      if (DynamicCast<WifiNetDevice>(m_staDevices.Get(staId))->GetAddress() ==
          hdr.GetAddr2()) {
        break;
      }
    }
    NS_TEST_EXPECT_MSG_NE(+staId, m_nStations,
                          "Sender not found among stations");
    uint8_t tid = staId * 2;
    NS_TEST_EXPECT_MSG_EQ(+hdr.GetQosTid(), +tid,
                          "Expected a TID equal to " << +tid);
  }
  tStart = m_txPsdus[9].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS Null frame in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS Null frame in HE TB PPDU sent too late");
  qosNullNavEnd =
      m_txPsdus[9].endTx + m_txPsdus[9].psduMap.begin()->second->GetDuration();
  if (m_txopLimit == 0) {
    NS_TEST_EXPECT_MSG_EQ(
        qosNullNavEnd, m_txPsdus[9].endTx,
        "Expected null Duration/ID for QoS Null frame in HE TB PPDU");
  }
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosNullNavEnd,
                              "Duration/ID in QoS Null is too short");
  NS_TEST_EXPECT_MSG_LT(qosNullNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Null is too long");

  tEnd = m_txPsdus[9].endTx;
  tStart = m_txPsdus[10].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + ifs, tStart,
                        "Basic Trigger Frame sent too early");
  if (m_txopLimit > 0) {
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Basic Trigger Frame sent too late");
    auto muRtsNavEnd =
        m_txPsdus[10].endTx + m_txPsdus[10].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, muRtsNavEnd,
                                "Duration/ID in MU-RTS is too short");
    NS_TEST_EXPECT_MSG_LT(muRtsNavEnd, navEnd + tolerance,
                          "Duration/ID in MU-RTS is too long");
  }

  if (m_txopLimit == 0) {
    NS_TEST_ASSERT_MSG_GT_OR_EQ(m_txPsdus.size(), 15,
                                "Expected at least 15 transmitted packet");
    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[10].psduMap.size() == 1 &&
         m_txPsdus[10].psduMap[SU_STA_ID]->GetHeader(0).IsTrigger() &&
         m_txPsdus[10]
             .psduMap[SU_STA_ID]
             ->GetHeader(0)
             .GetAddr1()
             .IsBroadcast()),
        true, "Expected a Trigger Frame");
    m_txPsdus[10].psduMap[SU_STA_ID]->GetPayload(0)->PeekHeader(trigger);
    NS_TEST_EXPECT_MSG_EQ(trigger.IsMuRts(), true,
                          "Expected an MU-RTS Trigger Frame");
    NS_TEST_EXPECT_MSG_EQ(trigger.GetNUserInfoFields(), 4,
                          "Expected one User Info field per station");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[10].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the MU-RTS to occupy the entire channel width");
    for (const auto &userInfo : trigger) {
      NS_TEST_EXPECT_MSG_EQ(+userInfo.GetMuRtsRuAllocation(),
                            +m_muRtsRuAllocation,
                            "Unexpected RU Allocation value in MU-RTS");
    }

    tEnd = m_txPsdus[10].endTx;
    navEnd = tEnd + m_txPsdus[10].psduMap[SU_STA_ID]->GetDuration();

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[11].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[11].psduMap.size() == 1 &&
         m_txPsdus[11].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[11].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[11].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[11].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[11].endTx + m_txPsdus[11].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[12].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[12].psduMap.size() == 1 &&
         m_txPsdus[12].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[12].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[12].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[12].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[12].endTx + m_txPsdus[12].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[13].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[13].psduMap.size() == 1 &&
         m_txPsdus[13].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[13].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[13].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[13].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[13].endTx + m_txPsdus[13].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[14].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[14].psduMap.size() == 1 &&
         m_txPsdus[14].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[14].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[14].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[14].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[14].endTx + m_txPsdus[14].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    tEnd = m_txPsdus[14].endTx;
  } else {
    m_txPsdus.insert(std::next(m_txPsdus.begin(), 10), 5, {});
    tEnd = m_txPsdus[9].endTx;
  }

  NS_TEST_ASSERT_MSG_GT_OR_EQ(m_txPsdus.size(), 21,
                              "Expected at least 21 transmitted packets");
  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[15].psduMap.size() == 1 &&
       m_txPsdus[15].psduMap[SU_STA_ID]->GetHeader(0).IsTrigger() &&
       m_txPsdus[15].psduMap[SU_STA_ID]->GetHeader(0).GetAddr1().IsBroadcast()),
      true, "Expected a Trigger Frame");
  m_txPsdus[15].psduMap[SU_STA_ID]->GetPayload(0)->PeekHeader(trigger);
  NS_TEST_EXPECT_MSG_EQ(trigger.IsBasic(), true,
                        "Expected a Basic Trigger Frame");
  NS_TEST_EXPECT_MSG_EQ(trigger.GetNUserInfoFields(), 4,
                        "Expected one User Info field per station");
  tStart = m_txPsdus[15].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "Basic Trigger Frame sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Basic Trigger Frame sent too late");
  Time basicNavEnd =
      m_txPsdus[15].endTx + m_txPsdus[15].psduMap[SU_STA_ID]->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, basicNavEnd,
                              "Duration/ID in Basic TF is too short");
  NS_TEST_EXPECT_MSG_LT(basicNavEnd, navEnd + tolerance,
                        "Duration/ID in Basic TF is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[16].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[16].psduMap.size() == 1 &&
       m_txPsdus[16].psduMap.begin()->second->GetNMpdus() == 2 &&
       m_txPsdus[16].psduMap.begin()->second->GetHeader(0).IsQosData() &&
       m_txPsdus[16].psduMap.begin()->second->GetHeader(1).IsQosData()),
      true, "Expected 2 QoS data frames in an HE TB PPDU");
  tEnd = m_txPsdus[15].endTx;
  tStart = m_txPsdus[16].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS data frames in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS data frames in HE TB PPDU sent too late");
  Time qosDataNavEnd = m_txPsdus[16].endTx +
                       m_txPsdus[16].psduMap.begin()->second->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosDataNavEnd,
                              "Duration/ID in QoS Data is too short");
  NS_TEST_EXPECT_MSG_LT(qosDataNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Data is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[17].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[17].psduMap.size() == 1 &&
       m_txPsdus[17].psduMap.begin()->second->GetNMpdus() == 2 &&
       m_txPsdus[17].psduMap.begin()->second->GetHeader(0).IsQosData() &&
       m_txPsdus[17].psduMap.begin()->second->GetHeader(1).IsQosData()),
      true, "Expected 2 QoS data frames in an HE TB PPDU");
  tStart = m_txPsdus[17].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS data frames in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS data frames in HE TB PPDU sent too late");
  qosDataNavEnd = m_txPsdus[17].endTx +
                  m_txPsdus[17].psduMap.begin()->second->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosDataNavEnd,
                              "Duration/ID in QoS Data is too short");
  NS_TEST_EXPECT_MSG_LT(qosDataNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Data is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[18].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[18].psduMap.size() == 1 &&
       m_txPsdus[18].psduMap.begin()->second->GetNMpdus() == 2 &&
       m_txPsdus[18].psduMap.begin()->second->GetHeader(0).IsQosData() &&
       m_txPsdus[18].psduMap.begin()->second->GetHeader(1).IsQosData()),
      true, "Expected 2 QoS data frames in an HE TB PPDU");
  tStart = m_txPsdus[18].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS data frames in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS data frames in HE TB PPDU sent too late");
  qosDataNavEnd = m_txPsdus[18].endTx +
                  m_txPsdus[18].psduMap.begin()->second->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosDataNavEnd,
                              "Duration/ID in QoS Data is too short");
  NS_TEST_EXPECT_MSG_LT(qosDataNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Data is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[19].txVector.GetPreambleType() == m_tbPreamble &&
       m_txPsdus[19].psduMap.size() == 1 &&
       m_txPsdus[19].psduMap.begin()->second->GetNMpdus() == 2 &&
       m_txPsdus[19].psduMap.begin()->second->GetHeader(0).IsQosData() &&
       m_txPsdus[19].psduMap.begin()->second->GetHeader(1).IsQosData()),
      true, "Expected 2 QoS data frames in an HE TB PPDU");
  tStart = m_txPsdus[19].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "QoS data frames in HE TB PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "QoS data frames in HE TB PPDU sent too late");
  qosDataNavEnd = m_txPsdus[19].endTx +
                  m_txPsdus[19].psduMap.begin()->second->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, qosDataNavEnd,
                              "Duration/ID in QoS Data is too short");
  NS_TEST_EXPECT_MSG_LT(qosDataNavEnd, navEnd + tolerance,
                        "Duration/ID in QoS Data is too long");

  NS_TEST_EXPECT_MSG_EQ(
      (m_txPsdus[20].psduMap.size() == 1 &&
       m_txPsdus[20].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAck() &&
       m_txPsdus[20].psduMap[SU_STA_ID]->GetHeader(0).GetAddr1().IsBroadcast()),
      true, "Expected a Block Ack");
  m_txPsdus[20].psduMap[SU_STA_ID]->GetPayload(0)->PeekHeader(blockAck);
  NS_TEST_EXPECT_MSG_EQ(blockAck.IsMultiSta(), true,
                        "Expected a Multi-STA Block Ack");
  NS_TEST_EXPECT_MSG_EQ(blockAck.GetNPerAidTidInfoSubfields(), 4,
                        "Expected one Per AID TID Info subfield per station");
  for (uint8_t i = 0; i < 4; i++) {
    NS_TEST_EXPECT_MSG_EQ(blockAck.GetAckType(i), true,
                          "Expected All-ack context");
    NS_TEST_EXPECT_MSG_EQ(+blockAck.GetTidInfo(i), 14,
                          "Expected All-ack context");
  }
  tEnd = m_txPsdus[19].endTx;
  tStart = m_txPsdus[20].startTx;
  NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                        "Multi-STA Block Ack sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "Multi-STA Block Ack sent too late");
  auto multiStaBaNavEnd =
      m_txPsdus[20].endTx + m_txPsdus[20].psduMap[SU_STA_ID]->GetDuration();
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, multiStaBaNavEnd,
                              "Duration/ID in Multi-STA BlockAck is too short");
  NS_TEST_EXPECT_MSG_LT(multiStaBaNavEnd, navEnd + tolerance,
                        "Duration/ID in Multi-STA BlockAck is too long");

  if (m_txopLimit == 0) {
    NS_TEST_ASSERT_MSG_GT_OR_EQ(m_txPsdus.size(), 26,
                                "Expected at least 26 transmitted packet");
    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[21].psduMap.size() == 1 &&
         m_txPsdus[21].psduMap[SU_STA_ID]->GetHeader(0).IsTrigger() &&
         m_txPsdus[21]
             .psduMap[SU_STA_ID]
             ->GetHeader(0)
             .GetAddr1()
             .IsBroadcast()),
        true, "Expected a Trigger Frame");
    m_txPsdus[21].psduMap[SU_STA_ID]->GetPayload(0)->PeekHeader(trigger);
    NS_TEST_EXPECT_MSG_EQ(trigger.IsMuRts(), true,
                          "Expected an MU-RTS Trigger Frame");
    NS_TEST_EXPECT_MSG_EQ(trigger.GetNUserInfoFields(), 4,
                          "Expected one User Info field per station");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[21].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the MU-RTS to occupy the entire channel width");
    for (const auto &userInfo : trigger) {
      NS_TEST_EXPECT_MSG_EQ(+userInfo.GetMuRtsRuAllocation(),
                            +m_muRtsRuAllocation,
                            "Unexpected RU Allocation value in MU-RTS");
    }
    tEnd = m_txPsdus[20].endTx;
    tStart = m_txPsdus[21].startTx;
    NS_TEST_EXPECT_MSG_LT_OR_EQ(tEnd + ifs, tStart,
                                "MU-RTS Trigger Frame sent too early");
    tEnd = m_txPsdus[21].endTx;
    navEnd = tEnd + m_txPsdus[21].psduMap[SU_STA_ID]->GetDuration();

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[22].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[22].psduMap.size() == 1 &&
         m_txPsdus[22].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[22].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[22].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[22].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[22].endTx + m_txPsdus[22].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[23].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[23].psduMap.size() == 1 &&
         m_txPsdus[23].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[23].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[23].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[23].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[23].endTx + m_txPsdus[23].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[24].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[24].psduMap.size() == 1 &&
         m_txPsdus[24].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[24].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[24].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[24].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[24].endTx + m_txPsdus[24].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[25].txVector.GetPreambleType() != WIFI_PREAMBLE_HE_TB &&
         m_txPsdus[25].psduMap.size() == 1 &&
         m_txPsdus[25].psduMap.begin()->second->GetNMpdus() == 1 &&
         m_txPsdus[25].psduMap.begin()->second->GetHeader(0).GetType() ==
             WIFI_MAC_CTL_CTS),
        true, "Expected a CTS frame");
    NS_TEST_EXPECT_MSG_EQ(
        m_txPsdus[25].txVector.GetChannelWidth(), m_channelWidth,
        "Expected the CTS to occupy the entire channel width");

    tStart = m_txPsdus[25].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart, "CTS frame sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "CTS frame sent too late");
    ctsNavEnd =
        m_txPsdus[25].endTx + m_txPsdus[25].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, ctsNavEnd,
                                "Duration/ID in CTS frame is too short");
    NS_TEST_EXPECT_MSG_LT(ctsNavEnd, navEnd + tolerance,
                          "Duration/ID in CTS frame is too long");

    tEnd = m_txPsdus[25].endTx;
  } else {
    m_txPsdus.insert(std::next(m_txPsdus.begin(), 21), 5, {});
    tEnd = m_txPsdus[20].endTx;
  }

  NS_TEST_ASSERT_MSG_GT_OR_EQ(m_txPsdus.size(), 27,
                              "Expected at least 27 transmitted packet");
  NS_TEST_EXPECT_MSG_EQ(m_txPsdus[26].txVector.GetPreambleType(),
                        m_dlMuPreamble, "Expected a DL MU PPDU");
  NS_TEST_EXPECT_MSG_EQ(m_txPsdus[26].psduMap.size(), 4,
                        "Expected 4 PSDUs within the DL MU PPDU");
  NS_TEST_EXPECT_MSG_LT_OR_EQ(
      m_txPsdus[26].endTx - m_txPsdus[26].startTx,
      GetPpduMaxTime(m_txPsdus[26].txVector.GetPreambleType()),
      "TX duration cannot exceed max PPDU duration");
  for (auto &psdu : m_txPsdus[26].psduMap) {
    NS_TEST_EXPECT_MSG_LT_OR_EQ(psdu.second->GetSize(), m_maxAmpduSize,
                                "Max A-MPDU size exceeded");
  }
  tStart = m_txPsdus[26].startTx;
  NS_TEST_EXPECT_MSG_LT_OR_EQ(tEnd + sifs, tStart, "DL MU PPDU sent too early");
  NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                        "DL MU PPDU sent too late");

  auto dlMuNavEnd = m_txPsdus[26].endTx;
  for (auto &psdu : m_txPsdus[26].psduMap) {
    if (dlMuNavEnd == m_txPsdus[26].endTx) {
      dlMuNavEnd += psdu.second->GetDuration();
    } else {
      NS_TEST_EXPECT_MSG_EQ(m_txPsdus[26].endTx + psdu.second->GetDuration(),
                            dlMuNavEnd,
                            "Duration/ID must be the same for all PSDUs");
    }
  }
  NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, dlMuNavEnd,
                              "Duration/ID in DL MU PPDU is too short");
  NS_TEST_EXPECT_MSG_LT(dlMuNavEnd, navEnd + tolerance,
                        "Duration/ID in DL MU PPDU is too long");

  std::size_t nTxPsdus = 0;

  if (m_dlMuAckType == WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE) {
    NS_TEST_EXPECT_MSG_GT_OR_EQ(m_txPsdus.size(), 34,
                                "Expected at least 34 packets");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[27].psduMap.size() == 1 &&
         m_txPsdus[27].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tEnd = m_txPsdus[26].endTx;
    tStart = m_txPsdus[27].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "First Block Ack sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "First Block Ack sent too late");
    Time baNavEnd =
        m_txPsdus[27].endTx + m_txPsdus[27].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(
        navEnd, baNavEnd, "Duration/ID in 1st BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in 1st BlockAck is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[28].psduMap.size() == 1 &&
         m_txPsdus[28].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAckReq()),
        true, "Expected a Block Ack Request");
    tEnd = m_txPsdus[27].endTx;
    tStart = m_txPsdus[28].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "First Block Ack Request sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "First Block Ack Request sent too late");
    Time barNavEnd =
        m_txPsdus[28].endTx + m_txPsdus[28].psduMap[SU_STA_ID]->GetDuration();
    if (m_txopLimit > 0) {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, barNavEnd,
                                  "Duration/ID in BlockAckReq is too short");
      NS_TEST_EXPECT_MSG_LT(barNavEnd, navEnd + tolerance,
                            "Duration/ID in BlockAckReq is too long");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[29].psduMap.size() == 1 &&
         m_txPsdus[29].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tEnd = m_txPsdus[28].endTx;
    tStart = m_txPsdus[29].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Second Block Ack sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Second Block Ack sent too late");
    baNavEnd =
        m_txPsdus[29].endTx + m_txPsdus[29].psduMap[SU_STA_ID]->GetDuration();
    if (m_txopLimit > 0) {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                  "Duration/ID in BlockAck is too short");
      NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                            "Duration/ID in BlockAck is too long");
    } else {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(barNavEnd, baNavEnd,
                                  "Duration/ID in BlockAck is too short");
      NS_TEST_EXPECT_MSG_LT(baNavEnd, barNavEnd + tolerance,
                            "Duration/ID in BlockAck is too long");
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[29].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[30].psduMap.size() == 1 &&
         m_txPsdus[30].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAckReq()),
        true, "Expected a Block Ack Request");
    tEnd = m_txPsdus[29].endTx;
    tStart = m_txPsdus[30].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Second Block Ack Request sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Second Block Ack Request sent too late");
    barNavEnd =
        m_txPsdus[30].endTx + m_txPsdus[30].psduMap[SU_STA_ID]->GetDuration();
    if (m_txopLimit > 0) {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, barNavEnd,
                                  "Duration/ID in BlockAckReq is too short");
      NS_TEST_EXPECT_MSG_LT(barNavEnd, navEnd + tolerance,
                            "Duration/ID in BlockAckReq is too long");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[31].psduMap.size() == 1 &&
         m_txPsdus[31].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tEnd = m_txPsdus[30].endTx;
    tStart = m_txPsdus[31].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Third Block Ack sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Third Block Ack sent too late");
    baNavEnd =
        m_txPsdus[31].endTx + m_txPsdus[31].psduMap[SU_STA_ID]->GetDuration();
    if (m_txopLimit > 0) {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                  "Duration/ID in BlockAck is too short");
      NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                            "Duration/ID in BlockAck is too long");
    } else {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(barNavEnd, baNavEnd,
                                  "Duration/ID in BlockAck is too short");
      NS_TEST_EXPECT_MSG_LT(baNavEnd, barNavEnd + tolerance,
                            "Duration/ID in BlockAck is too long");
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[31].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[32].psduMap.size() == 1 &&
         m_txPsdus[32].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAckReq()),
        true, "Expected a Block Ack Request");
    tEnd = m_txPsdus[31].endTx;
    tStart = m_txPsdus[32].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Third Block Ack Request sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Third Block Ack Request sent too late");
    barNavEnd =
        m_txPsdus[32].endTx + m_txPsdus[32].psduMap[SU_STA_ID]->GetDuration();
    if (m_txopLimit > 0) {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, barNavEnd,
                                  "Duration/ID in BlockAckReq is too short");
      NS_TEST_EXPECT_MSG_LT(barNavEnd, navEnd + tolerance,
                            "Duration/ID in BlockAckReq is too long");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[33].psduMap.size() == 1 &&
         m_txPsdus[33].psduMap[SU_STA_ID]->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tEnd = m_txPsdus[32].endTx;
    tStart = m_txPsdus[33].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Fourth Block Ack sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Fourth Block Ack sent too late");
    baNavEnd =
        m_txPsdus[33].endTx + m_txPsdus[33].psduMap[SU_STA_ID]->GetDuration();
    if (m_txopLimit > 0) {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                  "Duration/ID in BlockAck is too short");
      NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                            "Duration/ID in BlockAck is too long");
    } else {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(barNavEnd, baNavEnd,
                                  "Duration/ID in BlockAck is too short");
      NS_TEST_EXPECT_MSG_LT(baNavEnd, barNavEnd + tolerance,
                            "Duration/ID in BlockAck is too long");
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[33].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    nTxPsdus = 34;
  } else if (m_dlMuAckType == WifiAcknowledgment::DL_MU_TF_MU_BAR) {
    NS_TEST_EXPECT_MSG_GT_OR_EQ(m_txPsdus.size(), 32,
                                "Expected at least 32 packets");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[27].psduMap.size() == 1 &&
         m_txPsdus[27].psduMap[SU_STA_ID]->GetHeader(0).IsTrigger()),
        true, "Expected a MU-BAR Trigger Frame");
    tEnd = m_txPsdus[26].endTx;
    tStart = m_txPsdus[27].startTx;
    NS_TEST_EXPECT_MSG_EQ(tStart, tEnd + sifs,
                          "MU-BAR Trigger Frame sent at wrong time");
    auto muBarNavEnd =
        m_txPsdus[27].endTx + m_txPsdus[27].psduMap[SU_STA_ID]->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(
        navEnd, muBarNavEnd,
        "Duration/ID in MU-BAR Trigger Frame is too short");
    NS_TEST_EXPECT_MSG_LT(muBarNavEnd, navEnd + tolerance,
                          "Duration/ID in MU-BAR Trigger Frame is too long");

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[28].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[28].psduMap.size() == 1 &&
         m_txPsdus[28].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tEnd = m_txPsdus[27].endTx;
    tStart = m_txPsdus[28].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    Time baNavEnd = m_txPsdus[28].endTx +
                    m_txPsdus[28].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[28].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[29].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[29].psduMap.size() == 1 &&
         m_txPsdus[29].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tStart = m_txPsdus[29].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    baNavEnd = m_txPsdus[29].endTx +
               m_txPsdus[29].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in 1st BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[29].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[30].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[30].psduMap.size() == 1 &&
         m_txPsdus[30].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tStart = m_txPsdus[30].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    baNavEnd = m_txPsdus[30].endTx +
               m_txPsdus[30].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in 1st BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[30].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[31].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[31].psduMap.size() == 1 &&
         m_txPsdus[31].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tStart = m_txPsdus[31].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    baNavEnd = m_txPsdus[31].endTx +
               m_txPsdus[31].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in 1st BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[31].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    nTxPsdus = 32;
  } else if (m_dlMuAckType == WifiAcknowledgment::DL_MU_AGGREGATE_TF) {
    NS_TEST_ASSERT_MSG_GT_OR_EQ(m_txPsdus.size(), 31,
                                "Expected at least 31 packets");

    for (auto &psdu : m_txPsdus[26].psduMap) {
      NS_TEST_EXPECT_MSG_EQ(
          (*std::prev(psdu.second->end()))->GetHeader().IsTrigger(), true,
          "Expected an aggregated MU-BAR Trigger Frame");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[27].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[27].psduMap.size() == 1 &&
         m_txPsdus[27].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tEnd = m_txPsdus[26].endTx;
    tStart = m_txPsdus[27].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    Time baNavEnd = m_txPsdus[27].endTx +
                    m_txPsdus[27].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[27].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[28].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[28].psduMap.size() == 1 &&
         m_txPsdus[28].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tStart = m_txPsdus[28].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    baNavEnd = m_txPsdus[28].endTx +
               m_txPsdus[28].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[28].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[29].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[29].psduMap.size() == 1 &&
         m_txPsdus[29].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tStart = m_txPsdus[29].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    baNavEnd = m_txPsdus[29].endTx +
               m_txPsdus[29].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[29].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    NS_TEST_EXPECT_MSG_EQ(
        (m_txPsdus[30].txVector.GetPreambleType() == m_tbPreamble &&
         m_txPsdus[30].psduMap.size() == 1 &&
         m_txPsdus[30].psduMap.begin()->second->GetHeader(0).IsBlockAck()),
        true, "Expected a Block Ack");
    tStart = m_txPsdus[30].startTx;
    NS_TEST_EXPECT_MSG_LT(tEnd + sifs, tStart,
                          "Block Ack in HE TB PPDU sent too early");
    NS_TEST_EXPECT_MSG_LT(tStart, tEnd + sifs + tolerance,
                          "Block Ack in HE TB PPDU sent too late");
    baNavEnd = m_txPsdus[30].endTx +
               m_txPsdus[30].psduMap.begin()->second->GetDuration();
    NS_TEST_EXPECT_MSG_LT_OR_EQ(navEnd, baNavEnd,
                                "Duration/ID in BlockAck frame is too short");
    NS_TEST_EXPECT_MSG_LT(baNavEnd, navEnd + tolerance,
                          "Duration/ID in BlockAck is too long");
    if (m_txopLimit == 0) {
      NS_TEST_EXPECT_MSG_EQ(baNavEnd, m_txPsdus[30].endTx,
                            "Expected null Duration/ID for BlockAck");
    }

    nTxPsdus = 31;
  }

  NS_TEST_EXPECT_MSG_EQ(m_received, m_nPktsPerSta * m_nStations - m_flushed,
                        "Not all DL packets have been received");

  if (m_muEdcaParameterSet.muTimer > 0 && m_muEdcaParameterSet.muAifsn == 0) {
    for (std::size_t i = nTxPsdus; i < m_txPsdus.size(); ++i) {
      if (m_txPsdus[i].psduMap.size() == 1 &&
          !m_txPsdus[i].psduMap.begin()->second->GetHeader(0).IsCts() &&
          m_txPsdus[i].psduMap.begin()->second->GetHeader(0).GetAddr2() !=
              m_apDevice->GetAddress() &&
          !m_txPsdus[i].txVector.IsUlMu()) {
        NS_TEST_EXPECT_MSG_GT_OR_EQ(
            m_txPsdus[i].startTx.GetMicroSeconds(),
            m_edcaDisabledStartTime.GetMicroSeconds() +
                m_muEdcaParameterSet.muTimer * m_muTimerRes,
            "A station transmitted before the MU EDCA timer expired");
        break;
      }
    }
  } else if (m_muEdcaParameterSet.muTimer > 0 &&
             m_muEdcaParameterSet.muAifsn > 0) {
    for (const auto &cwValue : m_cwValues) {
      NS_TEST_EXPECT_MSG_EQ(
          (cwValue == 2 || cwValue >= m_muEdcaParameterSet.muCwMin), true,
          "A station did not set the correct MU CW min");
    }
  }

  m_txPsdus.clear();
}

void OfdmaAckSequenceTest::DoRun() {
  uint32_t previousSeed = RngSeedManager::GetSeed();
  uint64_t previousRun = RngSeedManager::GetRun();
  Config::SetGlobal("RngSeed", UintegerValue(2));
  Config::SetGlobal("RngRun", UintegerValue(2));
  int64_t streamNumber = 10;

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  NodeContainer wifiOldStaNodes;
  NodeContainer wifiNewStaNodes;
  wifiOldStaNodes.Create(m_nStations / 2);
  wifiNewStaNodes.Create(m_nStations - m_nStations / 2);
  NodeContainer wifiStaNodes(wifiOldStaNodes, wifiNewStaNodes);

  Ptr<MultiModelSpectrumChannel> spectrumChannel =
      CreateObject<MultiModelSpectrumChannel>();
  Ptr<FriisPropagationLossModel> lossModel =
      CreateObject<FriisPropagationLossModel>();
  spectrumChannel->AddPropagationLossModel(lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  spectrumChannel->SetPropagationDelayModel(delayModel);

  SpectrumWifiPhyHelper phy;
  phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetErrorRateModel("ns3::NistErrorRateModel");
  phy.SetChannel(spectrumChannel);
  switch (m_channelWidth) {
  case 20:
    phy.Set("ChannelSettings", StringValue("{36, 20, BAND_5GHZ, 0}"));
    break;
  case 40:
    phy.Set("ChannelSettings", StringValue("{38, 40, BAND_5GHZ, 0}"));
    break;
  case 80:
    phy.Set("ChannelSettings", StringValue("{42, 80, BAND_5GHZ, 0}"));
    break;
  case 160:
    phy.Set("ChannelSettings", StringValue("{50, 160, BAND_5GHZ, 0}"));
    break;
  default:
    NS_ABORT_MSG("Invalid channel bandwidth (must be 20, 40, 80 or 160)");
  }

  Config::SetDefault("ns3::WifiDefaultProtectionManager::EnableMuRts",
                     BooleanValue(true));
  Config::SetDefault("ns3::HeConfiguration::MuBeAifsn",
                     UintegerValue(m_muEdcaParameterSet.muAifsn));
  Config::SetDefault("ns3::HeConfiguration::MuBeCwMin",
                     UintegerValue(m_muEdcaParameterSet.muCwMin));
  Config::SetDefault("ns3::HeConfiguration::MuBeCwMax",
                     UintegerValue(m_muEdcaParameterSet.muCwMax));
  Config::SetDefault(
      "ns3::HeConfiguration::BeMuEdcaTimer",
      TimeValue(MicroSeconds(8192 * m_muEdcaParameterSet.muTimer)));

  Config::SetDefault("ns3::HeConfiguration::MuBkAifsn",
                     UintegerValue(m_muEdcaParameterSet.muAifsn));
  Config::SetDefault("ns3::HeConfiguration::MuBkCwMin",
                     UintegerValue(m_muEdcaParameterSet.muCwMin));
  Config::SetDefault("ns3::HeConfiguration::MuBkCwMax",
                     UintegerValue(m_muEdcaParameterSet.muCwMax));
  Config::SetDefault(
      "ns3::HeConfiguration::BkMuEdcaTimer",
      TimeValue(MicroSeconds(8192 * m_muEdcaParameterSet.muTimer)));

  Config::SetDefault("ns3::HeConfiguration::MuViAifsn",
                     UintegerValue(m_muEdcaParameterSet.muAifsn));
  Config::SetDefault("ns3::HeConfiguration::MuViCwMin",
                     UintegerValue(m_muEdcaParameterSet.muCwMin));
  Config::SetDefault("ns3::HeConfiguration::MuViCwMax",
                     UintegerValue(m_muEdcaParameterSet.muCwMax));
  Config::SetDefault(
      "ns3::HeConfiguration::ViMuEdcaTimer",
      TimeValue(MicroSeconds(8192 * m_muEdcaParameterSet.muTimer)));

  Config::SetDefault("ns3::HeConfiguration::MuVoAifsn",
                     UintegerValue(m_muEdcaParameterSet.muAifsn));
  Config::SetDefault("ns3::HeConfiguration::MuVoCwMin",
                     UintegerValue(m_muEdcaParameterSet.muCwMin));
  Config::SetDefault("ns3::HeConfiguration::MuVoCwMax",
                     UintegerValue(m_muEdcaParameterSet.muCwMax));
  Config::SetDefault(
      "ns3::HeConfiguration::VoMuEdcaTimer",
      TimeValue(MicroSeconds(8192 * m_muEdcaParameterSet.muTimer)));

  Config::SetDefault("ns3::WifiMacQueue::MaxDelay", TimeValue(Seconds(2)));

  WifiHelper wifi;
  wifi.SetStandard(m_scenario == WifiOfdmaScenario::EHT
                       ? WIFI_STANDARD_80211be
                       : WIFI_STANDARD_80211ax);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("HeMcs11"));
  wifi.ConfigHeOptions(
      "MuBeAifsn", UintegerValue(m_muEdcaParameterSet.muAifsn), "MuBeCwMin",
      UintegerValue(m_muEdcaParameterSet.muCwMin), "MuBeCwMax",
      UintegerValue(m_muEdcaParameterSet.muCwMax), "BeMuEdcaTimer",
      TimeValue(MicroSeconds(m_muTimerRes * m_muEdcaParameterSet.muTimer)),
      "BkMuEdcaTimer",
      TimeValue(MicroSeconds(m_muTimerRes * m_muEdcaParameterSet.muTimer)),
      "ViMuEdcaTimer",
      TimeValue(MicroSeconds(m_muTimerRes * m_muEdcaParameterSet.muTimer)),
      "VoMuEdcaTimer",
      TimeValue(MicroSeconds(m_muTimerRes * m_muEdcaParameterSet.muTimer)));

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType(
      "ns3::StaWifiMac", "Ssid", SsidValue(ssid), "BE_MaxAmsduSize",
      UintegerValue(0), "BE_MaxAmpduSize", UintegerValue(m_maxAmpduSize),
      "BE_BlockAckThreshold", UintegerValue(2), "BK_MaxAmsduSize",
      UintegerValue(0), "BK_MaxAmpduSize", UintegerValue(m_maxAmpduSize),
      "BK_BlockAckThreshold", UintegerValue(2), "VI_MaxAmsduSize",
      UintegerValue(0), "VI_MaxAmpduSize", UintegerValue(m_maxAmpduSize),
      "VI_BlockAckThreshold", UintegerValue(2), "VO_MaxAmsduSize",
      UintegerValue(0), "VO_MaxAmpduSize", UintegerValue(m_maxAmpduSize),
      "VO_BlockAckThreshold", UintegerValue(2), "ActiveProbing",
      BooleanValue(false));

  m_staDevices = wifi.Install(phy, mac, wifiOldStaNodes);

  wifi.SetStandard(m_scenario == WifiOfdmaScenario::HE ? WIFI_STANDARD_80211ax
                                                       : WIFI_STANDARD_80211be);
  m_staDevices =
      NetDeviceContainer(m_staDevices, wifi.Install(phy, mac, wifiNewStaNodes));

  wifi.SetStandard(WIFI_STANDARD_80211ac);
  wifi.Install(phy, mac, Create<Node>());

  wifi.SetStandard(m_scenario == WifiOfdmaScenario::HE ? WIFI_STANDARD_80211ax
                                                       : WIFI_STANDARD_80211be);

  mac.SetType("ns3::ApWifiMac", "BeaconGeneration", BooleanValue(true));
  mac.SetMultiUserScheduler("ns3::TestMultiUserScheduler", "ModulationClass",
                            EnumValue(m_scenario == WifiOfdmaScenario::EHT
                                          ? WIFI_MOD_CLASS_EHT
                                          : WIFI_MOD_CLASS_HE),
                            "AccessReqInterval", TimeValue(Seconds(1.5)),
                            "DelayAccessReqUponAccess", BooleanValue(false));
  mac.SetAckManager("ns3::WifiDefaultAckManager", "DlMuAckSequenceType",
                    EnumValue(m_dlMuAckType));

  m_apDevice =
      DynamicCast<WifiNetDevice>(wifi.Install(phy, mac, wifiApNode).Get(0));

  streamNumber +=
      wifi.AssignStreams(NetDeviceContainer(m_apDevice), streamNumber);
  streamNumber += wifi.AssignStreams(m_staDevices, streamNumber);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, 1.0, 0.0));
  positionAlloc->Add(Vector(-1.0, 0.0, 0.0));
  positionAlloc->Add(Vector(-1.0, -1.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNodes);

  NetDeviceContainer allDevices(NetDeviceContainer(m_apDevice), m_staDevices);
  for (uint32_t i = 0; i < allDevices.GetN(); i++) {
    auto dev = DynamicCast<WifiNetDevice>(allDevices.Get(i));
    dev->GetMac()->GetQosTxop(AC_BE)->SetTxopLimit(MicroSeconds(m_txopLimit));
    dev->GetMac()->GetQosTxop(AC_BK)->SetTxopLimit(MicroSeconds(m_txopLimit));
    dev->GetMac()->GetQosTxop(AC_VI)->SetTxopLimit(MicroSeconds(m_txopLimit));
    dev->GetMac()->GetQosTxop(AC_VO)->SetTxopLimit(MicroSeconds(m_txopLimit));
    dev->GetMac()->GetQosTxop(AC_BE)->SetAifsn(3);
    dev->GetMac()->GetQosTxop(AC_BK)->SetAifsn(3);
    dev->GetMac()->GetQosTxop(AC_VI)->SetAifsn(3);
    dev->GetMac()->GetQosTxop(AC_VO)->SetAifsn(3);
  }

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiApNode);
  packetSocket.Install(wifiStaNodes);

  for (uint16_t i = 0; i < m_nStations; i++) {
    PacketSocketAddress socket;
    socket.SetSingleDevice(m_apDevice->GetIfIndex());
    socket.SetPhysicalAddress(m_staDevices.Get(i)->GetAddress());
    socket.SetProtocol(1);

    Ptr<PacketSocketClient> client1 = CreateObject<PacketSocketClient>();
    client1->SetAttribute("PacketSize", UintegerValue(1400));
    client1->SetAttribute("MaxPackets", UintegerValue(2));
    client1->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    client1->SetAttribute("Priority", UintegerValue(i * 2));
    client1->SetRemote(socket);
    wifiApNode.Get(0)->AddApplication(client1);
    client1->SetStartTime(Seconds(1) + i * MilliSeconds(1));
    client1->SetStopTime(Seconds(2.0));

    Ptr<PacketSocketClient> client2 = CreateObject<PacketSocketClient>();
    client2->SetAttribute("PacketSize", UintegerValue(1400 + i * 100));
    client2->SetAttribute("MaxPackets", UintegerValue(m_nPktsPerSta));
    client2->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    client2->SetAttribute("Priority", UintegerValue(i * 2));
    client2->SetRemote(socket);
    wifiApNode.Get(0)->AddApplication(client2);
    client2->SetStartTime(Seconds(1.5003));
    client2->SetStopTime(Seconds(2.5));

    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(socket);
    wifiStaNodes.Get(i)->AddApplication(server);
    server->SetStartTime(Seconds(0.0));
    server->SetStopTime(Seconds(3.0));
  }

  for (uint16_t i = 0; i < m_nStations; i++) {
    m_sockets[i].SetSingleDevice(m_staDevices.Get(i)->GetIfIndex());
    m_sockets[i].SetPhysicalAddress(m_apDevice->GetAddress());
    m_sockets[i].SetProtocol(1);

    Ptr<PacketSocketClient> client1 = CreateObject<PacketSocketClient>();
    client1->SetAttribute("PacketSize", UintegerValue(1400));
    client1->SetAttribute("MaxPackets", UintegerValue(2));
    client1->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
    client1->SetAttribute("Priority", UintegerValue(i * 2));
    client1->SetRemote(m_sockets[i]);
    wifiStaNodes.Get(i)->AddApplication(client1);
    client1->SetStartTime(Seconds(1.005) + i * MilliSeconds(1));
    client1->SetStopTime(Seconds(2.0));

    Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
    server->SetLocal(m_sockets[i]);
    wifiApNode.Get(0)->AddApplication(server);
    server->SetStartTime(Seconds(0.0));
    server->SetStopTime(Seconds(3.0));
  }

  Config::Connect("/NodeList/*/ApplicationList/0/$ns3::PacketSocketServer/Rx",
                  MakeCallback(&OfdmaAckSequenceTest::L7Receive, this));
  Config::Connect(
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
      MakeCallback(&OfdmaAckSequenceTest::Transmit, this));

  Simulator::Stop(Seconds(3));
  Simulator::Run();

  CheckResults(m_apDevice->GetMac()->GetWifiPhy()->GetSifs(),
               m_apDevice->GetMac()->GetWifiPhy()->GetSlot(),
               m_apDevice->GetMac()->GetQosTxop(AC_BE)->Txop::GetAifsn());

  Simulator::Destroy();

  Config::SetGlobal("RngSeed", UintegerValue(previousSeed));
  Config::SetGlobal("RngRun", UintegerValue(previousRun));
}

class WifiMacOfdmaTestSuite : public TestSuite {
public:
  WifiMacOfdmaTestSuite();
};

WifiMacOfdmaTestSuite::WifiMacOfdmaTestSuite()
    : TestSuite("wifi-mac-ofdma", UNIT) {
  using MuEdcaParams =
      std::initializer_list<OfdmaAckSequenceTest::MuEdcaParameterSet>;

  for (auto &muEdcaParameterSet :
       MuEdcaParams{{0, 0, 0, 0}, {0, 127, 2047, 100}, {10, 127, 2047, 100}}) {
    for (const auto scenario :
         {WifiOfdmaScenario::HE, WifiOfdmaScenario::HE_EHT,
          WifiOfdmaScenario::EHT}) {
      AddTestCase(new OfdmaAckSequenceTest(
                      20, WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE, 10000,
                      5632, 15, muEdcaParameterSet, scenario),
                  TestCase::QUICK);
      AddTestCase(new OfdmaAckSequenceTest(
                      20, WifiAcknowledgment::DL_MU_AGGREGATE_TF, 10000, 5632,
                      15, muEdcaParameterSet, scenario),
                  TestCase::QUICK);
      AddTestCase(new OfdmaAckSequenceTest(
                      20, WifiAcknowledgment::DL_MU_TF_MU_BAR, 10000, 5632, 15,
                      muEdcaParameterSet, scenario),
                  TestCase::QUICK);
      AddTestCase(new OfdmaAckSequenceTest(
                      40, WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE, 10000, 0,
                      15, muEdcaParameterSet, scenario),
                  TestCase::QUICK);
      AddTestCase(
          new OfdmaAckSequenceTest(40, WifiAcknowledgment::DL_MU_AGGREGATE_TF,
                                   10000, 0, 15, muEdcaParameterSet, scenario),
          TestCase::QUICK);
      AddTestCase(
          new OfdmaAckSequenceTest(40, WifiAcknowledgment::DL_MU_TF_MU_BAR,
                                   10000, 0, 15, muEdcaParameterSet, scenario),
          TestCase::QUICK);
    }
  }
}

static WifiMacOfdmaTestSuite g_wifiMacOfdmaTestSuite;
