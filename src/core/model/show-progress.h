
#ifndef SHOW_PROGRESS_H
#define SHOW_PROGRESS_H

#include "event-id.h"
#include "nstime.h"
#include "system-wall-clock-ms.h"
#include "system-wall-clock-timestamp.h"
#include "time-printer.h"

#include <iostream>

namespace ns3 {

class ShowProgress {
public:
  ShowProgress(const Time interval = Seconds(1.0),
               std::ostream &os = std::cout);

  ~ShowProgress();

  void SetInterval(const Time interval);

  void SetTimePrinter(TimePrinter lp);

  void SetStream(std::ostream &os);

  void SetVerbose(bool verbose);

private:
  void Start();

  void Stop();

  void ScheduleCheckProgress();

  void CheckProgress();

  void GiveFeedback(uint64_t nEvents, int64x64_t ratio, int64x64_t speed);

  static const int64x64_t HYSTERESIS;
  static const int64x64_t MAXGAIN;

  SystemWallClockMs m_timer;
  SystemWallClockTimestamp m_stamp;
  Time m_elapsed;
  Time m_interval;
  Time m_vtime;
  EventId m_event;
  uint64_t m_eventCount;

  TimePrinter m_printer;
  std::ostream *m_os;
  bool m_verbose;
  uint64_t m_repCount;
};

} // namespace ns3

#endif
