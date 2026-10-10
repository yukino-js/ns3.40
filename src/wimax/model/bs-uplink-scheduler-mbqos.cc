

#include "bs-uplink-scheduler-mbqos.h"

#include "bandwidth-manager.h"
#include "bs-link-manager.h"
#include "bs-net-device.h"
#include "burst-profile-manager.h"
#include "cid.h"
#include "connection-manager.h"
#include "service-flow-record.h"
#include "service-flow.h"
#include "ss-manager.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("UplinkSchedulerMBQoS");

NS_OBJECT_ENSURE_REGISTERED(UplinkSchedulerMBQoS);

UplinkSchedulerMBQoS::UplinkSchedulerMBQoS() {}

UplinkSchedulerMBQoS::UplinkSchedulerMBQoS(Time time)
    : m_windowInterval(time) {}

UplinkSchedulerMBQoS::~UplinkSchedulerMBQoS() {
  SetBs(nullptr);
  m_uplinkAllocations.clear();
}

TypeId UplinkSchedulerMBQoS::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::UplinkSchedulerMBQoS")

          .SetParent<UplinkScheduler>()

          .SetGroupName("Wimax")

          .AddAttribute(
              "WindowInterval", "The time to wait to reset window",
              TimeValue(Seconds(1.0)),
              MakeTimeAccessor(&UplinkSchedulerMBQoS::m_windowInterval),
              MakeTimeChecker());
  return tid;
}

void UplinkSchedulerMBQoS::InitOnce() { UplinkSchedWindowTimer(); }

std::list<OfdmUlMapIe> UplinkSchedulerMBQoS::GetUplinkAllocations() const {
  return m_uplinkAllocations;
}

void UplinkSchedulerMBQoS::GetChannelDescriptorsToUpdate(bool &updateDcd,
                                                         bool &updateUcd,
                                                         bool &sendDcd,
                                                         bool &sendUcd) {

  uint32_t randNr = rand();
  if (randNr % 5 == 0 || GetBs()->GetNrDcdSent() == 0) {
    sendDcd = true;
  }

  randNr = rand();
  if (randNr % 5 == 0 || GetBs()->GetNrUcdSent() == 0) {
    sendUcd = true;
  }

  if (!sendDcd) {
    randNr = rand();
    if (randNr % 4 == 0) {
      sendDcd = true;
    }
  }

  if (!sendUcd) {
    randNr = rand();
    if (randNr % 4 == 0) {
      sendUcd = true;
    }
  }

  Time timeSinceLastDcd = Simulator::Now() - GetDcdTimeStamp();
  Time timeSinceLastUcd = Simulator::Now() - GetUcdTimeStamp();

  if (timeSinceLastDcd > GetBs()->GetDcdInterval()) {
    sendDcd = true;
    SetDcdTimeStamp(Simulator::Now());
  }

  if (timeSinceLastUcd > GetBs()->GetUcdInterval()) {
    sendUcd = true;
    SetUcdTimeStamp(Simulator::Now());
  }
}

uint32_t UplinkSchedulerMBQoS::CalculateAllocationStartTime() {
  return GetBs()->GetNrDlSymbols() * GetBs()->GetPhy()->GetPsPerSymbol() +
         GetBs()->GetTtg();
}

void UplinkSchedulerMBQoS::AddUplinkAllocation(OfdmUlMapIe &ulMapIe,
                                               const uint32_t &allocationSize,
                                               uint32_t &symbolsToAllocation,
                                               uint32_t &availableSymbols) {
  ulMapIe.SetDuration(allocationSize);
  ulMapIe.SetStartTime(symbolsToAllocation);
  m_uplinkAllocations.push_back(ulMapIe);
  symbolsToAllocation += allocationSize;
  availableSymbols -= allocationSize;
}

void UplinkSchedulerMBQoS::UplinkSchedWindowTimer() {
  NS_LOG(LOG_DEBUG, "Window Reset at " << (Simulator::Now()).As(Time::S));

  uint32_t min_bw = 0;

  if (!GetBs()->GetSSManager()) {
    Simulator::Schedule(m_windowInterval,
                        &UplinkSchedulerMBQoS::UplinkSchedWindowTimer, this);
    return;
  }

  std::vector<SSRecord *> *ssRecords = GetBs()->GetSSManager()->GetSSRecords();

  for (auto iter = ssRecords->begin(); iter != ssRecords->end(); ++iter) {
    SSRecord *ssRecord = *iter;
    std::vector<ServiceFlow *> serviceFlows =
        ssRecord->GetServiceFlows(ServiceFlow::SF_TYPE_ALL);

    for (auto iter2 = serviceFlows.begin(); iter2 != serviceFlows.end();
         ++iter2) {
      ServiceFlow *serviceFlow = *iter2;
      if ((serviceFlow->GetSchedulingType() == ServiceFlow::SF_TYPE_RTPS) ||
          (serviceFlow->GetSchedulingType() == ServiceFlow::SF_TYPE_NRTPS)) {
        min_bw = (int)ceil(serviceFlow->GetMinReservedTrafficRate());

        if ((serviceFlow->GetRecord()->GetBacklogged() > 0) &&
            (serviceFlow->GetRecord()->GetBwSinceLastExpiry() < min_bw)) {
          serviceFlow->GetRecord()->UpdateBwSinceLastExpiry(-min_bw);

          if (serviceFlow->GetRecord()->GetBacklogged() <
              (serviceFlow->GetRecord()->GetBwSinceLastExpiry())) {
            serviceFlow->GetRecord()->SetBwSinceLastExpiry(
                -serviceFlow->GetRecord()->GetBacklogged());
          }
        } else {
          serviceFlow->GetRecord()->SetBwSinceLastExpiry(0);
        }
      }
    }
  }

  Simulator::Schedule(m_windowInterval,
                      &UplinkSchedulerMBQoS::UplinkSchedWindowTimer, this);
}

