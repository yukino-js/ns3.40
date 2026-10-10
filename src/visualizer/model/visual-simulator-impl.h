
#ifndef VISUAL_SIMULATOR_IMPL_H
#define VISUAL_SIMULATOR_IMPL_H

#include "ns3/simulator-impl.h"

namespace ns3 {

class VisualSimulatorImpl : public SimulatorImpl {
public:
  static TypeId GetTypeId();

  VisualSimulatorImpl();
  ~VisualSimulatorImpl() override;

  void Destroy() override;
  bool IsFinished() const override;
  void Stop() override;
  void Stop(const Time &delay) override;
  EventId Schedule(const Time &delay, EventImpl *event) override;
  void ScheduleWithContext(uint32_t context, const Time &delay,
                           EventImpl *event) override;
  EventId ScheduleNow(EventImpl *event) override;
  EventId ScheduleDestroy(EventImpl *event) override;
  void Remove(const EventId &id) override;
  void Cancel(const EventId &id) override;
  bool IsExpired(const EventId &id) const override;
  void Run() override;
  Time Now() const override;
  Time GetDelayLeft(const EventId &id) const override;
  Time GetMaximumSimulationTime() const override;
  void SetScheduler(ObjectFactory schedulerFactory) override;
  uint32_t GetSystemId() const override;
  uint32_t GetContext() const override;
  uint64_t GetEventCount() const override;

  void RunRealSimulator();

protected:
  void DoDispose() override;
  void NotifyConstructionCompleted() override;

private:
  Ptr<SimulatorImpl> GetSim();
  Ptr<SimulatorImpl> m_simulator;
  ObjectFactory m_simulatorImplFactory;
};

} // namespace ns3

#endif
