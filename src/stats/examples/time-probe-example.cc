

#include "ns3/core-module.h"
#include "ns3/gnuplot-helper.h"
#include "ns3/time-probe.h"

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TimeProbeExample");

class Emitter : public Object {
public:
  static TypeId GetTypeId();
  Emitter();

private:
  void DoInitialize() override;
  void Emit();

  TracedValue<Time> m_interval;
  Time m_last;
  Ptr<ExponentialRandomVariable> m_var;
};

NS_OBJECT_ENSURE_REGISTERED(Emitter);

TypeId Emitter::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::Emitter")
          .SetParent<Object>()
          .SetGroupName("Stats")
          .AddConstructor<Emitter>()
          .AddTraceSource("Interval", "Trace source",
                          MakeTraceSourceAccessor(&Emitter::m_interval),
                          "ns3::TracedValueCallback::Time");
  return tid;
}

Emitter::Emitter() : m_interval(Seconds(0)), m_last(Seconds(0)) {
  m_var = CreateObject<ExponentialRandomVariable>();
}

void Emitter::DoInitialize() {
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Emit, this);
}

void Emitter::Emit() {
  NS_LOG_DEBUG("Emitting at " << Simulator::Now().As(Time::S));
  m_interval = Simulator::Now() - m_last;
  m_last = Simulator::Now();
  TimeProbe::SetValueByPath("/Names/probe3", m_interval);
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Emit, this);
}

void NotifyViaTraceSource(std::string context, Time oldVal, Time newVal) {
  BooleanValue verbose;
  GlobalValue::GetValueByName("verbose", verbose);
  if (verbose.Get()) {
    std::cout << "context: " << context << " old " << oldVal.As(Time::S)
              << " new " << newVal.As(Time::S) << std::endl;
  }
}

void NotifyViaProbe(std::string context, double oldVal, double newVal) {
  BooleanValue verbose;
  GlobalValue::GetValueByName("verbose", verbose);
  if (verbose.Get()) {
    std::cout << "context: " << context << " old " << oldVal << " new "
              << newVal << std::endl;
  }
}

static ns3::GlobalValue g_verbose("verbose", "Whether to enable verbose output",
                                  ns3::BooleanValue(false),
                                  ns3::MakeBooleanChecker());

int main(int argc, char *argv[]) {
  double stopTime = 100.0;
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("stopTime", "Time (seconds) to terminate simulation", stopTime);
  cmd.AddValue("verbose", "Whether to enable verbose output", verbose);
  cmd.Parse(argc, argv);
  bool connected;

  if (verbose) {
    GlobalValue::Bind("verbose", BooleanValue(true));
    LogComponentEnable("TimeProbeExample", LOG_LEVEL_ALL);
  }

  Ptr<Emitter> emitter = CreateObject<Emitter>();
  Names::Add("/Names/Emitter", emitter);

  connected = emitter->TraceConnect("Interval", "raw trace source",
                                    MakeCallback(&NotifyViaTraceSource));
  NS_ASSERT_MSG(connected, "Trace source not connected");

  Ptr<TimeProbe> probe1 = CreateObject<TimeProbe>();
  probe1->SetName("probe1");

  connected = probe1->ConnectByObject("Interval", emitter);
  NS_ASSERT_MSG(connected, "Trace source not connected to probe1");

  connected = probe1->TraceConnect("Output", probe1->GetName(),
                                   MakeCallback(&NotifyViaProbe));
  NS_ASSERT_MSG(connected, "Trace source not connected to probe1 Output");

  Ptr<TimeProbe> probe2 = CreateObject<TimeProbe>();
  probe2->SetName("probe2");

  probe2->ConnectByPath("/Names/Emitter/Interval");

  connected =
      probe2->TraceConnect("Output", "probe2", MakeCallback(&NotifyViaProbe));
  NS_ASSERT_MSG(connected, "Trace source not connected to probe2 Output");

  Ptr<TimeProbe> probe3 = CreateObject<TimeProbe>();
  probe3->SetName("probe3");

  Names::Add("/Names/probe3", probe3);

  connected =
      probe3->TraceConnect("Output", "probe3", MakeCallback(&NotifyViaProbe));
  NS_ASSERT_MSG(connected, "Trace source not connected to probe3 Output");

  GnuplotHelper plotHelper;
  plotHelper.ConfigurePlot(
      "time-probe-example", "Emitter interarrivals vs. Time",
      "Simulation time (Seconds)", "Interarrival time (Seconds)", "png");

  plotHelper.PlotProbe("ns3::TimeProbe", "/Names/Emitter/Interval", "Output",
                       "Emitter Interarrival Time",
                       GnuplotAggregator::KEY_INSIDE);

  Simulator::Schedule(Seconds(0.0), &Emitter::Initialize, emitter);

  Simulator::Stop(Seconds(stopTime));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