void UplinkSchedulerMBQoS::Schedule() {
  m_uplinkAllocations.clear();
  SetIsIrIntrvlAllocated(false);
  SetIsInvIrIntrvlAllocated(false);
  bool allocationForDsa = false;

  uint32_t symbolsToAllocation = 0;
  uint32_t allocationSize = 0;
  uint32_t availableSymbols = GetBs()->GetNrUlSymbols();

  AllocateInitialRangingInterval(symbolsToAllocation, availableSymbols);

  std::vector<SSRecord *> *ssRecords = GetBs()->GetSSManager()->GetSSRecords();
  for (auto iter = ssRecords->begin(); iter != ssRecords->end(); ++iter) {
    SSRecord *ssRecord = *iter;

    if (ssRecord->GetIsBroadcastSS()) {
      continue;
    }
    Cid cid = ssRecord->GetBasicCid();
    OfdmUlMapIe ulMapIe;
    ulMapIe.SetCid(cid);

    if (ssRecord->GetPollForRanging() &&
        ssRecord->GetRangingStatus() ==
            WimaxNetDevice::RANGING_STATUS_CONTINUE) {
      ulMapIe.SetUiuc(OfdmUlBurstProfile::UIUC_INITIAL_RANGING);
      allocationSize = GetBs()->GetRangReqOppSize();
      SetIsInvIrIntrvlAllocated(true);

      if (availableSymbols >= allocationSize) {
        AddUplinkAllocation(ulMapIe, allocationSize, symbolsToAllocation,
                            availableSymbols);
      } else {
        break;
      }
    } else {
      WimaxPhy::ModulationType modulationType = ssRecord->GetModulationType();

      ulMapIe.SetUiuc(GetBs()->GetBurstProfileManager()->GetBurstProfile(
          modulationType, WimaxNetDevice::DIRECTION_UPLINK));

      if (ssRecord->GetRangingStatus() ==
              WimaxNetDevice::RANGING_STATUS_SUCCESS &&
          !ssRecord->GetAreServiceFlowsAllocated()) {
        if (!allocationForDsa) {
          allocationSize =
              GetBs()->GetPhy()->GetNrSymbols(sizeof(DsaReq), modulationType);

          if (availableSymbols >= allocationSize) {
            AddUplinkAllocation(ulMapIe, allocationSize, symbolsToAllocation,
                                availableSymbols);
            allocationForDsa = true;
          } else {
            break;
          }
        }
      } else {

        if (availableSymbols) {
          if (ssRecord->GetHasServiceFlowUgs()) {
            NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S)
                               << " offering be unicast polling");
            Time frame_duration = GetBs()->GetPhy()->GetFrameDuration();
            Time timestamp =
                (*(ssRecord->GetServiceFlows(ServiceFlow::SF_TYPE_UGS).begin()))
                    ->GetRecord()
                    ->GetLastGrantTime() +
                MilliSeconds(
                    (*(ssRecord->GetServiceFlows(ServiceFlow::SF_TYPE_UGS)
                           .begin()))
                        ->GetUnsolicitedGrantInterval());

            int64_t frame =
                ((timestamp - Simulator::Now()) / frame_duration).GetHigh();

            if (frame <= 1) {
              ServiceUnsolicitedGrants(ssRecord, ServiceFlow::SF_TYPE_UGS,
                                       ulMapIe, modulationType,
                                       symbolsToAllocation, availableSymbols);
            }
          }

          if (ssRecord->GetHasServiceFlowRtps()) {
            NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S)
                               << " offering rtps unicast polling");
            Ptr<UlJob> jobRTPSPoll = CreateUlJob(
                ssRecord, ServiceFlow::SF_TYPE_RTPS, UNICAST_POLLING);
            EnqueueJob(UlJob::HIGH, jobRTPSPoll);
          }

          if (ssRecord->GetHasServiceFlowNrtps()) {
            NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S)
                               << " offering nrtps unicast polling");
            Ptr<UlJob> jobNRTPSPoll = CreateUlJob(
                ssRecord, ServiceFlow::SF_TYPE_NRTPS, UNICAST_POLLING);
            EnqueueJob(UlJob::HIGH, jobNRTPSPoll);
          }

          if (ssRecord->GetHasServiceFlowBe()) {
            NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S)
                               << " offering be unicast polling");
            Ptr<UlJob> jobBEPoll =
                CreateUlJob(ssRecord, ServiceFlow::SF_TYPE_BE, UNICAST_POLLING);
            EnqueueJob(UlJob::HIGH, jobBEPoll);
          }
        }
      }
    }
  }
  NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S) << " high queue has "
                     << m_uplinkJobs_high.size() << " jobs - after sched");

  uint32_t availableSymbolsAux = availableSymbols;
  uint32_t symbolsUsed = 0;

  symbolsUsed += CountSymbolsQueue(m_uplinkJobs_high);
  availableSymbolsAux -= symbolsUsed;

  CheckDeadline(availableSymbolsAux);

  CheckMinimumBandwidth(availableSymbolsAux);

  NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S) << " high queue has "
                     << m_uplinkJobs_high.size() << " jobs");
  while ((availableSymbols) && (!m_uplinkJobs_high.empty())) {
    Ptr<UlJob> job = m_uplinkJobs_high.front();
    OfdmUlMapIe ulMapIe;
    SSRecord *ssRecord = job->GetSsRecord();
    ServiceFlow::SchedulingType schedulingType = job->GetSchedulingType();

    Cid cid = ssRecord->GetBasicCid();
    ulMapIe.SetCid(cid);
    WimaxPhy::ModulationType modulationType = ssRecord->GetModulationType();
    ulMapIe.SetUiuc(GetBs()->GetBurstProfileManager()->GetBurstProfile(
        modulationType, WimaxNetDevice::DIRECTION_UPLINK));

    ReqType reqType = job->GetType();

    if (reqType == UNICAST_POLLING) {
      ServiceUnsolicitedGrants(ssRecord, schedulingType, ulMapIe,
                               modulationType, symbolsToAllocation,
                               availableSymbols);
    } else if (reqType == DATA) {
      ServiceFlow *serviceFlow = job->GetServiceFlow();
      uint32_t allocSizeBytes = job->GetSize();
      ServiceBandwidthRequestsBytes(serviceFlow, schedulingType, ulMapIe,
                                    modulationType, symbolsToAllocation,
                                    availableSymbols, allocSizeBytes);
    }
    m_uplinkJobs_high.pop_front();
  }

  NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S) << " interqueue has "
                     << m_uplinkJobs_inter.size() << " jobs");
  while ((availableSymbols) && (!m_uplinkJobs_inter.empty())) {
    NS_LOG_DEBUG("At " << Simulator::Now().As(Time::S)
                       << " Scheduling interqueue");
    Ptr<UlJob> job = m_uplinkJobs_inter.front();
    OfdmUlMapIe ulMapIe;
    SSRecord *ssRecord = job->GetSsRecord();
    ServiceFlow::SchedulingType schedulingType = job->GetSchedulingType();

    Cid cid = ssRecord->GetBasicCid();
    ulMapIe.SetCid(cid);
    WimaxPhy::ModulationType modulationType = ssRecord->GetModulationType();
    ulMapIe.SetUiuc(GetBs()->GetBurstProfileManager()->GetBurstProfile(
        modulationType, WimaxNetDevice::DIRECTION_UPLINK));

    ReqType reqType = job->GetType();

    if (reqType == DATA) {
      ServiceBandwidthRequests(ssRecord, schedulingType, ulMapIe,
                               modulationType, symbolsToAllocation,
                               availableSymbols);
    } else {
      NS_FATAL_ERROR(
          "Intermediate priority queue only should enqueue data packets.");
    }
    m_uplinkJobs_inter.pop_front();
  }

  while ((availableSymbols) && (!m_uplinkJobs_low.empty())) {
    Ptr<UlJob> job = m_uplinkJobs_low.front();
    OfdmUlMapIe ulMapIe;
    SSRecord *ssRecord = job->GetSsRecord();
    ServiceFlow::SchedulingType schedulingType = job->GetSchedulingType();

    Cid cid = ssRecord->GetBasicCid();
    ulMapIe.SetCid(cid);
    WimaxPhy::ModulationType modulationType = ssRecord->GetModulationType();
    ulMapIe.SetUiuc(GetBs()->GetBurstProfileManager()->GetBurstProfile(
        modulationType, WimaxNetDevice::DIRECTION_UPLINK));

    ReqType reqType = job->GetType();

    if (reqType == DATA) {
      ServiceBandwidthRequests(ssRecord, schedulingType, ulMapIe,
                               modulationType, symbolsToAllocation,
                               availableSymbols);
    } else {
      NS_FATAL_ERROR("Low priority queue only should enqueue data packets.");
    }
    m_uplinkJobs_low.pop_front();
  }

  OfdmUlMapIe ulMapIeEnd;
  ulMapIeEnd.SetCid(Cid(0));
  ulMapIeEnd.SetStartTime(symbolsToAllocation);
  ulMapIeEnd.SetUiuc(OfdmUlBurstProfile::UIUC_END_OF_MAP);
  ulMapIeEnd.SetDuration(0);
  m_uplinkAllocations.push_back(ulMapIeEnd);

  GetBs()->GetBandwidthManager()->SetSubframeRatio();
}

