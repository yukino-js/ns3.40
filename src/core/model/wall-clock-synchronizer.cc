
#include "wall-clock-synchronizer.h"

#include "log.h"

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <mutex>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WallClockSynchronizer");

NS_OBJECT_ENSURE_REGISTERED(WallClockSynchronizer);

TypeId WallClockSynchronizer::GetTypeId() {
  static TypeId tid = TypeId("ns3::WallClockSynchronizer")
                          .SetParent<Synchronizer>()
                          .SetGroupName("Core");
  return tid;
}

WallClockSynchronizer::WallClockSynchronizer() {
  NS_LOG_FUNCTION(this);

  m_jiffy = std::chrono::system_clock::period::num * std::nano::den /
            std::chrono::system_clock::period::den;
  NS_LOG_INFO("Jiffy is " << m_jiffy << " ns");
}

WallClockSynchronizer::~WallClockSynchronizer() { NS_LOG_FUNCTION(this); }

bool WallClockSynchronizer::DoRealtime() {
  NS_LOG_FUNCTION(this);
  return true;
}

uint64_t WallClockSynchronizer::DoGetCurrentRealtime() {
  NS_LOG_FUNCTION(this);
  return GetNormalizedRealtime();
}

void WallClockSynchronizer::DoSetOrigin(uint64_t ns) {
  NS_LOG_FUNCTION(this << ns);
  m_realtimeOriginNano = GetRealtime();
  NS_LOG_INFO("origin = " << m_realtimeOriginNano);
}

int64_t WallClockSynchronizer::DoGetDrift(uint64_t ns) {
  NS_LOG_FUNCTION(this << ns);
  uint64_t nsNow = GetNormalizedRealtime();

  if (nsNow > ns) {
    return (int64_t)(nsNow - ns);
  } else {
    return -(int64_t)(ns - nsNow);
  }
}

bool WallClockSynchronizer::DoSynchronize(uint64_t nsCurrent,
                                          uint64_t nsDelay) {
  NS_LOG_FUNCTION(this << nsCurrent << nsDelay);
  uint64_t ns = DriftCorrect(nsCurrent, nsDelay);
  NS_LOG_INFO("Synchronize ns = " << ns);
  uint64_t numberJiffies = ns / m_jiffy;
  NS_LOG_INFO("Synchronize numberJiffies = " << numberJiffies);
  if (numberJiffies > 3) {
    NS_LOG_INFO("SleepWait for " << numberJiffies * m_jiffy << " ns");
    NS_LOG_INFO("SleepWait until " << nsCurrent + numberJiffies * m_jiffy
                                   << " ns");
    if (!SleepWait((numberJiffies - 3) * m_jiffy)) {
      NS_LOG_INFO("SleepWait interrupted");
      return false;
    }
  }
  NS_LOG_INFO("Done with SleepWait");
  int64_t nsDrift = DoGetDrift(nsCurrent + nsDelay);
  if (nsDrift >= 0) {
    NS_LOG_INFO("Back from SleepWait: IML8 " << nsDrift);
    return true;
  }
  NS_LOG_INFO("SpinWait until " << nsCurrent + nsDelay);
  return SpinWait(nsCurrent + nsDelay);
}

void WallClockSynchronizer::DoSignal() {
  NS_LOG_FUNCTION(this);

  std::unique_lock<std::mutex> lock(m_mutex);
  m_condition = true;

  lock.unlock();
  m_conditionVariable.notify_one();
}

void WallClockSynchronizer::DoSetCondition(bool cond) {
  NS_LOG_FUNCTION(this << cond);
  m_condition = cond;
}

void WallClockSynchronizer::DoEventStart() {
  NS_LOG_FUNCTION(this);
  m_nsEventStart = GetNormalizedRealtime();
}

uint64_t WallClockSynchronizer::DoEventEnd() {
  NS_LOG_FUNCTION(this);
  return GetNormalizedRealtime() - m_nsEventStart;
}

bool WallClockSynchronizer::SpinWait(uint64_t ns) {
  NS_LOG_FUNCTION(this << ns);
  for (;;) {
    if (GetNormalizedRealtime() >= ns) {
      return true;
    }
    if (m_condition) {
      return false;
    }
  }
  return true;
}

bool WallClockSynchronizer::SleepWait(uint64_t ns) {
  NS_LOG_FUNCTION(this << ns);

  std::unique_lock<std::mutex> lock(m_mutex);
  bool finishedWaiting = m_conditionVariable.wait_for(
      lock, std::chrono::nanoseconds(ns), [this]() { return m_condition; });

  return finishedWaiting;
}

uint64_t WallClockSynchronizer::DriftCorrect(uint64_t nsNow, uint64_t nsDelay) {
  NS_LOG_FUNCTION(this << nsNow << nsDelay);
  int64_t drift = DoGetDrift(nsNow);
  if (drift < 0) {
    return nsDelay;
  }
  auto correction = (uint64_t)drift;
  if (correction <= nsDelay) {
    return nsDelay - correction;
  } else {
    return 0;
  }
}

uint64_t WallClockSynchronizer::GetRealtime() {
  NS_LOG_FUNCTION(this);
  auto now = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

uint64_t WallClockSynchronizer::GetNormalizedRealtime() {
  NS_LOG_FUNCTION(this);
  return GetRealtime() - m_realtimeOriginNano;
}

} // namespace ns3
