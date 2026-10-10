
#include "ss-link-manager.h"

#include "burst-profile-manager.h"
#include "service-flow-manager.h"

#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"

#include <stdint.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SSLinkManager");

NS_OBJECT_ENSURE_REGISTERED(SSLinkManager);

TypeId SSLinkManager::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::SSLinkManager").SetParent<Object>().SetGroupName("Wimax");
  return tid;
}

SSLinkManager::SSLinkManager(Ptr<SubscriberStationNetDevice> ss)
    : m_ss(ss), m_rangingStatus(WimaxNetDevice::RANGING_STATUS_EXPIRED),
      m_bsEirp(65535), m_eirXPIrMax(65535), m_pTxIrMax(0),
      m_initRangOppNumber(0), m_contentionRangingRetries(0),
      m_rngReqFrameNumber(0), m_dlChnlNr(0), m_frequency(0),
      m_rangingIntervalFound(false), m_nrRngReqsSent(0), m_nrRngRspsRecvd(0),
      m_nrInvitedPollsRecvd(0), m_rangingCW(0), m_rangingBO(0),
      m_nrRangingTransOpps(0), m_isBackoffSet(false), m_rangingAnomalies(0) {}

SSLinkManager::~SSLinkManager() { m_ss = nullptr; }

void SSLinkManager::DoDispose() { m_ss = nullptr; }

void SSLinkManager::SetBsEirp(uint16_t bs_eirp) { m_bsEirp = bs_eirp; }

void SSLinkManager::SetEirXPIrMax(uint16_t eir_x_p_ir_max) {
  m_eirXPIrMax = eir_x_p_ir_max;
}

void SSLinkManager::SetRangingIntervalFound(bool rangingIntervalFound) {
  m_rangingIntervalFound = rangingIntervalFound;
}

bool SSLinkManager::GetRangingIntervalFound() const {
  return m_rangingIntervalFound;
}

void SSLinkManager::SetNrRangingTransOpps(uint8_t nrRangingTransOpps) {
  m_nrRangingTransOpps = nrRangingTransOpps;
}

void SSLinkManager::SetRangingCW(uint8_t rangingCW) { m_rangingCW = rangingCW; }

void SSLinkManager::IncrementNrInvitedPollsRecvd() { m_nrInvitedPollsRecvd++; }

EventId SSLinkManager::GetDlMapSyncTimeoutEvent() {
  return m_dlMapSyncTimeoutEvent;
}

void SSLinkManager::StartScanning(SubscriberStationNetDevice::EventType type,
                                  bool deleteParameters) {

  if (deleteParameters) {
    DeleteUplinkParameters();
  }

  NS_ASSERT_MSG(
      !m_ss->IsRegistered(),
      "Subscriber Station: Error while scanning: Already registered with a BS");

  if (m_ss->GetState() != SubscriberStationNetDevice::SS_STATE_IDLE) {
    m_dlChnlNr++;
  }

  if (m_dlChnlNr >= 200) {
    m_dlChnlNr = 0;
  }

  uint64_t dlChannel = m_ss->GetChannel(m_dlChnlNr);

  m_ss->SetState(SubscriberStationNetDevice::SS_STATE_SCANNING);
  m_ss->GetPhy()->StartScanning(
      dlChannel, m_ss->GetIntervalT20(),
      MakeCallback(&SSLinkManager::EndScanning, this));
}

void SSLinkManager::EndScanning(bool status, uint64_t frequency) {
  if (status) {
    StartSynchronizing();
    m_frequency = frequency;
  } else {
    StartScanning(SubscriberStationNetDevice::EVENT_NONE, false);
  }
}

void SSLinkManager::StartSynchronizing() {
  m_ss->SetState(SubscriberStationNetDevice::SS_STATE_SYNCHRONIZING);
  m_ss->SetTimer(
      Simulator::Schedule(
          m_ss->GetIntervalT21(), &SSLinkManager::StartScanning, this,
          SubscriberStationNetDevice::EVENT_DL_MAP_SYNC_TIMEOUT, false),
      m_dlMapSyncTimeoutEvent);
}

