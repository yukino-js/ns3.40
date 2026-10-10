
#include "rr-ff-mac-scheduler.h"

#include "lte-amc.h"
#include "lte-common.h"
#include "lte-vendor-specific-parameters.h"

#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>

#include <cfloat>
#include <climits>
#include <set>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RrFfMacScheduler");

static const int Type0AllocationRbg[4] = {
    10,
    26,
    63,
    110,
};

NS_OBJECT_ENSURE_REGISTERED(RrFfMacScheduler);

RrFfMacScheduler::RrFfMacScheduler()
    : m_cschedSapUser(nullptr), m_schedSapUser(nullptr), m_nextRntiDl(0),
      m_nextRntiUl(0) {
  m_amc = CreateObject<LteAmc>();
  m_cschedSapProvider = new MemberCschedSapProvider<RrFfMacScheduler>(this);
  m_schedSapProvider = new MemberSchedSapProvider<RrFfMacScheduler>(this);
}

RrFfMacScheduler::~RrFfMacScheduler() { NS_LOG_FUNCTION(this); }

void RrFfMacScheduler::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_dlHarqProcessesDciBuffer.clear();
  m_dlHarqProcessesTimer.clear();
  m_dlHarqProcessesRlcPduListBuffer.clear();
  m_dlInfoListBuffered.clear();
  m_ulHarqCurrentProcessId.clear();
  m_ulHarqProcessesStatus.clear();
  m_ulHarqProcessesDciBuffer.clear();
  delete m_cschedSapProvider;
  delete m_schedSapProvider;
}

TypeId RrFfMacScheduler::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::RrFfMacScheduler")
          .SetParent<FfMacScheduler>()
          .SetGroupName("Lte")
          .AddConstructor<RrFfMacScheduler>()
          .AddAttribute(
              "CqiTimerThreshold",
              "The number of TTIs a CQI is valid (default 1000 - 1 sec.)",
              UintegerValue(1000),
              MakeUintegerAccessor(&RrFfMacScheduler::m_cqiTimersThreshold),
              MakeUintegerChecker<uint32_t>())
          .AddAttribute("HarqEnabled",
                        "Activate/Deactivate the HARQ [by default is active].",
                        BooleanValue(true),
                        MakeBooleanAccessor(&RrFfMacScheduler::m_harqOn),
                        MakeBooleanChecker())
          .AddAttribute("UlGrantMcs",
                        "The MCS of the UL grant, must be [0..15] (default 0)",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RrFfMacScheduler::m_ulGrantMcs),
                        MakeUintegerChecker<uint8_t>());
  return tid;
}

void RrFfMacScheduler::SetFfMacCschedSapUser(FfMacCschedSapUser *s) {
  m_cschedSapUser = s;
}

void RrFfMacScheduler::SetFfMacSchedSapUser(FfMacSchedSapUser *s) {
  m_schedSapUser = s;
}

FfMacCschedSapProvider *RrFfMacScheduler::GetFfMacCschedSapProvider() {
  return m_cschedSapProvider;
}

FfMacSchedSapProvider *RrFfMacScheduler::GetFfMacSchedSapProvider() {
  return m_schedSapProvider;
}

void RrFfMacScheduler::SetLteFfrSapProvider(LteFfrSapProvider *s) {
  m_ffrSapProvider = s;
}

LteFfrSapUser *RrFfMacScheduler::GetLteFfrSapUser() { return m_ffrSapUser; }

void RrFfMacScheduler::DoCschedCellConfigReq(
    const FfMacCschedSapProvider::CschedCellConfigReqParameters &params) {
  NS_LOG_FUNCTION(this);
  m_cschedCellConfig = params;
  m_rachAllocationMap.resize(m_cschedCellConfig.m_ulBandwidth, 0);
  FfMacCschedSapUser::CschedUeConfigCnfParameters cnf;
  cnf.m_result = SUCCESS;
  m_cschedSapUser->CschedUeConfigCnf(cnf);
}

void RrFfMacScheduler::DoCschedUeConfigReq(
    const FfMacCschedSapProvider::CschedUeConfigReqParameters &params) {
  NS_LOG_FUNCTION(this << " RNTI " << params.m_rnti << " txMode "
                       << (uint16_t)params.m_transmissionMode);
  auto it = m_uesTxMode.find(params.m_rnti);
  if (it == m_uesTxMode.end()) {
    m_uesTxMode.insert(
        std::pair<uint16_t, double>(params.m_rnti, params.m_transmissionMode));
    m_dlHarqCurrentProcessId.insert(
        std::pair<uint16_t, uint8_t>(params.m_rnti, 0));
    DlHarqProcessesStatus_t dlHarqPrcStatus;
    dlHarqPrcStatus.resize(8, 0);
    m_dlHarqProcessesStatus.insert(std::pair<uint16_t, DlHarqProcessesStatus_t>(
        params.m_rnti, dlHarqPrcStatus));
    DlHarqProcessesTimer_t dlHarqProcessesTimer;
    dlHarqProcessesTimer.resize(8, 0);
    m_dlHarqProcessesTimer.insert(std::pair<uint16_t, DlHarqProcessesTimer_t>(
        params.m_rnti, dlHarqProcessesTimer));
    DlHarqProcessesDciBuffer_t dlHarqdci;
    dlHarqdci.resize(8);
    m_dlHarqProcessesDciBuffer.insert(
        std::pair<uint16_t, DlHarqProcessesDciBuffer_t>(params.m_rnti,
                                                        dlHarqdci));
    DlHarqRlcPduListBuffer_t dlHarqRlcPdu;
    dlHarqRlcPdu.resize(2);
    dlHarqRlcPdu.at(0).resize(8);
    dlHarqRlcPdu.at(1).resize(8);
    m_dlHarqProcessesRlcPduListBuffer.insert(
        std::pair<uint16_t, DlHarqRlcPduListBuffer_t>(params.m_rnti,
                                                      dlHarqRlcPdu));
    m_ulHarqCurrentProcessId.insert(
        std::pair<uint16_t, uint8_t>(params.m_rnti, 0));
    UlHarqProcessesStatus_t ulHarqPrcStatus;
    ulHarqPrcStatus.resize(8, 0);
    m_ulHarqProcessesStatus.insert(std::pair<uint16_t, UlHarqProcessesStatus_t>(
        params.m_rnti, ulHarqPrcStatus));
    UlHarqProcessesDciBuffer_t ulHarqdci;
    ulHarqdci.resize(8);
    m_ulHarqProcessesDciBuffer.insert(
        std::pair<uint16_t, UlHarqProcessesDciBuffer_t>(params.m_rnti,
                                                        ulHarqdci));
  } else {
    (*it).second = params.m_transmissionMode;
  }
}

void RrFfMacScheduler::DoCschedLcConfigReq(
    const FfMacCschedSapProvider::CschedLcConfigReqParameters &params) {
  NS_LOG_FUNCTION(this);
}

void RrFfMacScheduler::DoCschedLcReleaseReq(
    const FfMacCschedSapProvider::CschedLcReleaseReqParameters &params) {
  NS_LOG_FUNCTION(this);
  for (std::size_t i = 0; i < params.m_logicalChannelIdentity.size(); i++) {
    auto it = m_rlcBufferReq.begin();
    while (it != m_rlcBufferReq.end()) {
      if (((*it).m_rnti == params.m_rnti) &&
          ((*it).m_logicalChannelIdentity ==
           params.m_logicalChannelIdentity.at(i))) {
        it = m_rlcBufferReq.erase(it);
      } else {
        it++;
      }
    }
  }
}

