

#include "ns3/core-module.h"
#include "ns3/stats-module.h"

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FileHelperExample");

class Emitter : public Object {
public:
  static TypeId GetTypeId();
  Emitter();

private:
  void DoInitialize() override;
  void Count();

  TracedValue<uint32_t> m_counter;
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
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Count, this);
}

void Emitter::Count() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("Counting at " << Simulator::Now().As(Time::S));
  m_counter += 1.0;
  Simulator::Schedule(Seconds(m_var->GetValue()), &Emitter::Count, this);
}

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  Ptr<Emitter> emitter = CreateObject<Emitter>();
  Names::Add("/Names/Emitter", emitter);

  FileHelper fileHelper;

  fileHelper.ConfigureFile("file-helper-example", FileAggregator::FORMATTED);

  fileHelper.Set2dFormat("Time (Seconds) = %.3f\tCount = %.0f");

  fileHelper.WriteProbe("ns3::Uinteger32Probe", "/Names/Emitter/Counter",
                        "Output");

  Simulator::Schedule(Seconds(0.0), &Emitter::Initialize, emitter);

  Simulator::Stop(Seconds(100.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
