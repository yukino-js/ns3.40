

#include "ns3/core-module.h"
#include "ns3/double-probe.h"

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DoubleProbeExample");

class Emitter : public Object {
public:
  static TypeId GetTypeId();
  Emitter();

private:
  void DoInitialize() override;

  void Emit();
  void Count();

  TracedValue<double> m_counter;
  Ptr<ExponentialRandomVariable> m_var;
};

NS_OBJECT_ENSURE_REGISTERED(Emitter);

TypeId Emitter::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::Emitter")
          .SetParent<Object>()
          .SetGroupName("Stats")
          .AddConstructor<Emitter>()
          .AddTraceSource("Counter", "sample counter",
                          MakeTraceSourceAccessor(&Emitter::m_counter),
                          "ns3::TracedValueCallback::Double");
  return tid;
}

Emitter::Emitter() {
  NS_LOG_FUNCTION(this);
  m_counter = 0;
  m_var = CreateObject<ExponentialRandomVariable>();
}

void Emitter::DoInitialize() {
  NS_LOG_FUNCTION(this);
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Emit, this);
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Count, this);
}

void Emitter::Emit() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("Emitting at " << Simulator::Now().As(Time::S));
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Emit, this);
}

void Emitter::Count() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("Counting at " << Simulator::Now().As(Time::S));
  m_counter += 1.0;
  DoubleProbe::SetValueByPath("/Names/StaticallyAccessedProbe", m_counter);
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Count, this);
}

void NotifyViaTraceSource(std::string context, double oldVal, double newVal) {
  NS_LOG_DEBUG("context: " << context << " old " << oldVal << " new "
                           << newVal);
}

void NotifyViaProbe(std::string context, double oldVal, double newVal) {
  NS_LOG_DEBUG("context: " << context << " old " << oldVal << " new "
                           << newVal);
}

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);
  bool connected;

  Ptr<Emitter> emitter = CreateObject<Emitter>();
  Names::Add("/Names/Emitter", emitter);

  connected = emitter->TraceConnect("Counter", "sample context",
                                    MakeCallback(&NotifyViaTraceSource));
  NS_ASSERT_MSG(connected, "Trace source not connected");

  Ptr<DoubleProbe> probe1 = CreateObject<DoubleProbe>();
  probe1->SetName("ObjectProbe");

  connected = probe1->ConnectByObject("Counter", emitter);
  NS_ASSERT_MSG(connected, "Trace source not connected to probe1");

  connected = probe1->TraceConnect("Output", probe1->GetName(),
                                   MakeCallback(&NotifyViaProbe));
  NS_ASSERT_MSG(connected, "Trace source not connected to probe1 Output");

  Ptr<DoubleProbe> probe2 = CreateObject<DoubleProbe>();
  probe2->SetName("PathProbe");

  probe2->ConnectByPath("/Names/Emitter/Counter");

  connected = probe2->TraceConnect("Output", "/Names/Probes/PathProbe/Output",
                                   MakeCallback(&NotifyViaProbe));
  NS_ASSERT_MSG(connected, "Trace source not connected to probe2 Output");

  Ptr<DoubleProbe> probe3 = CreateObject<DoubleProbe>();
  probe3->SetName("StaticallyAccessedProbe");
  Names::Add("/Names/Probes", probe3->GetName(), probe3);

  connected = probe3->TraceConnect(
      "Output", "/Names/Probes/StaticallyAccessedProbe/Output",
      MakeCallback(&NotifyViaProbe));
  NS_ASSERT_MSG(connected, "Trace source not connected to probe3 Output");

  Simulator::Schedule(Seconds(0.0), &Emitter::Initialize, emitter);

  Simulator::Stop(Seconds(100.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