void RrFfMacScheduler::DoCschedUeReleaseReq(
    const FfMacCschedSapProvider::CschedUeReleaseReqParameters &params) {
  NS_LOG_FUNCTION(this << " Release RNTI " << params.m_rnti);

  m_uesTxMode.erase(params.m_rnti);
  m_dlHarqCurrentProcessId.erase(params.m_rnti);
  m_dlHarqProcessesStatus.erase(params.m_rnti);
  m_dlHarqProcessesTimer.erase(params.m_rnti);
  m_dlHarqProcessesDciBuffer.erase(params.m_rnti);
  m_dlHarqProcessesRlcPduListBuffer.erase(params.m_rnti);
  m_ulHarqCurrentProcessId.erase(params.m_rnti);
  m_ulHarqProcessesStatus.erase(params.m_rnti);
  m_ulHarqProcessesDciBuffer.erase(params.m_rnti);
  m_ceBsrRxed.erase(params.m_rnti);
  auto it = m_rlcBufferReq.begin();
  while (it != m_rlcBufferReq.end()) {
    if ((*it).m_rnti == params.m_rnti) {
      NS_LOG_INFO(this << " Erase RNTI " << (*it).m_rnti << " LC "
                       << (uint16_t)(*it).m_logicalChannelIdentity);
      it = m_rlcBufferReq.erase(it);
    } else {
      it++;
    }
  }
  if (m_nextRntiUl == params.m_rnti) {
    m_nextRntiUl = 0;
  }

  if (m_nextRntiDl == params.m_rnti) {
    m_nextRntiDl = 0;
  }
}

void RrFfMacScheduler::DoSchedDlRlcBufferReq(
    const FfMacSchedSapProvider::SchedDlRlcBufferReqParameters &params) {
  NS_LOG_FUNCTION(this << params.m_rnti
                       << (uint32_t)params.m_logicalChannelIdentity);
  auto it = m_rlcBufferReq.begin();
  bool newLc = true;
  while (it != m_rlcBufferReq.end()) {
    if (((*it).m_rnti == params.m_rnti) &&
        ((*it).m_logicalChannelIdentity == params.m_logicalChannelIdentity)) {
      it = m_rlcBufferReq.erase(it);
      newLc = false;
    } else {
      ++it;
    }
  }
  m_rlcBufferReq.insert(it, params);
  NS_LOG_INFO(this << " RNTI " << params.m_rnti << " LC "
                   << (uint16_t)params.m_logicalChannelIdentity
                   << " RLC tx size " << params.m_rlcTransmissionQueueSize
                   << " RLC retx size " << params.m_rlcRetransmissionQueueSize
                   << " RLC stat size " << params.m_rlcStatusPduSize);
  if (newLc) {
    m_p10CqiRxed.insert(std::pair<uint16_t, uint8_t>(params.m_rnti, 1));
    m_p10CqiTimers.insert(
        std::pair<uint16_t, uint32_t>(params.m_rnti, m_cqiTimersThreshold));
  }
}

void RrFfMacScheduler::DoSchedDlPagingBufferReq(
    const FfMacSchedSapProvider::SchedDlPagingBufferReqParameters &params) {
  NS_LOG_FUNCTION(this);
  NS_FATAL_ERROR("method not implemented");
}

void RrFfMacScheduler::DoSchedDlMacBufferReq(
    const FfMacSchedSapProvider::SchedDlMacBufferReqParameters &params) {
  NS_LOG_FUNCTION(this);
  NS_FATAL_ERROR("method not implemented");
}

int RrFfMacScheduler::GetRbgSize(int dlbandwidth) {
  for (int i = 0; i < 4; i++) {
    if (dlbandwidth < Type0AllocationRbg[i]) {
      return (i + 1);
    }
  }

  return (-1);
}

bool RrFfMacScheduler::SortRlcBufferReq(
    FfMacSchedSapProvider::SchedDlRlcBufferReqParameters i,
    FfMacSchedSapProvider::SchedDlRlcBufferReqParameters j) {
  return (i.m_rnti < j.m_rnti);
}

bool RrFfMacScheduler::HarqProcessAvailability(uint16_t rnti) {
  NS_LOG_FUNCTION(this << rnti);

  auto it = m_dlHarqCurrentProcessId.find(rnti);
  if (it == m_dlHarqCurrentProcessId.end()) {
    NS_FATAL_ERROR("No Process Id found for this RNTI " << rnti);
  }
  auto itStat = m_dlHarqProcessesStatus.find(rnti);
  if (itStat == m_dlHarqProcessesStatus.end()) {
    NS_FATAL_ERROR("No Process Id Statusfound for this RNTI " << rnti);
  }
  uint8_t i = (*it).second;
  do {
    i = (i + 1) % HARQ_PROC_NUM;
  } while (((*itStat).second.at(i) != 0) && (i != (*it).second));

  return (*itStat).second.at(i) == 0;
}

uint8_t RrFfMacScheduler::UpdateHarqProcessId(uint16_t rnti) {
  NS_LOG_FUNCTION(this << rnti);

  if (!m_harqOn) {
    return (0);
  }

  auto it = m_dlHarqCurrentProcessId.find(rnti);
  if (it == m_dlHarqCurrentProcessId.end()) {
    NS_FATAL_ERROR("No Process Id found for this RNTI " << rnti);
  }
  auto itStat = m_dlHarqProcessesStatus.find(rnti);
  if (itStat == m_dlHarqProcessesStatus.end()) {
    NS_FATAL_ERROR("No Process Id Statusfound for this RNTI " << rnti);
  }
  uint8_t i = (*it).second;
  do {
    i = (i + 1) % HARQ_PROC_NUM;
  } while (((*itStat).second.at(i) != 0) && (i != (*it).second));
  if ((*itStat).second.at(i) == 0) {
    (*it).second = i;
    (*itStat).second.at(i) = 1;
  } else {
    return (9);
  }

  return ((*it).second);
}

void RrFfMacScheduler::RefreshHarqProcesses() {
  NS_LOG_FUNCTION(this);

  for (auto itTimers = m_dlHarqProcessesTimer.begin();
       itTimers != m_dlHarqProcessesTimer.end(); itTimers++) {
    for (uint16_t i = 0; i < HARQ_PROC_NUM; i++) {
      if ((*itTimers).second.at(i) == HARQ_DL_TIMEOUT) {

        NS_LOG_INFO(this << " Reset HARQ proc " << i << " for RNTI "
                         << (*itTimers).first);
        auto itStat = m_dlHarqProcessesStatus.find((*itTimers).first);
        if (itStat == m_dlHarqProcessesStatus.end()) {
          NS_FATAL_ERROR("No Process Id Status found for this RNTI "
                         << (*itTimers).first);
        }
        (*itStat).second.at(i) = 0;
        (*itTimers).second.at(i) = 0;
      } else {
        (*itTimers).second.at(i)++;
      }
    }
  }
}