void SSLinkManager::SendRangingRequest(uint8_t uiuc, uint16_t allocationSize) {
  NS_ASSERT_MSG(
      m_ss->GetState() ==
              SubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL ||
          m_ss->GetState() ==
              SubscriberStationNetDevice::SS_STATE_WAITING_INV_RANG_INTRVL,
      "SS: Error while sending a ranging request: the ss state should be "
      "SS_STATE_WAITING_REG_RANG_INTRVL or SS_STATE_WAITING_INV_RANG_INTRVL");

  if (m_nrRngReqsSent == 0) {
    m_pTxIrMax = CalculateMaxIRSignalStrength();
    m_rngreq.SetReqDlBurstProfile(
        m_ss->GetBurstProfileManager()->GetBurstProfileToRequest());
    m_rngreq.SetMacAddress(m_ss->GetMacAddress());
  } else {
    m_pTxIrMax++;
    if (m_nrRngRspsRecvd > 0) {
      m_rngreq.SetRangingAnomalies(m_rangingAnomalies);
    }
  }

  Ptr<Packet> packet = Create<Packet>();
  Ptr<PacketBurst> burst = Create<PacketBurst>();

  packet->AddHeader(m_rngreq);
  packet->AddHeader(
      ManagementMessageType(ManagementMessageType::MESSAGE_TYPE_RNG_REQ));

  Ptr<WimaxConnection> connection;

  if (m_rangingStatus == WimaxNetDevice::RANGING_STATUS_CONTINUE) {
    connection = m_ss->GetBasicConnection();
  } else {
    connection = m_ss->GetInitialRangingConnection();
  }

  m_ss->Enqueue(packet, MacHeaderType(), connection);

  m_ss->SetState(SubscriberStationNetDevice::SS_STATE_WAITING_RNG_RSP);
  m_ss->SetTimer(Simulator::Schedule(m_ss->GetIntervalT3(),
                                     &SSLinkManager::StartContentionResolution,
                                     this),
                 m_waitForRngRspEvent);
  m_nrRngReqsSent++;

  NS_ASSERT_MSG(
      allocationSize ==
          m_ss->GetCurrentUcd().GetChannelEncodings().GetRangReqOppSize() /
              m_ss->GetPhy()->GetPsPerSymbol(),
      "SS: Error while sending a ranging request: the allocation size is not "
      "correct");

  m_ss->SendBurst(uiuc, allocationSize, connection);
}

void SSLinkManager::StartContentionResolution() {
  NS_ASSERT_MSG(
      m_ss->GetState() ==
              SubscriberStationNetDevice::SS_STATE_WAITING_RNG_RSP ||
          m_ss->GetState() ==
              SubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL ||
          m_ss->GetState() ==
              SubscriberStationNetDevice::SS_STATE_ADJUSTING_PARAMETERS,
      "SS: Can not start connection resolution: The SS state should be "
      "SS_STATE_WAITING_RNG_RSP "
      "or SS_STATE_WAITING_REG_RANG_INTRVL or SS_STATE_ADJUSTING_PARAMETERS");

  if (m_ss->GetState() ==
      SubscriberStationNetDevice::SS_STATE_WAITING_RNG_RSP) {
    m_ss->SetState(
        SubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL);
    IncreaseRangingRequestCW();
    m_contentionRangingRetries++;
  } else if (m_ss->GetState() ==
             SubscriberStationNetDevice::SS_STATE_ADJUSTING_PARAMETERS) {
    m_ss->SetState(
        SubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL);
  }

  if (m_contentionRangingRetries == m_ss->GetMaxContentionRangingRetries()) {
    StartScanning(SubscriberStationNetDevice::EVENT_NONE, false);
  } else {
    if (!m_isBackoffSet) {
      SelectRandomBackoff();
    }
  }
}

void SSLinkManager::PerformBackoff() {
  Time deferTime = Seconds(0);
  Time timeToAllocation = Seconds(0);
  uint16_t nrPsPerRangOpp =
      m_ss->GetCurrentUcd().GetChannelEncodings().GetRangReqOppSize();
  uint16_t oppSize =
      m_ss->GetCurrentUcd().GetChannelEncodings().GetRangReqOppSize() /
      m_ss->GetPhy()->GetPsPerSymbol();

  for (uint8_t deferTOs = 0; deferTOs < m_nrRangingTransOpps; deferTOs++) {
    if (m_rangingBO == 0) {
      deferTime = Seconds(deferTOs * nrPsPerRangOpp *
                          m_ss->GetPhy()->GetPsDuration().GetSeconds());
      timeToAllocation = m_ss->GetTimeToAllocation(deferTime);

      Simulator::Schedule(timeToAllocation, &SSLinkManager::SendRangingRequest,
                          this, OfdmUlBurstProfile::UIUC_INITIAL_RANGING,
                          oppSize);

      m_rngReqFrameNumber = m_ss->GetNrFrames();
      m_initRangOppNumber = deferTOs + 1;

      m_isBackoffSet = false;
      break;
    }
    m_rangingBO--;
  }
}

void SSLinkManager::SelectRandomBackoff() {
  NS_ASSERT_MSG(m_rangingCW != 0 && m_rangingBO == 0,
                "be sure that CW has been set and BO is not already set");

  m_rangingBO = (rand() % m_rangingCW);
  m_isBackoffSet = true;
}