bool UplinkSchedulerMBQoS::ServiceBandwidthRequestsBytes(
    ServiceFlow *serviceFlow, ServiceFlow::SchedulingType schedulingType,
    OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
    uint32_t &symbolsToAllocation, uint32_t &availableSymbols,
    uint32_t allocationSizeBytes) {
  uint32_t allocSizeBytes = allocationSizeBytes;
  uint32_t allocSizeSymbols = 0;

  ServiceFlowRecord *record = serviceFlow->GetRecord();

  uint32_t requiredBandwidth = record->GetRequestedBandwidth();

  if (requiredBandwidth > 0) {
    allocSizeSymbols =
        GetBs()->GetPhy()->GetNrSymbols(allocSizeBytes, modulationType);

    if (availableSymbols < allocSizeSymbols) {
      allocSizeSymbols = availableSymbols;
    }

    if (availableSymbols >= allocSizeSymbols) {
      NS_LOG_DEBUG(
          "At " << Simulator::Now().As(Time::S) << " BS uplink scheduler, "
                << serviceFlow->GetSchedulingTypeStr()
                << " allocation, size: " << allocSizeSymbols << " symbols"
                << ", CID: " << serviceFlow->GetConnection()->GetCid()
                << ", SFID: " << serviceFlow->GetSfid()
                << ", bw requested: " << record->GetRequestedBandwidth()
                << ", bw granted: " << allocSizeBytes << std::endl);

      record->UpdateGrantedBandwidthTemp(allocSizeBytes);
      record->UpdateGrantedBandwidth(allocSizeBytes);
      record->UpdateRequestedBandwidth(-allocSizeBytes);

      record->UpdateBwSinceLastExpiry(allocSizeBytes);

      AddUplinkAllocation(ulMapIe, allocSizeSymbols, symbolsToAllocation,
                          availableSymbols);
    } else {
      return false;
    }
  }
  return true;
}