void RrFfMacScheduler::DoSchedDlTriggerReq(
    const FfMacSchedSapProvider::SchedDlTriggerReqParameters &params) {
  NS_LOG_FUNCTION(this << " DL Frame no. " << (params.m_sfnSf >> 4)
                       << " subframe no. " << (0xF & params.m_sfnSf));

  RefreshDlCqiMaps();
  int rbgSize = GetRbgSize(m_cschedCellConfig.m_dlBandwidth);
  int rbgNum = m_cschedCellConfig.m_dlBandwidth / rbgSize;
  FfMacSchedSapUser::SchedDlConfigIndParameters ret;

  std::vector<bool> rbgMap;
  uint16_t rbgAllocatedNum = 0;
  std::set<uint16_t> rntiAllocated;
  rbgMap.resize(m_cschedCellConfig.m_dlBandwidth / rbgSize, false);

  for (auto itProcId = m_ulHarqCurrentProcessId.begin();
       itProcId != m_ulHarqCurrentProcessId.end(); itProcId++) {
    (*itProcId).second = ((*itProcId).second + 1) % HARQ_PROC_NUM;
  }

  m_rachAllocationMap.resize(m_cschedCellConfig.m_ulBandwidth, 0);
  uint16_t rbStart = 0;
  for (auto itRach = m_rachList.begin(); itRach != m_rachList.end(); itRach++) {
    NS_ASSERT_MSG(m_amc->GetUlTbSizeFromMcs(m_ulGrantMcs,
                                            m_cschedCellConfig.m_ulBandwidth) >
                      (*itRach).m_estimatedSize,
                  " Default UL Grant MCS does not allow to send RACH messages");
    BuildRarListElement_s newRar;
    newRar.m_rnti = (*itRach).m_rnti;
    newRar.m_grant.m_rnti = newRar.m_rnti;
    newRar.m_grant.m_mcs = m_ulGrantMcs;
    uint16_t rbLen = 1;
    uint16_t tbSizeBits = 0;
    while ((tbSizeBits < (*itRach).m_estimatedSize) &&
           (rbStart + rbLen < m_cschedCellConfig.m_ulBandwidth)) {
      rbLen++;
      tbSizeBits = m_amc->GetUlTbSizeFromMcs(m_ulGrantMcs, rbLen);
    }
    if (tbSizeBits < (*itRach).m_estimatedSize) {
      break;
    }
    newRar.m_grant.m_rbStart = rbStart;
    newRar.m_grant.m_rbLen = rbLen;
    newRar.m_grant.m_tbSize = tbSizeBits / 8;
    newRar.m_grant.m_hopping = false;
    newRar.m_grant.m_tpc = 0;
    newRar.m_grant.m_cqiRequest = false;
    newRar.m_grant.m_ulDelay = false;
    NS_LOG_INFO(this << " UL grant allocated to RNTI " << (*itRach).m_rnti
                     << " rbStart " << rbStart << " rbLen " << rbLen << " MCS "
                     << (uint16_t)m_ulGrantMcs << " tbSize "
                     << newRar.m_grant.m_tbSize);
    for (uint16_t i = rbStart; i < rbStart + rbLen; i++) {
      m_rachAllocationMap.at(i) = (*itRach).m_rnti;
    }

    if (m_harqOn) {
      UlDciListElement_s uldci;
      uldci.m_rnti = newRar.m_rnti;
      uldci.m_rbLen = rbLen;
      uldci.m_rbStart = rbStart;
      uldci.m_mcs = m_ulGrantMcs;
      uldci.m_tbSize = tbSizeBits / 8;
      uldci.m_ndi = 1;
      uldci.m_cceIndex = 0;
      uldci.m_aggrLevel = 1;
      uldci.m_ueTxAntennaSelection = 3;
      uldci.m_hopping = false;
      uldci.m_n2Dmrs = 0;
      uldci.m_tpc = 0;
      uldci.m_cqiRequest = false;
      uldci.m_ulIndex = 0;
      uldci.m_dai = 1;
      uldci.m_freqHopping = 0;
      uldci.m_pdcchPowerOffset = 0;

      uint8_t harqId = 0;
      auto itProcId = m_ulHarqCurrentProcessId.find(uldci.m_rnti);
      if (itProcId == m_ulHarqCurrentProcessId.end()) {
        NS_FATAL_ERROR("No info find in HARQ buffer for UE " << uldci.m_rnti);
      }
      harqId = (*itProcId).second;
      auto itDci = m_ulHarqProcessesDciBuffer.find(uldci.m_rnti);
      if (itDci == m_ulHarqProcessesDciBuffer.end()) {
        NS_FATAL_ERROR(
            "Unable to find RNTI entry in UL DCI HARQ buffer for RNTI "
            << uldci.m_rnti);
      }
      (*itDci).second.at(harqId) = uldci;
    }

    rbStart = rbStart + rbLen;
    ret.m_buildRarList.push_back(newRar);
  }
  m_rachList.clear();

  RefreshHarqProcesses();
  if (!m_dlInfoListBuffered.empty()) {
    if (!params.m_dlInfoList.empty()) {
      NS_LOG_INFO(this << " Received DL-HARQ feedback");
      m_dlInfoListBuffered.insert(m_dlInfoListBuffered.end(),
                                  params.m_dlInfoList.begin(),
                                  params.m_dlInfoList.end());
    }
  } else {
    if (!params.m_dlInfoList.empty()) {
      m_dlInfoListBuffered = params.m_dlInfoList;
    }
  }
  if (!m_harqOn) {
    m_dlInfoListBuffered.clear();
  }
  std::vector<DlInfoListElement_s> dlInfoListUntxed;
  for (std::size_t i = 0; i < m_dlInfoListBuffered.size(); i++) {
    auto itRnti = rntiAllocated.find(m_dlInfoListBuffered.at(i).m_rnti);
    if (itRnti != rntiAllocated.end()) {
      continue;
    }
    auto nLayers = m_dlInfoListBuffered.at(i).m_harqStatus.size();
    std::vector<bool> retx;
    NS_LOG_INFO(this << " Processing DLHARQ feedback");
    if (nLayers == 1) {
      retx.push_back(m_dlInfoListBuffered.at(i).m_harqStatus.at(0) ==
                     DlInfoListElement_s::NACK);
      retx.push_back(false);
    } else {
      retx.push_back(m_dlInfoListBuffered.at(i).m_harqStatus.at(0) ==
                     DlInfoListElement_s::NACK);
      retx.push_back(m_dlInfoListBuffered.at(i).m_harqStatus.at(1) ==
                     DlInfoListElement_s::NACK);
    }
    if (retx.at(0) || retx.at(1)) {
      uint16_t rnti = m_dlInfoListBuffered.at(i).m_rnti;
      uint8_t harqId = m_dlInfoListBuffered.at(i).m_harqProcessId;
      NS_LOG_INFO(this << " HARQ retx RNTI " << rnti << " harqId "
                       << (uint16_t)harqId);
      auto itHarq = m_dlHarqProcessesDciBuffer.find(rnti);
      if (itHarq == m_dlHarqProcessesDciBuffer.end()) {
        NS_FATAL_ERROR("No info find in HARQ buffer for UE " << rnti);
      }

      DlDciListElement_s dci = (*itHarq).second.at(harqId);
      int rv = 0;
      if (dci.m_rv.size() == 1) {
        rv = dci.m_rv.at(0);
      } else {
        rv =
            (dci.m_rv.at(0) > dci.m_rv.at(1) ? dci.m_rv.at(0) : dci.m_rv.at(1));
      }

      if (rv == 3) {
        NS_LOG_INFO("Max number of retransmissions reached -> drop process");
        auto it = m_dlHarqProcessesStatus.find(rnti);
        if (it == m_dlHarqProcessesStatus.end()) {
          NS_LOG_ERROR("No info find in HARQ buffer for UE (might change eNB) "
                       << m_dlInfoListBuffered.at(i).m_rnti);
        }
        (*it).second.at(harqId) = 0;
        auto itRlcPdu = m_dlHarqProcessesRlcPduListBuffer.find(rnti);
        if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end()) {
          NS_FATAL_ERROR("Unable to find RlcPdcList in HARQ buffer for RNTI "
                         << m_dlInfoListBuffered.at(i).m_rnti);
        }
        for (std::size_t k = 0; k < (*itRlcPdu).second.size(); k++) {
          (*itRlcPdu).second.at(k).at(harqId).clear();
        }
        continue;
      }
      std::vector<int> dciRbg;
      uint32_t mask = 0x1;
      NS_LOG_INFO("Original RBGs " << dci.m_rbBitmap << " rnti " << dci.m_rnti);
      for (int j = 0; j < 32; j++) {
        if (((dci.m_rbBitmap & mask) >> j) == 1) {
          dciRbg.push_back(j);
          NS_LOG_INFO("\t" << j);
        }
        mask = (mask << 1);
      }
      bool free = true;
      for (std::size_t j = 0; j < dciRbg.size(); j++) {
        if (rbgMap.at(dciRbg.at(j))) {
          free = false;
          break;
        }
      }
      if (free) {
        for (std::size_t j = 0; j < dciRbg.size(); j++) {
          rbgMap.at(dciRbg.at(j)) = true;
          NS_LOG_INFO("RBG " << dciRbg.at(j) << " assigned");
          rbgAllocatedNum++;
        }

        NS_LOG_INFO(this << " Send retx in the same RBGs");
      } else {
        uint8_t j = 0;
        uint8_t rbgId = (dciRbg.at(dciRbg.size() - 1) + 1) % rbgNum;
        uint8_t startRbg = dciRbg.at(dciRbg.size() - 1);
        std::vector<bool> rbgMapCopy = rbgMap;
        while ((j < dciRbg.size()) && (startRbg != rbgId)) {
          if (!rbgMapCopy.at(rbgId)) {
            rbgMapCopy.at(rbgId) = true;
            dciRbg.at(j) = rbgId;
            j++;
          }
          rbgId = (rbgId + 1) % rbgNum;
        }
        if (j == dciRbg.size()) {
          uint32_t rbgMask = 0;
          for (std::size_t k = 0; k < dciRbg.size(); k++) {
            rbgMask = rbgMask + (0x1 << dciRbg.at(k));
            NS_LOG_INFO(this << " New allocated RBG " << dciRbg.at(k));
            rbgAllocatedNum++;
          }
          dci.m_rbBitmap = rbgMask;
          rbgMap = rbgMapCopy;
        } else {
          dlInfoListUntxed.push_back(m_dlInfoListBuffered.at(i));
          NS_LOG_INFO(this << " No resource for this retx -> buffer it");
        }
      }
      BuildDataListElement_s newEl;
      auto itRlcPdu = m_dlHarqProcessesRlcPduListBuffer.find(rnti);
      if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end()) {
        NS_FATAL_ERROR("Unable to find RlcPdcList in HARQ buffer for RNTI "
                       << rnti);
      }
      for (std::size_t j = 0; j < nLayers; j++) {
        if (retx.at(j)) {
          if (j >= dci.m_ndi.size()) {
            dci.m_ndi.push_back(0);
            dci.m_rv.push_back(0);
            dci.m_mcs.push_back(0);
            dci.m_tbsSize.push_back(0);
            NS_LOG_INFO(this << " layer " << (uint16_t)j
                             << " no txed (MIMO transition)");
          } else {
            dci.m_ndi.at(j) = 0;
            dci.m_rv.at(j)++;
            (*itHarq).second.at(harqId).m_rv.at(j)++;
            NS_LOG_INFO(this << " layer " << (uint16_t)j << " RV "
                             << (uint16_t)dci.m_rv.at(j));
          }
        } else {
          dci.m_ndi.at(j) = 0;
          dci.m_rv.at(j) = 0;
          dci.m_mcs.at(j) = 0;
          dci.m_tbsSize.at(j) = 0;
          NS_LOG_INFO(this << " layer " << (uint16_t)j << " no retx");
        }
      }

      for (std::size_t k = 0;
           k < (*itRlcPdu).second.at(0).at(dci.m_harqProcess).size(); k++) {
        std::vector<RlcPduListElement_s> rlcPduListPerLc;
        for (std::size_t j = 0; j < nLayers; j++) {
          if (retx.at(j)) {
            if (j < dci.m_ndi.size()) {
              NS_LOG_INFO(" layer " << (uint16_t)j << " tb size "
                                    << dci.m_tbsSize.at(j));
              rlcPduListPerLc.push_back(
                  (*itRlcPdu).second.at(j).at(dci.m_harqProcess).at(k));
            }
          } else {
            NS_LOG_INFO(" layer " << (uint16_t)j << " tb size "
                                  << dci.m_tbsSize.at(j));
            RlcPduListElement_s emptyElement;
            emptyElement.m_logicalChannelIdentity =
                (*itRlcPdu)
                    .second.at(j)
                    .at(dci.m_harqProcess)
                    .at(k)
                    .m_logicalChannelIdentity;
            emptyElement.m_size = 0;
            rlcPduListPerLc.push_back(emptyElement);
          }
        }

        if (!rlcPduListPerLc.empty()) {
          newEl.m_rlcPduList.push_back(rlcPduListPerLc);
        }
      }
      newEl.m_rnti = rnti;
      newEl.m_dci = dci;
      (*itHarq).second.at(harqId).m_rv = dci.m_rv;
      auto itHarqTimer = m_dlHarqProcessesTimer.find(rnti);
      if (itHarqTimer == m_dlHarqProcessesTimer.end()) {
        NS_FATAL_ERROR("Unable to find HARQ timer for RNTI " << (uint16_t)rnti);
      }
      (*itHarqTimer).second.at(harqId) = 0;
      ret.m_buildDataList.push_back(newEl);
      rntiAllocated.insert(rnti);
    } else {
      NS_LOG_INFO(this << " HARQ ACK UE " << m_dlInfoListBuffered.at(i).m_rnti);
      auto it = m_dlHarqProcessesStatus.find(m_dlInfoListBuffered.at(i).m_rnti);
      if (it == m_dlHarqProcessesStatus.end()) {
        NS_FATAL_ERROR("No info find in HARQ buffer for UE "
                       << m_dlInfoListBuffered.at(i).m_rnti);
      }
      (*it).second.at(m_dlInfoListBuffered.at(i).m_harqProcessId) = 0;
      auto itRlcPdu = m_dlHarqProcessesRlcPduListBuffer.find(
          m_dlInfoListBuffered.at(i).m_rnti);
      if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end()) {
        NS_FATAL_ERROR("Unable to find RlcPdcList in HARQ buffer for RNTI "
                       << m_dlInfoListBuffered.at(i).m_rnti);
      }
      for (std::size_t k = 0; k < (*itRlcPdu).second.size(); k++) {
        (*itRlcPdu)
            .second.at(k)
            .at(m_dlInfoListBuffered.at(i).m_harqProcessId)
            .clear();
      }
    }
  }
  m_dlInfoListBuffered.clear();
  m_dlInfoListBuffered = dlInfoListUntxed;

  if (rbgAllocatedNum == rbgNum) {
    if (!ret.m_buildDataList.empty() || !ret.m_buildRarList.empty()) {
      m_schedSapUser->SchedDlConfigInd(ret);
    }
    return;
  }

  std::list<FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  m_rlcBufferReq.sort(SortRlcBufferReq);
  int nflows = 0;
  int nTbs = 0;
  std::map<uint16_t, uint8_t> lcActivesPerRnti;
  for (it = m_rlcBufferReq.begin(); it != m_rlcBufferReq.end(); it++) {
    auto itRnti = rntiAllocated.find((*it).m_rnti);
    if ((((*it).m_rlcTransmissionQueueSize > 0) ||
         ((*it).m_rlcRetransmissionQueueSize > 0) ||
         ((*it).m_rlcStatusPduSize > 0)) &&
        (itRnti == rntiAllocated.end()) &&
        (HarqProcessAvailability((*it).m_rnti)))

    {
      NS_LOG_LOGIC(this << " User " << (*it).m_rnti << " LC "
                        << (uint16_t)(*it).m_logicalChannelIdentity
                        << " is active, status  " << (*it).m_rlcStatusPduSize
                        << " retx " << (*it).m_rlcRetransmissionQueueSize
                        << " tx " << (*it).m_rlcTransmissionQueueSize);
      auto itCqi = m_p10CqiRxed.find((*it).m_rnti);
      uint8_t cqi = 0;
      if (itCqi != m_p10CqiRxed.end()) {
        cqi = (*itCqi).second;
      } else {
        cqi = 1;
      }
      if (cqi != 0) {
        nflows++;
        auto itLcRnti = lcActivesPerRnti.find((*it).m_rnti);
        if (itLcRnti != lcActivesPerRnti.end()) {
          (*itLcRnti).second++;
        } else {
          lcActivesPerRnti.insert(
              std::pair<uint16_t, uint8_t>((*it).m_rnti, 1));
          nTbs++;
        }
      }
    }
  }

  if (nflows == 0) {
    if ((!ret.m_buildDataList.empty()) || (!ret.m_buildRarList.empty())) {
      m_schedSapUser->SchedDlConfigInd(ret);
    }
    return;
  }

  int rbgPerTb = (nTbs > 0) ? ((rbgNum - rbgAllocatedNum) / nTbs) : INT_MAX;
  NS_LOG_INFO(this << " Flows to be transmitted " << nflows << " rbgPerTb "
                   << rbgPerTb);
  if (rbgPerTb == 0) {
    rbgPerTb = 1;
  }
  int rbgAllocated = 0;

  if (m_nextRntiDl != 0) {
    NS_LOG_DEBUG("Start from the successive of " << (uint16_t)m_nextRntiDl);
    for (it = m_rlcBufferReq.begin(); it != m_rlcBufferReq.end(); it++) {
      if ((*it).m_rnti == m_nextRntiDl) {
        it++;
        if (it == m_rlcBufferReq.end()) {
          it = m_rlcBufferReq.begin();
        }
        m_nextRntiDl = (*it).m_rnti;
        break;
      }
    }

    if (it == m_rlcBufferReq.end()) {
      NS_LOG_ERROR(this << " no user found");
    }
  } else {
    it = m_rlcBufferReq.begin();
    m_nextRntiDl = (*it).m_rnti;
  }
  do {
    auto itLcRnti = lcActivesPerRnti.find((*it).m_rnti);
    auto itRnti = rntiAllocated.find((*it).m_rnti);
    if ((itLcRnti == lcActivesPerRnti.end()) ||
        (itRnti != rntiAllocated.end())) {
      uint16_t rntiDiscarded = (*it).m_rnti;
      while (it != m_rlcBufferReq.end()) {
        if ((*it).m_rnti != rntiDiscarded) {
          break;
        }
        it++;
      }
      if (it == m_rlcBufferReq.end()) {
        it = m_rlcBufferReq.begin();
      }
      continue;
    }
    auto itTxMode = m_uesTxMode.find((*it).m_rnti);
    if (itTxMode == m_uesTxMode.end()) {
      NS_FATAL_ERROR("No Transmission Mode info on user " << (*it).m_rnti);
    }
    auto nLayer = TransmissionModesLayers::TxMode2LayerNum((*itTxMode).second);
    int lcNum = (*itLcRnti).second;
    BuildDataListElement_s newEl;
    newEl.m_rnti = (*it).m_rnti;
    DlDciListElement_s newDci;
    newDci.m_rnti = (*it).m_rnti;
    newDci.m_harqProcess = UpdateHarqProcessId((*it).m_rnti);
    newDci.m_resAlloc = 0;
    newDci.m_rbBitmap = 0;
    auto itCqi = m_p10CqiRxed.find(newEl.m_rnti);
    for (uint8_t i = 0; i < nLayer; i++) {
      if (itCqi == m_p10CqiRxed.end()) {
        newDci.m_mcs.push_back(0);
      } else {
        newDci.m_mcs.push_back(m_amc->GetMcsFromCqi((*itCqi).second));
      }
    }
    int tbSize =
        (m_amc->GetDlTbSizeFromMcs(newDci.m_mcs.at(0), rbgPerTb * rbgSize) / 8);
    uint16_t rlcPduSize = tbSize / lcNum;
    while ((*it).m_rnti == newEl.m_rnti) {
      if (((*it).m_rlcTransmissionQueueSize > 0) ||
          ((*it).m_rlcRetransmissionQueueSize > 0) ||
          ((*it).m_rlcStatusPduSize > 0)) {
        std::vector<RlcPduListElement_s> newRlcPduLe;
        for (uint8_t j = 0; j < nLayer; j++) {
          RlcPduListElement_s newRlcEl;
          newRlcEl.m_logicalChannelIdentity = (*it).m_logicalChannelIdentity;
          NS_LOG_INFO(this << "LCID "
                           << (uint32_t)newRlcEl.m_logicalChannelIdentity
                           << " size " << rlcPduSize << " ID " << (*it).m_rnti
                           << " layer " << (uint16_t)j);
          newRlcEl.m_size = rlcPduSize;
          UpdateDlRlcBufferInfo((*it).m_rnti, newRlcEl.m_logicalChannelIdentity,
                                rlcPduSize);
          newRlcPduLe.push_back(newRlcEl);

          if (m_harqOn) {
            auto itRlcPdu =
                m_dlHarqProcessesRlcPduListBuffer.find((*it).m_rnti);
            if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end()) {
              NS_FATAL_ERROR(
                  "Unable to find RlcPdcList in HARQ buffer for RNTI "
                  << (*it).m_rnti);
            }
            (*itRlcPdu)
                .second.at(j)
                .at(newDci.m_harqProcess)
                .push_back(newRlcEl);
          }
        }
        newEl.m_rlcPduList.push_back(newRlcPduLe);
        lcNum--;
      }
      it++;
      if (it == m_rlcBufferReq.end()) {
        it = m_rlcBufferReq.begin();
        break;
      }
    }
    uint32_t rbgMask = 0;
    uint16_t i = 0;
    NS_LOG_INFO(this << " DL - Allocate user " << newEl.m_rnti << " LCs "
                     << (uint16_t)(*itLcRnti).second << " bytes " << tbSize
                     << " mcs " << (uint16_t)newDci.m_mcs.at(0) << " harqId "
                     << (uint16_t)newDci.m_harqProcess << " layers " << nLayer);
    NS_LOG_INFO("RBG:");
    while (i < rbgPerTb) {
      if (!rbgMap.at(rbgAllocated)) {
        rbgMask = rbgMask + (0x1 << rbgAllocated);
        NS_LOG_INFO("\t " << rbgAllocated);
        i++;
        rbgMap.at(rbgAllocated) = true;
        rbgAllocatedNum++;
      }
      rbgAllocated++;
    }
    newDci.m_rbBitmap = rbgMask;

    for (std::size_t i = 0; i < nLayer; i++) {
      newDci.m_tbsSize.push_back(tbSize);
      newDci.m_ndi.push_back(1);
      newDci.m_rv.push_back(0);
    }

    newDci.m_tpc = 1;

    newEl.m_dci = newDci;
    if (m_harqOn) {
      auto itDci = m_dlHarqProcessesDciBuffer.find(newEl.m_rnti);
      if (itDci == m_dlHarqProcessesDciBuffer.end()) {
        NS_FATAL_ERROR("Unable to find RNTI entry in DCI HARQ buffer for RNTI "
                       << newEl.m_rnti);
      }
      (*itDci).second.at(newDci.m_harqProcess) = newDci;
      auto itHarqTimer = m_dlHarqProcessesTimer.find(newEl.m_rnti);
      if (itHarqTimer == m_dlHarqProcessesTimer.end()) {
        NS_FATAL_ERROR("Unable to find HARQ timer for RNTI "
                       << (uint16_t)newEl.m_rnti);
      }
      (*itHarqTimer).second.at(newDci.m_harqProcess) = 0;
    }

    ret.m_buildDataList.push_back(newEl);
    if (rbgAllocatedNum == rbgNum) {
      m_nextRntiDl = newEl.m_rnti;
      break;
    }
  } while ((*it).m_rnti != m_nextRntiDl);

  ret.m_nrOfPdcchOfdmSymbols = 1;

  m_schedSapUser->SchedDlConfigInd(ret);
}

