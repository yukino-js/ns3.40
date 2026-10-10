
#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include "nstime.h"
#include "object.h"

#include <stdint.h>

namespace ns3 {

class Synchronizer : public Object {
public:
  static TypeId GetTypeId();

  Synchronizer();
  ~Synchronizer() override;

  bool Realtime();

  uint64_t GetCurrentRealtime();

  void SetOrigin(uint64_t ts);

  uint64_t GetOrigin();

  int64_t GetDrift(uint64_t ts);

  bool Synchronize(uint64_t tsCurrent, uint64_t tsDelay);

  void Signal();

  void SetCondition(bool cond);

  void EventStart();

  uint64_t EventEnd();

protected:
  virtual void DoSetOrigin(uint64_t ns) = 0;

  virtual bool DoRealtime() = 0;

  virtual uint64_t DoGetCurrentRealtime() = 0;

  virtual bool DoSynchronize(uint64_t nsCurrent, uint64_t nsDelay) = 0;

  virtual void DoSignal() = 0;

  virtual void DoSetCondition(bool cond) = 0;

  virtual int64_t DoGetDrift(uint64_t ns) = 0;

  virtual void DoEventStart() = 0;
  virtual uint64_t DoEventEnd() = 0;

  uint64_t m_realtimeOriginNano;
  uint64_t m_simOriginNano;

private:
  uint64_t TimeStepToNanosecond(uint64_t ts);

  uint64_t NanosecondToTimeStep(uint64_t ns);
};

} // namespace ns3

#endif
