

#include "show-progress.h"

#include "event-id.h"
#include "log.h"
#include "nstime.h"
#include "simulator.h"

#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ShowProgress");

const int64x64_t ShowProgress::HYSTERESIS = 1.414;
const int64x64_t ShowProgress::MAXGAIN = 2.0;

ShowProgress::ShowProgress(const Time interval, std::ostream &os)
    : m_timer(), m_stamp(), m_elapsed(), m_interval(interval), m_vtime(Time(1)),
      m_event(), m_eventCount(0), m_printer(DefaultTimePrinter), m_os(&os),
      m_verbose(false), m_repCount(0) {
  NS_LOG_FUNCTION(this << interval);
  ScheduleCheckProgress();
  Start();
}

ShowProgress::~ShowProgress() { Stop(); }

void ShowProgress::SetInterval(const Time interval) {
  NS_LOG_FUNCTION(this << interval);
  const int64x64_t ratio = interval / m_interval;
  m_interval = interval;
  if (m_vtime > Time(1)) {
    m_vtime = m_vtime * ratio;
  }
  Simulator::Cancel(m_event);
  Start();
}

void ShowProgress::SetTimePrinter(TimePrinter lp) {
  NS_LOG_FUNCTION(this << lp);
  m_printer = lp;
}

void ShowProgress::SetVerbose(bool verbose) {
  NS_LOG_FUNCTION(this << verbose);
  m_verbose = verbose;
}

void ShowProgress::SetStream(std::ostream &os) { m_os = &os; }

void ShowProgress::ScheduleCheckProgress() {
  NS_LOG_FUNCTION(this);
  m_event = Simulator::Schedule(m_vtime, &ShowProgress::CheckProgress, this);
  m_timer.Start();
}

void ShowProgress::GiveFeedback(uint64_t nEvents, int64x64_t ratio,
                                int64x64_t speed) {
  auto precision = m_os->precision();
  auto flags = m_os->flags();

  m_os->setf(std::ios::fixed, std::ios::floatfield);

  if (m_verbose) {
    (*m_os) << std::right << std::setw(5) << m_repCount << std::left
            << (ratio > (1.0 / HYSTERESIS) ? "-->" : "   ")
            << std::setprecision(9) << " [del: " << m_elapsed.As(Time::S)
            << "/ int: " << m_interval.As(Time::S) << " = rat: " << ratio
            << (ratio > HYSTERESIS ? " dn"
                                   : (ratio < 1.0 / HYSTERESIS ? " up" : " --"))
            << ", vt: " << m_vtime.As(Time::S) << "] ";
  }

  (*m_printer)(*m_os);

  (*m_os) << " (" << std::setprecision(3) << std::setw(8) << speed.GetDouble()
          << "x real time) " << nEvents << " events processed" << std::endl
          << std::flush;

  m_os->precision(precision);
  m_os->flags(flags);
}

void ShowProgress::CheckProgress() {
  m_elapsed += MilliSeconds(m_timer.End());
  NS_LOG_FUNCTION(this << m_elapsed);

  if (m_elapsed <= Time(0)) {
    m_vtime = m_vtime * MAXGAIN;
    ++m_repCount;
    ScheduleCheckProgress();
    return;
  }

  const int64x64_t speed = m_vtime / m_elapsed;

  const int64x64_t ratio = m_elapsed / m_interval;

  uint64_t events = Simulator::GetEventCount();
  uint64_t nEvents = events - m_eventCount;
  if (ratio > HYSTERESIS) {
    int64x64_t f = 1 + (ratio - 1) / 2;
    if (ratio > MAXGAIN) {
      f = MAXGAIN;
    }

    m_vtime = m_vtime / f;
  } else if (ratio < 1.0 / HYSTERESIS) {
    int64x64_t f = 1 + (1 / ratio - 1) / 2;
    if (1 / ratio > MAXGAIN) {
      f = MAXGAIN;
    }
    m_vtime = m_vtime * f;
  }

  if (ratio > (1.0 / HYSTERESIS)) {
    GiveFeedback(nEvents, ratio, speed);
    m_elapsed = Time(0);
    m_eventCount = events;
  } else {
    NS_LOG_LOGIC("skipping update: " << ratio);
  }
  ++m_repCount;

  ScheduleCheckProgress();
}

void ShowProgress::Start() {
  m_stamp.Stamp();
  (*m_os) << "Start wall clock: " << m_stamp.ToString() << std::endl;
}

void ShowProgress::Stop() {
  m_stamp.Stamp();
  (*m_os) << "End wall clock:  " << m_stamp.ToString()
          << "\nElapsed wall clock: " << m_stamp.GetInterval() << "s"
          << std::endl;
}

} // namespace ns3