void RrFfMacScheduler::DoSchedDlRachInfoReq(
    const FfMacSchedSapProvider::SchedDlRachInfoReqParameters &params) {
  NS_LOG_FUNCTION(this);

  m_rachList = params.m_rachList;
}

void RrFfMacScheduler::DoSchedDlCqiInfoReq(
    const FfMacSchedSapProvider::SchedDlCqiInfoReqParameters &params) {
  NS_LOG_FUNCTION(this);

  for (unsigned int i = 0; i < params.m_cqiList.size(); i++) {
    if (params.m_cqiList.at(i).m_cqiType == CqiListElement_s::P10) {
      NS_LOG_LOGIC("wideband CQI "
                   << (uint32_t)params.m_cqiList.at(i).m_wbCqi.at(0)
                   << " reported");
      uint16_t rnti = params.m_cqiList.at(i).m_rnti;
      auto it = m_p10CqiRxed.find(rnti);
      if (it == m_p10CqiRxed.end()) {
        m_p10CqiRxed.insert(std::pair<uint16_t, uint8_t>(
            rnti, params.m_cqiList.at(i).m_wbCqi.at(0)));
        m_p10CqiTimers.insert(
            std::pair<uint16_t, uint32_t>(rnti, m_cqiTimersThreshold));
      } else {
        (*it).second = params.m_cqiList.at(i).m_wbCqi.at(0);
        auto itTimers = m_p10CqiTimers.find(rnti);
        (*itTimers).second = m_cqiTimersThreshold;
      }
    } else if (params.m_cqiList.at(i).m_cqiType == CqiListElement_s::A30) {
    } else {
      NS_LOG_ERROR(this << " CQI type unknown");
    }
  }
}

