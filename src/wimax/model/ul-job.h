
#ifndef UL_JOB_H
#define UL_JOB_H

#include "service-flow-record.h"
#include "service-flow.h"
#include "ss-record.h"

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class SSRecord;
class ServiceFlow;

enum ReqType { DATA, UNICAST_POLLING };

class UlJob : public Object {
public:
  enum JobPriority { LOW, INTERMEDIATE, HIGH };

  UlJob();
  ~UlJob() override;
  SSRecord *GetSsRecord() const;
  void SetSsRecord(SSRecord *ssRecord);
  ServiceFlow::SchedulingType GetSchedulingType() const;
  void SetSchedulingType(ServiceFlow::SchedulingType schedulingType);
  ServiceFlow *GetServiceFlow() const;
  void SetServiceFlow(ServiceFlow *serviceFlow);

  ReqType GetType() const;
  void SetType(ReqType type);

  Time GetReleaseTime() const;
  void SetReleaseTime(Time releaseTime);

  Time GetPeriod() const;
  void SetPeriod(Time period);

  Time GetDeadline() const;
  void SetDeadline(Time deadline);

  uint32_t GetSize() const;
  void SetSize(uint32_t size);

private:
  friend bool operator==(const UlJob &a, const UlJob &b);

  Time m_releaseTime;
  Time m_period;
  Time m_deadline;
  uint32_t m_size;
  ServiceFlow::SchedulingType m_schedulingType;

  SSRecord *m_ssRecord;

  ReqType m_type;
  ServiceFlow *m_serviceFlow;
};

class PriorityUlJob : public Object {
public:
  PriorityUlJob();
  int GetPriority() const;
  void SetPriority(int priority);

  Ptr<UlJob> GetUlJob() const;
  void SetUlJob(Ptr<UlJob> job);

private:
  int m_priority;
  Ptr<UlJob> m_job;
};

struct SortProcess {
  bool operator()(PriorityUlJob &left, PriorityUlJob &right) const {
    if (left.GetPriority() < right.GetPriority()) {
      return true;
    } else if (left.GetPriority() == right.GetPriority()) {
      int32_t leftBacklogged =
          left.GetUlJob()->GetServiceFlow()->GetRecord()->GetBacklogged();
      int32_t rightBacklogged =
          left.GetUlJob()->GetServiceFlow()->GetRecord()->GetBacklogged();
      return leftBacklogged <= rightBacklogged;
    } else {
      return false;
    }
  }
};

struct SortProcessPtr {
  bool operator()(Ptr<PriorityUlJob> &left, Ptr<PriorityUlJob> &right) const {
    if (left->GetPriority() < right->GetPriority()) {
      return true;
    } else if (left->GetPriority() == right->GetPriority()) {
      int32_t leftBacklogged =
          left->GetUlJob()->GetServiceFlow()->GetRecord()->GetBacklogged();
      int32_t rightBacklogged =
          left->GetUlJob()->GetServiceFlow()->GetRecord()->GetBacklogged();
      return leftBacklogged <= rightBacklogged;
    } else {
      return false;
    }
  }
};

} // namespace ns3

#endif