uint32_t UplinkSchedulerMBQoS::CountSymbolsQueue(std::list<Ptr<UlJob>> jobs) {
  uint32_t symbols = 0;
  for (auto iter = jobs.begin(); iter != jobs.end(); ++iter) {
    Ptr<UlJob> job = *iter;

    symbols += CountSymbolsJobs(job);
  }
  return symbols;
}

Ptr<UlJob>
UplinkSchedulerMBQoS::CreateUlJob(SSRecord *ssRecord,
                                  ServiceFlow::SchedulingType schedType,
                                  ReqType reqType) {
  Ptr<UlJob> job = CreateObject<UlJob>();
  job->SetSsRecord(ssRecord);
  job->SetSchedulingType(schedType);
  job->SetServiceFlow(*(ssRecord->GetServiceFlows(schedType).begin()));
  job->SetType(reqType);
  return job;
}

uint32_t UplinkSchedulerMBQoS::CountSymbolsJobs(Ptr<UlJob> job) {
  SSRecord *ssRecord = job->GetSsRecord();
  ServiceFlow *serviceFlow = job->GetServiceFlow();
  uint32_t allocationSize = 0;

  if (job->GetType() == UNICAST_POLLING) {
    Time currentTime = Simulator::Now();
    allocationSize = 0;
    if ((currentTime - serviceFlow->GetRecord()->GetGrantTimeStamp())
            .GetMilliSeconds() >=
        serviceFlow->GetUnsolicitedPollingInterval()) {
      allocationSize = GetBs()->GetBwReqOppSize();
    }
  } else {
    uint16_t sduSize = serviceFlow->GetSduSize();
    ServiceFlowRecord *record = serviceFlow->GetRecord();
    uint32_t requiredBandwidth =
        record->GetRequestedBandwidth() - record->GetGrantedBandwidth();
    if (requiredBandwidth > 0) {
      WimaxPhy::ModulationType modulationType = ssRecord->GetModulationType();
      if (sduSize > 0) {
        allocationSize =
            GetBs()->GetPhy()->GetNrSymbols(sduSize, modulationType);
      } else {
        allocationSize =
            GetBs()->GetPhy()->GetNrSymbols(requiredBandwidth, modulationType);
      }
    }
  }
  return allocationSize;
}

void UplinkSchedulerMBQoS::EnqueueJob(UlJob::JobPriority priority,
                                      Ptr<UlJob> job) {
  switch (priority) {
  case UlJob::HIGH:
    m_uplinkJobs_high.push_back(job);
    break;
  case UlJob::INTERMEDIATE:
    m_uplinkJobs_inter.push_back(job);
    break;
  case UlJob::LOW:
    m_uplinkJobs_low.push_back(job);
  }
}