void RrFfMacScheduler::DoSchedUlTriggerReq(
    const FfMacSchedSapProvider::SchedUlTriggerReqParameters &params) {
  NS_LOG_FUNCTION(this << " UL - Frame no. " << (params.m_sfnSf >> 4)
                       << " subframe no. " << (0xF & params.m_sfnSf) << " size "
                       << params.m_ulInfoList.size());

  RefreshUlCqiMaps();

  FfMacSchedSapUser::SchedUlConfigIndParameters ret;
  std::vector<bool> rbMap;
  std::set<uint16_t> rntiAllocated;
  std::vector<uint16_t> rbgAllocationMap;
  rbgAllocationMap = m_rachAllocationMap;
  m_rachAllocationMap.clear();
  m_rachAllocationMap.resize(m_cschedCellConfig.m_ulBandwidth, 0);

  rbMap.resize(m_cschedCellConfig.m_ulBandwidth, false);
  for (uint16_t i = 0; i < m_cschedCellConfig.m_ulBandwidth; i++) {
    if (rbgAllocationMap.at(i) != 0) {
      rbMap.at(i) = true;
      NS_LOG_DEBUG(this << " Allocated for RACH " << i);
    }
  }

  if (m_harqOn) {
    for (std::size_t i = 0; i < params.m_ulInfoList.size(); i++) {
      if (params.m_ulInfoList.at(i).m_receptionStatus ==
          UlInfoListElement_s::NotOk) {
        uint16_t rnti = params.m_ulInfoList.at(i).m_rnti;
        auto itProcId = m_ulHarqCurrentProcessId.find(rnti);
        if (itProcId == m_ulHarqCurrentProcessId.end()) {
          NS_LOG_ERROR("No info find in HARQ buffer for UE (might change eNB) "
                       << rnti);
        }
        uint8_t harqId =
            (uint8_t)((*itProcId).second - HARQ_PERIOD) % HARQ_PROC_NUM;
        NS_LOG_INFO(this << " UL-HARQ retx RNTI " << rnti << " harqId "
                         << (uint16_t)harqId);
        auto itHarq = m_ulHarqProcessesDciBuffer.find(rnti);
        if (itHarq == m_ulHarqProcessesDciBuffer.end()) {
          NS_LOG_ERROR(
              "No info find in UL-HARQ buffer for UE (might change eNB) "
              << rnti);
        }
        UlDciListElement_s dci = (*itHarq).second.at(harqId);
        auto itStat = m_ulHarqProcessesStatus.find(rnti);
        if (itStat == m_ulHarqProcessesStatus.end()) {
          NS_LOG_ERROR("No info find in HARQ buffer for UE (might change eNB) "
                       << rnti);
        }
        if ((*itStat).second.at(harqId) >= 3) {
          NS_LOG_INFO(
              "Max number of retransmissions reached (UL)-> drop process");
          continue;
        }
        bool free = true;
        for (int j = dci.m_rbStart; j < dci.m_rbStart + dci.m_rbLen; j++) {
          if (rbMap.at(j)) {
            free = false;
            NS_LOG_INFO(this << " BUSY " << j);
          }
        }
        if (free) {
          for (int j = dci.m_rbStart; j < dci.m_rbStart + dci.m_rbLen; j++) {
            rbMap.at(j) = true;
            rbgAllocationMap.at(j) = dci.m_rnti;
            NS_LOG_INFO("\tRB " << j);
          }
          NS_LOG_INFO(this << " Send retx in the same RBGs "
                           << (uint16_t)dci.m_rbStart << " to "
                           << dci.m_rbStart + dci.m_rbLen << " RV "
                           << (*itStat).second.at(harqId) + 1);
        } else {
          NS_LOG_INFO("Cannot allocate retx due to RACH allocations for UE "
                      << rnti);
          continue;
        }
        dci.m_ndi = 0;
        (*itStat).second.at((*itProcId).second) =
            (*itStat).second.at(harqId) + 1;
        (*itStat).second.at(harqId) = 0;
        (*itHarq).second.at((*itProcId).second) = dci;
        ret.m_dciList.push_back(dci);
        rntiAllocated.insert(dci.m_rnti);
      }
    }
  }

  std::map<uint16_t, uint32_t>::iterator it;
  int nflows = 0;

  for (it = m_ceBsrRxed.begin(); it != m_ceBsrRxed.end(); it++) {
    auto itRnti = rntiAllocated.find((*it).first);
    NS_LOG_INFO(this << " UE " << (*it).first << " queue " << (*it).second);
    if (((*it).second > 0) && (itRnti == rntiAllocated.end())) {
      nflows++;
    }
  }

  if (nflows == 0) {
    if (!ret.m_dciList.empty()) {
      m_allocationMaps.insert(std::pair<uint16_t, std::vector<uint16_t>>(
          params.m_sfnSf, rbgAllocationMap));
      m_schedSapUser->SchedUlConfigInd(ret);
    }
    return;
  }

  uint16_t rbPerFlow =
      (m_cschedCellConfig.m_ulBandwidth) / (nflows + rntiAllocated.size());
  if (rbPerFlow < 3) {
    rbPerFlow = 3;
  }
  uint16_t rbAllocated = 0;

  if (m_nextRntiUl != 0) {
    for (it = m_ceBsrRxed.begin(); it != m_ceBsrRxed.end(); it++) {
      if ((*it).first == m_nextRntiUl) {
        break;
      }
    }
    if (it == m_ceBsrRxed.end()) {
      NS_LOG_ERROR(this << " no user found");
    }
  } else {
    it = m_ceBsrRxed.begin();
    m_nextRntiUl = (*it).first;
  }
  NS_LOG_INFO(this << " NFlows " << nflows << " RB per Flow " << rbPerFlow);
  do {
    auto itRnti = rntiAllocated.find((*it).first);
    if ((itRnti != rntiAllocated.end()) || ((*it).second == 0)) {
      it++;
      if (it == m_ceBsrRxed.end()) {
        it = m_ceBsrRxed.begin();
      }
      continue;
    }
    if (rbAllocated + rbPerFlow - 1 > m_cschedCellConfig.m_ulBandwidth) {
      rbPerFlow = m_cschedCellConfig.m_ulBandwidth - rbAllocated;
      if (rbPerFlow < 3) {
        rbPerFlow = 0;
      }
    }
    NS_LOG_INFO(this << " try to allocate " << (*it).first);
    UlDciListElement_s uldci;
    uldci.m_rnti = (*it).first;
    uldci.m_rbLen = rbPerFlow;
    bool allocated = false;
    NS_LOG_INFO(this << " RB Allocated " << rbAllocated << " rbPerFlow "
                     << rbPerFlow << " flows " << nflows);
    while ((!allocated) &&
           ((rbAllocated + rbPerFlow - m_cschedCellConfig.m_ulBandwidth) < 1) &&
           (rbPerFlow != 0)) {
      bool free = true;
      for (int j = rbAllocated; j < rbAllocated + rbPerFlow; j++) {
        if (rbMap.at(j)) {
          free = false;
          break;
        }
      }
      if (free) {
        uldci.m_rbStart = rbAllocated;

        for (int j = rbAllocated; j < rbAllocated + rbPerFlow; j++) {
          rbMap.at(j) = true;
          rbgAllocationMap.at(j) = (*it).first;
          NS_LOG_INFO("\t " << j);
        }
        rbAllocated += rbPerFlow;
        allocated = true;
        break;
      }
      rbAllocated++;
      if (rbAllocated + rbPerFlow - 1 > m_cschedCellConfig.m_ulBandwidth) {
        rbPerFlow = m_cschedCellConfig.m_ulBandwidth - rbAllocated;
        if (rbPerFlow < 3) {
          rbPerFlow = 0;
        }
      }
    }
    if (!allocated) {
      m_nextRntiUl = (*it).first;
      if (!ret.m_dciList.empty()) {
        m_schedSapUser->SchedUlConfigInd(ret);
      }
      m_allocationMaps.insert(std::pair<uint16_t, std::vector<uint16_t>>(
          params.m_sfnSf, rbgAllocationMap));
      return;
    }
    auto itCqi = m_ueCqi.find((*it).first);
    int cqi = 0;
    if (itCqi == m_ueCqi.end()) {
      uldci.m_mcs = 0;
      NS_LOG_INFO(this << " UE does not have ULCQI " << (*it).first);
    } else {
      NS_ABORT_MSG_IF((*itCqi).second.empty(),
                      "CQI of RNTI = " << (*it).first << " has expired");
      double minSinr = (*itCqi).second.at(uldci.m_rbStart);
      for (uint16_t i = uldci.m_rbStart; i < uldci.m_rbStart + uldci.m_rbLen;
           i++) {
        if ((*itCqi).second.at(i) < minSinr) {
          minSinr = (*itCqi).second.at(i);
        }
      }
      double s = log2(1 + (std::pow(10, minSinr / 10) /
                           ((-std::log(5.0 * 0.00005)) / 1.5)));

      cqi = m_amc->GetCqiFromSpectralEfficiency(s);
      if (cqi == 0) {
        it++;
        if (it == m_ceBsrRxed.end()) {
          it = m_ceBsrRxed.begin();
        }
        NS_LOG_DEBUG(this << " UE discarded for CQI = 0, RNTI "
                          << uldci.m_rnti);
        for (uint16_t i = uldci.m_rbStart; i < uldci.m_rbStart + uldci.m_rbLen;
             i++) {
          rbgAllocationMap.at(i) = 0;
        }
        continue;
      }
      uldci.m_mcs = m_amc->GetMcsFromCqi(cqi);
    }
    uldci.m_tbSize = (m_amc->GetUlTbSizeFromMcs(uldci.m_mcs, rbPerFlow) / 8);

    UpdateUlRlcBufferInfo(uldci.m_rnti, uldci.m_tbSize);
    uldci.m_ndi = 1;
    uldci.m_cceIndex = 0;
    uldci.m_aggrLevel = 1;
    uldci.m_ueTxAntennaSelection = 3;
    uldci.m_hopping = false;
    uldci.m_n2Dmrs = 0;
    uldci.m_tpc = 0;
    uldci.m_cqiRequest = false;
    uldci.m_ulIndex = 0;
    uldci.m_dai = 1;
    uldci.m_freqHopping = 0;
    uldci.m_pdcchPowerOffset = 0;
    ret.m_dciList.push_back(uldci);
    uint8_t harqId = 0;
    if (m_harqOn) {
      auto itProcId = m_ulHarqCurrentProcessId.find(uldci.m_rnti);
      if (itProcId == m_ulHarqCurrentProcessId.end()) {
        NS_FATAL_ERROR("No info find in HARQ buffer for UE " << uldci.m_rnti);
      }
      harqId = (*itProcId).second;
      auto itDci = m_ulHarqProcessesDciBuffer.find(uldci.m_rnti);
      if (itDci == m_ulHarqProcessesDciBuffer.end()) {
        NS_FATAL_ERROR(
            "Unable to find RNTI entry in UL DCI HARQ buffer for RNTI "
            << uldci.m_rnti);
      }
      (*itDci).second.at(harqId) = uldci;
      auto itStat = m_ulHarqProcessesStatus.find(uldci.m_rnti);
      if (itStat == m_ulHarqProcessesStatus.end()) {
        NS_LOG_ERROR("No info find in HARQ buffer for UE (might change eNB) "
                     << uldci.m_rnti);
      }
      (*itStat).second.at(harqId) = 0;
    }

    NS_LOG_INFO(this << " UL Allocation - UE " << (*it).first << " startPRB "
                     << (uint32_t)uldci.m_rbStart << " nPRB "
                     << (uint32_t)uldci.m_rbLen << " CQI " << cqi << " MCS "
                     << (uint32_t)uldci.m_mcs << " TBsize " << uldci.m_tbSize
                     << " harqId " << (uint16_t)harqId);

    it++;
    if (it == m_ceBsrRxed.end()) {
      it = m_ceBsrRxed.begin();
    }
    if ((rbAllocated == m_cschedCellConfig.m_ulBandwidth) || (rbPerFlow == 0)) {
      m_nextRntiUl = (*it).first;
      break;
    }
  } while (((*it).first != m_nextRntiUl) && (rbPerFlow != 0));

  m_allocationMaps.insert(std::pair<uint16_t, std::vector<uint16_t>>(
      params.m_sfnSf, rbgAllocationMap));

  m_schedSapUser->SchedUlConfigInd(ret);
}