void SSLinkManager::IncreaseRangingRequestCW() {
  m_rangingCW = std::min(uint8_t((m_rangingCW * 2 + 1) - 1),
                         m_ss->GetCurrentUcd().GetRangingBackoffEnd());
}

void SSLinkManager::ResetRangingRequestCW() {
  m_rangingCW =
      (uint8_t)std::pow(
          2.0, (double)m_ss->GetCurrentUcd().GetRangingBackoffStart()) -
      1;
}

void SSLinkManager::PerformRanging(Cid cid, RngRsp rngrsp) {

  if (cid == m_ss->GetInitialRangingConnection()->GetCid()) {
    if (rngrsp.GetFrameNumber() == m_rngReqFrameNumber &&
        rngrsp.GetInitRangOppNumber() == m_initRangOppNumber) {
      Simulator::Cancel(m_waitForRngRspEvent);
      m_nrRngRspsRecvd++;

      ResetRangingRequestCW();
      AdjustRangingParameters(rngrsp);
      m_ss->SetState(SubscriberStationNetDevice::SS_STATE_ADJUSTING_PARAMETERS);
      return;
    }

    if (m_ss->GetAddress() != rngrsp.GetMacAddress()) {
      return;
    }

    m_ss->SetBasicConnection(
        CreateObject<WimaxConnection>(rngrsp.GetBasicCid(), Cid::BASIC));

    m_ss->SetPrimaryConnection(
        CreateObject<WimaxConnection>(rngrsp.GetPrimaryCid(), Cid::PRIMARY));
    m_ss->SetAreManagementConnectionsAllocated(true);
  } else {
  }

  m_nrRngRspsRecvd++;
  if (m_waitForRngRspEvent.IsRunning()) {
    Simulator::Cancel(m_waitForRngRspEvent);
  }

  m_rangingStatus = (WimaxNetDevice::RangingStatus)rngrsp.GetRangStatus();

  NS_ASSERT_MSG(m_rangingStatus == WimaxNetDevice::RANGING_STATUS_CONTINUE ||
                    m_rangingStatus == WimaxNetDevice::RANGING_STATUS_ABORT ||
                    m_rangingStatus == WimaxNetDevice::RANGING_STATUS_SUCCESS,
                "SS: Can not perform ranging: the ranging status should be "
                "RANGING_STATUS_CONTINUE or RANGING_STATUS_ABORT or "
                "RANGING_STATUS_SUCCESS");

  if (m_rangingStatus == WimaxNetDevice::RANGING_STATUS_ABORT) {
    if (rngrsp.GetDlFreqOverride()) {
    }
    m_ss->SetBasicConnection(nullptr);
    m_ss->SetPrimaryConnection(nullptr);
    m_ss->SetAreManagementConnectionsAllocated(false);
  } else {
    AdjustRangingParameters(rngrsp);

    if (m_rangingStatus == WimaxNetDevice::RANGING_STATUS_SUCCESS) {
      m_ss->SetState(SubscriberStationNetDevice::SS_STATE_REGISTERED);
      if (m_ss->HasServiceFlows() && !m_ss->GetAreServiceFlowsAllocated()) {
        m_ss->GetServiceFlowManager()->InitiateServiceFlows();
      }

      NegotiateBasicCapabilities();
    } else {
      m_ss->SetState(
          SubscriberStationNetDevice::SS_STATE_WAITING_INV_RANG_INTRVL);
    }
  }
}

void SSLinkManager::DeleteUplinkParameters() { m_ss->SetCurrentUcd(Ucd()); }

bool SSLinkManager::IsUlChannelUsable() { return true; }

void SSLinkManager::AdjustRangingParameters(const RngRsp &rngrsp) {
#if 0
  bool successful = true;
  uint8_t temp = rngrsp.GetTimingAdjust ();
  temp = rngrsp.GetPowerLevelAdjust ();
  temp = rngrsp.GetOffsetFreqAdjust ();


  if (!successful)
    {
    }
#endif
}

void SSLinkManager::NegotiateBasicCapabilities() {}

uint16_t SSLinkManager::CalculateMaxIRSignalStrength() {
  uint16_t rss = 1;

  if (m_bsEirp == 65535 || m_eirXPIrMax == 65535) {
    return GetMinTransmitPowerLevel();
  } else {
    return m_eirXPIrMax + m_bsEirp - rss;
  }

  return 0;
}

uint16_t SSLinkManager::GetMinTransmitPowerLevel() { return 10; }

void SSLinkManager::ScheduleScanningRestart(
    Time interval, SubscriberStationNetDevice::EventType eventType,
    bool deleteUlParameters, EventId &eventId) {
  m_ss->SetTimer(Simulator::Schedule(interval, &SSLinkManager::StartScanning,
                                     this, eventType, deleteUlParameters),
                 eventId);
}

} // namespace ns3