Ptr<UlJob> UplinkSchedulerMBQoS::DequeueJob(UlJob::JobPriority priority) {
  Ptr<UlJob> job_front;
  switch (priority) {
  case UlJob::HIGH:
    job_front = m_uplinkJobs_high.front();
    m_uplinkJobs_high.pop_front();
    break;
  case UlJob::INTERMEDIATE:
    job_front = m_uplinkJobs_inter.front();
    m_uplinkJobs_inter.pop_front();
    break;
  case UlJob::LOW:
    job_front = m_uplinkJobs_low.front();
    m_uplinkJobs_low.pop_front();
  }
  return job_front;
}

void UplinkSchedulerMBQoS::CheckDeadline(uint32_t &availableSymbols) {
  if (!m_uplinkJobs_inter.empty()) {
    auto iter = m_uplinkJobs_inter.begin();

    while (iter != m_uplinkJobs_inter.end() && availableSymbols) {
      Ptr<UlJob> job = *iter;

      if (job->GetSchedulingType() == ServiceFlow::SF_TYPE_RTPS) {
        Time deadline = job->GetDeadline();
        Time frame_duration = GetBs()->GetPhy()->GetFrameDuration();

        int64_t frame =
            ((deadline - Simulator::Now()) / frame_duration).GetHigh();

        NS_LOG_DEBUG(
            "At " << Simulator::Now().As(Time::S) << " reserved traffic rate: "
                  << job->GetServiceFlow()->GetMinReservedTrafficRate()
                  << " deadline: " << job->GetDeadline().As(Time::S)
                  << " frame start: " << GetBs()->m_frameStartTime.As(Time::S)
                  << " frame duration: " << frame_duration);

        if (frame >= 3) {
          if (availableSymbols) {
            uint32_t availableBytes = GetBs()->GetPhy()->GetNrBytes(
                availableSymbols, job->GetSsRecord()->GetModulationType());
            uint32_t allocationSize = job->GetSize();
            if (allocationSize > availableBytes) {
              allocationSize = availableBytes;
            }

            if (allocationSize == 0) {
              continue;
            }

            uint32_t symbolsToAllocate = GetBs()->GetPhy()->GetNrSymbols(
                allocationSize, job->GetSsRecord()->GetModulationType());
            if (symbolsToAllocate > availableSymbols) {
              symbolsToAllocate = availableSymbols;
              allocationSize = GetBs()->GetPhy()->GetNrBytes(
                  symbolsToAllocate, job->GetSsRecord()->GetModulationType());
            }

            job->SetSize(job->GetSize() - allocationSize);

            Ptr<UlJob> newJob = CreateObject<UlJob>();
            newJob->SetSsRecord(job->GetSsRecord());
            newJob->SetServiceFlow(job->GetServiceFlow());
            newJob->SetSize(allocationSize);
            newJob->SetDeadline(job->GetDeadline());
            newJob->SetReleaseTime(job->GetReleaseTime());
            newJob->SetSchedulingType(job->GetSchedulingType());
            newJob->SetPeriod(job->GetPeriod());
            newJob->SetType(job->GetType());

            EnqueueJob(UlJob::HIGH, newJob);

            iter++;
            if ((job->GetSize() - allocationSize) == 0) {
              m_uplinkJobs_inter.remove(job);
            }
          }
        } else {
          iter++;
        }
      } else {
        iter++;
      }
    }
  }
}