void RrFfMacScheduler::DoSchedUlNoiseInterferenceReq(
    const FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters
        &params) {
  NS_LOG_FUNCTION(this);
}

void RrFfMacScheduler::DoSchedUlSrInfoReq(
    const FfMacSchedSapProvider::SchedUlSrInfoReqParameters &params) {
  NS_LOG_FUNCTION(this);
}

void RrFfMacScheduler::DoSchedUlMacCtrlInfoReq(
    const FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters &params) {
  NS_LOG_FUNCTION(this);

  for (unsigned int i = 0; i < params.m_macCeList.size(); i++) {
    if (params.m_macCeList.at(i).m_macCeType == MacCeListElement_s::BSR) {

      uint32_t buffer = 0;
      for (uint8_t lcg = 0; lcg < 4; ++lcg) {
        uint8_t bsrId =
            params.m_macCeList.at(i).m_macCeValue.m_bufferStatus.at(lcg);
        buffer += BufferSizeLevelBsr::BsrId2BufferSize(bsrId);
      }

      uint16_t rnti = params.m_macCeList.at(i).m_rnti;
      auto it = m_ceBsrRxed.find(rnti);
      if (it == m_ceBsrRxed.end()) {
        m_ceBsrRxed.insert(std::pair<uint16_t, uint32_t>(rnti, buffer));
        NS_LOG_INFO(this << " Insert RNTI " << rnti << " queue " << buffer);
      } else {
        (*it).second = buffer;
        NS_LOG_INFO(this << " Update RNTI " << rnti << " queue " << buffer);
      }
    }
  }
}