void UplinkSchedulerMBQoS::CheckMinimumBandwidth(uint32_t &availableSymbols) {
  std::list<Ptr<PriorityUlJob>> priorityUlJobs;

  std::vector<SSRecord *> *ssRecords = GetBs()->GetSSManager()->GetSSRecords();
  for (auto iter = ssRecords->begin(); iter != ssRecords->end(); ++iter) {
    SSRecord *ssRecord = *iter;
    std::vector<ServiceFlow *> serviceFlows =
        ssRecord->GetServiceFlows(ServiceFlow::SF_TYPE_ALL);
    for (auto iter2 = serviceFlows.begin(); iter2 != serviceFlows.end();
         ++iter2) {
      ServiceFlow *serviceFlow = *iter2;
      if (serviceFlow->GetSchedulingType() == ServiceFlow::SF_TYPE_RTPS ||
          serviceFlow->GetSchedulingType() == ServiceFlow::SF_TYPE_NRTPS) {
        serviceFlow->GetRecord()->SetBackloggedTemp(
            serviceFlow->GetRecord()->GetBacklogged());
        serviceFlow->GetRecord()->SetGrantedBandwidthTemp(
            serviceFlow->GetRecord()->GetBwSinceLastExpiry());
      }
    }
  }

  for (auto iter = m_uplinkJobs_inter.begin(); iter != m_uplinkJobs_inter.end();
       ++iter) {
    Ptr<UlJob> job = *iter;
    ServiceFlow *serviceFlow = job->GetServiceFlow();
    if ((job->GetSchedulingType() == ServiceFlow::SF_TYPE_RTPS ||
         job->GetSchedulingType() == ServiceFlow::SF_TYPE_NRTPS) &&
        (serviceFlow->GetRecord()->GetBacklogged() > 0)) {
      uint32_t minReservedTrafficRate =
          serviceFlow->GetMinReservedTrafficRate();
      uint32_t grantedBandwidth =
          serviceFlow->GetRecord()->GetBwSinceLastExpiry();

      Ptr<PriorityUlJob> priorityUlJob = CreateObject<PriorityUlJob>();
      priorityUlJob->SetUlJob(job);
      if (minReservedTrafficRate <= grantedBandwidth) {
        priorityUlJob->SetPriority(-10000);
      } else {
        uint32_t allocationSize =
            serviceFlow->GetRecord()->GetRequestedBandwidth() -
            serviceFlow->GetRecord()->GetGrantedBandwidth();
        uint32_t sduSize = serviceFlow->GetSduSize();

        if (allocationSize > 0) {
          if (sduSize > 0) {
            allocationSize = sduSize;
          }
        }
        int priority = serviceFlow->GetRecord()->GetBackloggedTemp() -
                       (serviceFlow->GetRecord()->GetGrantedBandwidthTemp() -
                        minReservedTrafficRate);
        priorityUlJob->SetPriority(priority);
        serviceFlow->GetRecord()->SetGrantedBandwidthTemp(
            serviceFlow->GetRecord()->GetGrantedBandwidthTemp() +
            allocationSize);
        serviceFlow->GetRecord()->SetBackloggedTemp(
            serviceFlow->GetRecord()->GetBackloggedTemp() - allocationSize);
      }

      priorityUlJobs.push_back(priorityUlJob);
    }
  }

  priorityUlJobs.sort(SortProcessPtr());

  for (auto iter = priorityUlJobs.begin(); iter != priorityUlJobs.end();
       ++iter) {
    Ptr<PriorityUlJob> priorityUlJob = *iter;
    Ptr<UlJob> job_priority = priorityUlJob->GetUlJob();
    Ptr<UlJob> job = job_priority;
    if (availableSymbols) {
      availableSymbols -= CountSymbolsJobs(job);
      m_uplinkJobs_inter.remove(job);
      EnqueueJob(UlJob::HIGH, job);
    }
  }
}

void UplinkSchedulerMBQoS::ServiceUnsolicitedGrants(
    const SSRecord *ssRecord, ServiceFlow::SchedulingType schedulingType,
    OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
    uint32_t &symbolsToAllocation, uint32_t &availableSymbols) {
  uint32_t allocationSize = 0;
  uint8_t uiuc = ulMapIe.GetUiuc();
  std::vector<ServiceFlow *> serviceFlows =
      ssRecord->GetServiceFlows(schedulingType);

  for (auto iter = serviceFlows.begin(); iter != serviceFlows.end(); ++iter) {
    ServiceFlow *serviceFlow = *iter;

    allocationSize = GetBs()->GetBandwidthManager()->CalculateAllocationSize(
        ssRecord, serviceFlow);

    if (availableSymbols < allocationSize) {
      break;
    }

    if (allocationSize > 0) {
      ulMapIe.SetStartTime(symbolsToAllocation);
      if (serviceFlow->GetSchedulingType() != ServiceFlow::SF_TYPE_UGS) {
        ulMapIe.SetUiuc(OfdmUlBurstProfile::UIUC_REQ_REGION_FULL);
      }
    } else {
      continue;
    }

    if (serviceFlow->GetSchedulingType() == ServiceFlow::SF_TYPE_UGS) {
      NS_LOG_DEBUG("BS uplink scheduler, UGS allocation, size: "
                   << allocationSize << " symbols");
    } else {
      NS_LOG_DEBUG("BS uplink scheduler, "
                   << serviceFlow->GetSchedulingTypeStr()
                   << " unicast poll, size: " << allocationSize << " symbols"
                   << ", modulation: BPSK 1/2");
    }

    NS_LOG_DEBUG(", CID: " << serviceFlow->GetConnection()->GetCid()
                           << ", SFID: " << serviceFlow->GetSfid());

    serviceFlow->GetRecord()->SetLastGrantTime(Simulator::Now());
    AddUplinkAllocation(ulMapIe, allocationSize, symbolsToAllocation,
                        availableSymbols);
    ulMapIe.SetUiuc(uiuc);
  }
}

void UplinkSchedulerMBQoS::ServiceBandwidthRequests(
    const SSRecord *ssRecord, ServiceFlow::SchedulingType schedulingType,
    OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
    uint32_t &symbolsToAllocation, uint32_t &availableSymbols) {
  std::vector<ServiceFlow *> serviceFlows =
      ssRecord->GetServiceFlows(schedulingType);

  for (auto iter = serviceFlows.begin(); iter != serviceFlows.end(); ++iter) {
    if (!ServiceBandwidthRequests(*iter, schedulingType, ulMapIe,
                                  modulationType, symbolsToAllocation,
                                  availableSymbols)) {
      break;
    }
  }
}

bool UplinkSchedulerMBQoS::ServiceBandwidthRequests(
    ServiceFlow *serviceFlow, ServiceFlow::SchedulingType schedulingType,
    OfdmUlMapIe &ulMapIe, const WimaxPhy::ModulationType modulationType,
    uint32_t &symbolsToAllocation, uint32_t &availableSymbols) {
  uint32_t allocSizeBytes = 0;
  uint32_t allocSizeSymbols = 0;
  uint16_t sduSize = 0;

  ServiceFlowRecord *record = serviceFlow->GetRecord();
  sduSize = serviceFlow->GetSduSize();

  uint32_t requiredBandwidth =
      record->GetRequestedBandwidth() - record->GetGrantedBandwidth();
  if (requiredBandwidth > 0) {
    if (sduSize > 0) {
      allocSizeBytes = sduSize;
      allocSizeSymbols =
          GetBs()->GetPhy()->GetNrSymbols(sduSize, modulationType);
    } else {
      allocSizeBytes = requiredBandwidth;
      allocSizeSymbols =
          GetBs()->GetPhy()->GetNrSymbols(requiredBandwidth, modulationType);
    }

    if (availableSymbols >= allocSizeSymbols) {
      NS_LOG_DEBUG("BS uplink scheduler, "
                   << serviceFlow->GetSchedulingTypeStr()
                   << " allocation, size: " << allocSizeSymbols << " symbols"
                   << ", CID: " << serviceFlow->GetConnection()->GetCid()
                   << ", SFID: " << serviceFlow->GetSfid()
                   << ", bw requested: " << record->GetRequestedBandwidth()
                   << ", bw granted: " << record->GetGrantedBandwidth());

      record->UpdateGrantedBandwidth(allocSizeBytes);

      record->SetBwSinceLastExpiry(allocSizeBytes);

      if (serviceFlow->GetRecord()->GetBacklogged() < allocSizeBytes) {
        serviceFlow->GetRecord()->SetBacklogged(0);
      } else {
        serviceFlow->GetRecord()->IncreaseBacklogged(-allocSizeBytes);
      }
      serviceFlow->GetRecord()->SetLastGrantTime(Simulator::Now());

      AddUplinkAllocation(ulMapIe, allocSizeSymbols, symbolsToAllocation,
                          availableSymbols);
    } else {
      return false;
    }
  }
  return true;
}

void UplinkSchedulerMBQoS::AllocateInitialRangingInterval(
    uint32_t &symbolsToAllocation, uint32_t &availableSymbols) {
  Time ssUlStartTime = Seconds(CalculateAllocationStartTime() *
                               GetBs()->GetPsDuration().GetSeconds());
  SetNrIrOppsAllocated(
      GetBs()->GetLinkManager()->CalculateRangingOppsToAllocate());
  uint32_t allocationSize =
      GetNrIrOppsAllocated() * GetBs()->GetRangReqOppSize();
  Time timeSinceLastIrInterval = Simulator::Now() - GetTimeStampIrInterval();

  if (timeSinceLastIrInterval + GetBs()->GetPhy()->GetFrameDuration() >
          GetBs()->GetInitialRangingInterval() &&
      availableSymbols >= allocationSize) {
    SetIsIrIntrvlAllocated(true);
    OfdmUlMapIe ulMapIeIr;
    ulMapIeIr.SetCid((GetBs()->GetBroadcastConnection())->GetCid());
    ulMapIeIr.SetStartTime(symbolsToAllocation);
    ulMapIeIr.SetUiuc(OfdmUlBurstProfile::UIUC_INITIAL_RANGING);

    NS_LOG_DEBUG("BS uplink scheduler, initial ranging allocation, size: "
                 << allocationSize << " symbols"
                 << ", modulation: BPSK 1/2");

    for (uint8_t i = 0; i < GetNrIrOppsAllocated(); i++) {
      GetBs()->MarkRangingOppStart(
          ssUlStartTime +
          Seconds(symbolsToAllocation *
                  GetBs()->GetSymbolDuration().GetSeconds()) +
          Seconds(i * GetBs()->GetRangReqOppSize() *
                  GetBs()->GetSymbolDuration().GetSeconds()));
    }

    AddUplinkAllocation(ulMapIeIr, allocationSize, symbolsToAllocation,
                        availableSymbols);
    SetTimeStampIrInterval(Simulator::Now());
  }
}