void RrFfMacScheduler::DoSchedUlCqiInfoReq(
    const FfMacSchedSapProvider::SchedUlCqiInfoReqParameters &params) {
  NS_LOG_FUNCTION(this);

  switch (m_ulCqiFilter) {
  case FfMacScheduler::SRS_UL_CQI: {
    if (params.m_ulCqi.m_type != UlCqi_s::SRS) {
      return;
    }
  } break;
  case FfMacScheduler::PUSCH_UL_CQI: {
    if (params.m_ulCqi.m_type != UlCqi_s::PUSCH) {
      return;
    }
  } break;
  default:
    NS_FATAL_ERROR("Unknown UL CQI type");
  }
  switch (params.m_ulCqi.m_type) {
  case UlCqi_s::PUSCH: {
    auto itMap = m_allocationMaps.find(params.m_sfnSf);
    if (itMap == m_allocationMaps.end()) {
      NS_LOG_INFO(this << " Does not find info on allocation, size : "
                       << m_allocationMaps.size());
      return;
    }
    for (uint32_t i = 0; i < (*itMap).second.size(); i++) {
      double sinr =
          LteFfConverter::fpS11dot3toDouble(params.m_ulCqi.m_sinr.at(i));
      auto itCqi = m_ueCqi.find((*itMap).second.at(i));
      if (itCqi == m_ueCqi.end()) {
        std::vector<double> newCqi;
        for (uint32_t j = 0; j < m_cschedCellConfig.m_ulBandwidth; j++) {
          if (i == j) {
            newCqi.push_back(sinr);
          } else {
            newCqi.push_back(30.0);
          }
        }
        m_ueCqi.insert(std::pair<uint16_t, std::vector<double>>(
            (*itMap).second.at(i), newCqi));
        m_ueCqiTimers.insert(std::pair<uint16_t, uint32_t>(
            (*itMap).second.at(i), m_cqiTimersThreshold));
      } else {
        (*itCqi).second.at(i) = sinr;
        auto itTimers = m_ueCqiTimers.find((*itMap).second.at(i));
        (*itTimers).second = m_cqiTimersThreshold;
      }
    }
    m_allocationMaps.erase(itMap);
  } break;
  case UlCqi_s::SRS: {
    uint16_t rnti = 0;
    NS_ASSERT(!params.m_vendorSpecificList.empty());
    for (std::size_t i = 0; i < params.m_vendorSpecificList.size(); i++) {
      if (params.m_vendorSpecificList.at(i).m_type == SRS_CQI_RNTI_VSP) {
        Ptr<SrsCqiRntiVsp> vsp = DynamicCast<SrsCqiRntiVsp>(
            params.m_vendorSpecificList.at(i).m_value);
        rnti = vsp->GetRnti();
      }
    }
    auto itCqi = m_ueCqi.find(rnti);
    if (itCqi == m_ueCqi.end()) {
      std::vector<double> newCqi;
      for (uint32_t j = 0; j < m_cschedCellConfig.m_ulBandwidth; j++) {
        double sinr =
            LteFfConverter::fpS11dot3toDouble(params.m_ulCqi.m_sinr.at(j));
        newCqi.push_back(sinr);
        NS_LOG_INFO(this << " RNTI " << rnti << " new SRS-CQI for RB  " << j
                         << " value " << sinr);
      }
      m_ueCqi.insert(std::pair<uint16_t, std::vector<double>>(rnti, newCqi));
      m_ueCqiTimers.insert(
          std::pair<uint16_t, uint32_t>(rnti, m_cqiTimersThreshold));
    } else {
      for (uint32_t j = 0; j < m_cschedCellConfig.m_ulBandwidth; j++) {
        double sinr =
            LteFfConverter::fpS11dot3toDouble(params.m_ulCqi.m_sinr.at(j));
        (*itCqi).second.at(j) = sinr;
        NS_LOG_INFO(this << " RNTI " << rnti << " update SRS-CQI for RB  " << j
                         << " value " << sinr);
      }
      auto itTimers = m_ueCqiTimers.find(rnti);
      (*itTimers).second = m_cqiTimersThreshold;
    }
  } break;
  case UlCqi_s::PUCCH_1:
  case UlCqi_s::PUCCH_2:
  case UlCqi_s::PRACH: {
    NS_FATAL_ERROR("PfFfMacScheduler supports only PUSCH and SRS UL-CQIs");
  } break;
  default:
    NS_FATAL_ERROR("Unknown type of UL-CQI");
  }
}