void UplinkSchedulerMBQoS::SetupServiceFlow(SSRecord *ssRecord,
                                            ServiceFlow *serviceFlow) {
  uint8_t delayNrFrames = 1;
  uint32_t bitsPerSecond = serviceFlow->GetMinReservedTrafficRate();
  WimaxPhy::ModulationType modulation;
  uint32_t bytesPerFrame = (uint32_t((double)(bitsPerSecond)*GetBs()
                                         ->GetPhy()
                                         ->GetFrameDuration()
                                         .GetSeconds())) /
                           8;
  uint32_t frameDurationMSec =
      GetBs()->GetPhy()->GetFrameDuration().GetMilliSeconds();

  switch (serviceFlow->GetSchedulingType()) {
  case ServiceFlow::SF_TYPE_UGS: {
    if (serviceFlow->GetIsMulticast()) {
      modulation = serviceFlow->GetModulation();
    } else {
      modulation = ssRecord->GetModulationType();
    }
    uint32_t grantSize =
        GetBs()->GetPhy()->GetNrSymbols(bytesPerFrame, modulation);
    serviceFlow->GetRecord()->SetGrantSize(grantSize);

    uint32_t toleratedJitter = serviceFlow->GetToleratedJitter();

    if (toleratedJitter > frameDurationMSec) {
      delayNrFrames = (uint8_t)(toleratedJitter / frameDurationMSec);
    }

    uint16_t interval = delayNrFrames * frameDurationMSec;
    serviceFlow->SetUnsolicitedGrantInterval(interval);
  } break;
  case ServiceFlow::SF_TYPE_RTPS: {
    serviceFlow->SetUnsolicitedPollingInterval(20);
  } break;
  case ServiceFlow::SF_TYPE_NRTPS: {
    uint16_t interval = 1000;
    serviceFlow->SetUnsolicitedPollingInterval(interval);
  } break;
  case ServiceFlow::SF_TYPE_BE: {
  } break;
  default:
    NS_FATAL_ERROR("Invalid scheduling type");
  }
}

uint32_t UplinkSchedulerMBQoS::GetPendingSize(ServiceFlow *serviceFlow) {
  uint32_t size = 0;
  std::list<Ptr<PriorityUlJob>> priorityUlJobs;

  for (auto iter = m_uplinkJobs_inter.begin(); iter != m_uplinkJobs_inter.end();
       ++iter) {
    Ptr<UlJob> job = *iter;

    ServiceFlow *serviceFlowJob = job->GetServiceFlow();

    if (serviceFlowJob == serviceFlow) {
      size += job->GetSize();
    }
  }
  return size;
}

void UplinkSchedulerMBQoS::ProcessBandwidthRequest(
    const BandwidthRequestHeader &bwRequestHdr) {
  Ptr<UlJob> job = CreateObject<UlJob>();
  Ptr<WimaxConnection> connection =
      GetBs()->GetConnectionManager()->GetConnection(bwRequestHdr.GetCid());
  SSRecord *ssRecord =
      GetBs()->GetSSManager()->GetSSRecord(connection->GetCid());
  ServiceFlow *serviceFlow = connection->GetServiceFlow();

  uint32_t size = bwRequestHdr.GetBr();
  uint32_t pendingSize = GetPendingSize(serviceFlow);

  if (size > pendingSize) {
    size -= pendingSize;
  } else {
    size = 0;
  }

  if (size == 0) {
    return;
  }

  Time deadline = DetermineDeadline(serviceFlow);
  Time currentTime = Simulator::Now();
  const Time &period = deadline;

  NS_LOG_DEBUG(
      "At " << Simulator::Now().As(Time::S)
            << " at BS uplink scheduler, processing bandwidth request from."
            << ssRecord->GetMacAddress() << " and sf "
            << serviceFlow->GetSchedulingType() << " with deadline in "
            << deadline.As(Time::S) << " and size " << size << " aggreg size "
            << bwRequestHdr.GetBr());

  job->SetSsRecord(ssRecord);
  job->SetServiceFlow(serviceFlow);
  job->SetSize(size);
  job->SetDeadline(deadline);
  job->SetReleaseTime(currentTime);
  job->SetSchedulingType(serviceFlow->GetSchedulingType());
  job->SetPeriod(period);
  job->SetType(DATA);

  switch (serviceFlow->GetSchedulingType()) {
  case ServiceFlow::SF_TYPE_RTPS:
    EnqueueJob(UlJob::INTERMEDIATE, job);
    break;
  case ServiceFlow::SF_TYPE_NRTPS:
    EnqueueJob(UlJob::INTERMEDIATE, job);
    break;
  case ServiceFlow::SF_TYPE_BE:
    EnqueueJob(UlJob::LOW, job);
    break;
  default:
    EnqueueJob(UlJob::LOW, job);
    break;
  }
}

Time UplinkSchedulerMBQoS::DetermineDeadline(ServiceFlow *serviceFlow) {
  uint32_t latency = serviceFlow->GetMaximumLatency();
  Time lastGrantTime = serviceFlow->GetRecord()->GetLastGrantTime();
  Time deadline = MilliSeconds(latency) + lastGrantTime;
  return deadline;
}

void UplinkSchedulerMBQoS::OnSetRequestedBandwidth(ServiceFlowRecord *sfr) {}

} // namespace ns3