void RrFfMacScheduler::RefreshDlCqiMaps() {
  NS_LOG_FUNCTION(this << m_p10CqiTimers.size());
  auto itP10 = m_p10CqiTimers.begin();
  while (itP10 != m_p10CqiTimers.end()) {
    NS_LOG_INFO(this << " P10-CQI for user " << (*itP10).first << " is "
                     << (uint32_t)(*itP10).second << " thr "
                     << (uint32_t)m_cqiTimersThreshold);
    if ((*itP10).second == 0) {
      auto itMap = m_p10CqiRxed.find((*itP10).first);
      NS_ASSERT_MSG(itMap != m_p10CqiRxed.end(),
                    " Does not find CQI report for user " << (*itP10).first);
      NS_LOG_INFO(this << " P10-CQI exired for user " << (*itP10).first);
      m_p10CqiRxed.erase(itMap);
      auto temp = itP10;
      itP10++;
      m_p10CqiTimers.erase(temp);
    } else {
      (*itP10).second--;
      itP10++;
    }
  }
}

void RrFfMacScheduler::RefreshUlCqiMaps() {
  auto itUl = m_ueCqiTimers.begin();
  while (itUl != m_ueCqiTimers.end()) {
    NS_LOG_INFO(this << " UL-CQI for user " << (*itUl).first << " is "
                     << (uint32_t)(*itUl).second << " thr "
                     << (uint32_t)m_cqiTimersThreshold);
    if ((*itUl).second == 0) {
      auto itMap = m_ueCqi.find((*itUl).first);
      NS_ASSERT_MSG(itMap != m_ueCqi.end(),
                    " Does not find CQI report for user " << (*itUl).first);
      NS_LOG_INFO(this << " UL-CQI exired for user " << (*itUl).first);
      (*itMap).second.clear();
      m_ueCqi.erase(itMap);
      auto temp = itUl;
      itUl++;
      m_ueCqiTimers.erase(temp);
    } else {
      (*itUl).second--;
      itUl++;
    }
  }
}

void RrFfMacScheduler::UpdateDlRlcBufferInfo(uint16_t rnti, uint8_t lcid,
                                             uint16_t size) {
  NS_LOG_FUNCTION(this);
  for (auto it = m_rlcBufferReq.begin(); it != m_rlcBufferReq.end(); it++) {
    if (((*it).m_rnti == rnti) && ((*it).m_logicalChannelIdentity == lcid)) {
      NS_LOG_INFO(this << " UE " << rnti << " LC " << (uint16_t)lcid
                       << " txqueue " << (*it).m_rlcTransmissionQueueSize
                       << " retxqueue " << (*it).m_rlcRetransmissionQueueSize
                       << " status " << (*it).m_rlcStatusPduSize << " decrease "
                       << size);
      if (((*it).m_rlcStatusPduSize > 0) &&
          (size >= (*it).m_rlcStatusPduSize)) {
        (*it).m_rlcStatusPduSize = 0;
      } else if (((*it).m_rlcRetransmissionQueueSize > 0) &&
                 (size >= (*it).m_rlcRetransmissionQueueSize)) {
        (*it).m_rlcRetransmissionQueueSize = 0;
      } else if ((*it).m_rlcTransmissionQueueSize > 0) {
        uint32_t rlcOverhead;
        if (lcid == 1) {
          rlcOverhead = 4;
        } else {
          rlcOverhead = 2;
        }
        if ((*it).m_rlcTransmissionQueueSize <= size - rlcOverhead) {
          (*it).m_rlcTransmissionQueueSize = 0;
        } else {
          (*it).m_rlcTransmissionQueueSize -= size - rlcOverhead;
        }
      }
      return;
    }
  }
}

void RrFfMacScheduler::UpdateUlRlcBufferInfo(uint16_t rnti, uint16_t size) {
  size = size - 2;
  auto it = m_ceBsrRxed.find(rnti);
  if (it != m_ceBsrRxed.end()) {
    NS_LOG_INFO(this << " Update RLC BSR UE " << rnti << " size " << size
                     << " BSR " << (*it).second);
    if ((*it).second >= size) {
      (*it).second -= size;
    } else {
      (*it).second = 0;
    }
  } else {
    NS_LOG_ERROR(this << " Does not find BSR report info of UE " << rnti);
  }
}

void RrFfMacScheduler::TransmissionModeConfigurationUpdate(uint16_t rnti,
                                                           uint8_t txMode) {
  NS_LOG_FUNCTION(this << " RNTI " << rnti << " txMode " << (uint16_t)txMode);
  FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params;
  params.m_rnti = rnti;
  params.m_transmissionMode = txMode;
  m_cschedSapUser->CschedUeConfigUpdateInd(params);
}

} // namespace ns3
