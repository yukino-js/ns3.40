
#include "ns3/double-probe.h"
#include "ns3/names.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include "ns3/type-id.h"

using namespace ns3;

class SampleEmitter : public Object {
public:
  static TypeId GetTypeId();

  SampleEmitter() { m_var = CreateObject<ExponentialRandomVariable>(); }

  ~SampleEmitter() override {}

  void Start() { Reschedule(); }

  void Reschedule() {
    m_time = m_var->GetValue();
    Simulator::Schedule(Seconds(m_time), &SampleEmitter::Report, this);
    m_time += Simulator::Now().GetSeconds();
  }

  double GetTime() const { return m_time; }

  double GetValue() const { return aux; }

private:
  void Report() {
    aux = m_var->GetValue();
    m_trace = aux;
    Reschedule();
  }

  Ptr<ExponentialRandomVariable> m_var;
  double m_time;
  TracedValue<double> m_trace;
  double aux;
};

TypeId SampleEmitter::GetTypeId() {
  static TypeId tid =
      TypeId("SampleEmitter")
          .SetParent<Object>()
          .AddTraceSource("Emitter", "XX",
                          MakeTraceSourceAccessor(&SampleEmitter::m_trace),
                          "ns3::TracedValueCallback::Double");
  return tid;
}

class ProbeTestCase1 : public TestCase {
public:
  ProbeTestCase1();
  ~ProbeTestCase1() override;

private:
  void DoRun() override;

  void TraceSink(std::string context, double oldValue, double newValue);
  uint32_t m_objectProbed;
  uint32_t m_pathProbed;
  Ptr<SampleEmitter> m_s;
};

ProbeTestCase1::ProbeTestCase1()
    : TestCase("basic probe test case"), m_objectProbed(0), m_pathProbed(0) {}

ProbeTestCase1::~ProbeTestCase1() {}

void ProbeTestCase1::TraceSink(std::string context, double oldValue,
                               double newValue) {
  NS_TEST_ASSERT_MSG_GT(Simulator::Now(), Seconds(100),
                        "Probed a value outside of the time window");
  NS_TEST_ASSERT_MSG_LT(Simulator::Now(), Seconds(200),
                        "Probed a value outside of the time window");

  NS_TEST_ASSERT_MSG_EQ_TOL(
      m_s->GetValue(), newValue, 0.00001,
      "Value probed different than value in the variable");

  if (context == "testProbe") {
    m_objectProbed++;
  } else if (context == "testProbe2") {
    m_pathProbed++;
  }
}

void ProbeTestCase1::DoRun() {
  m_s = CreateObject<SampleEmitter>();

  Ptr<DoubleProbe> p = CreateObject<DoubleProbe>();
  p->SetName("testProbe");

  Simulator::Schedule(Seconds(1), &SampleEmitter::Start, m_s);
  p->SetAttribute("Start", TimeValue(Seconds(100.0)));
  p->SetAttribute("Stop", TimeValue(Seconds(200.0)));
  Simulator::Stop(Seconds(300));

  Names::Add("/Names/SampleEmitter", m_s);

  p->ConnectByObject("Emitter", m_s);

  p->TraceConnect("Output", p->GetName(),
                  MakeCallback(&ProbeTestCase1::TraceSink, this));

  Ptr<DoubleProbe> p2 = CreateObject<DoubleProbe>();
  p2->SetName("testProbe2");
  p2->SetAttribute("Start", TimeValue(Seconds(100.0)));
  p2->SetAttribute("Stop", TimeValue(Seconds(200.0)));

  p2->ConnectByPath("/Names/SampleEmitter/Emitter");

  p2->TraceConnect("Output", p2->GetName(),
                   MakeCallback(&ProbeTestCase1::TraceSink, this));

  Simulator::Run();

  NS_TEST_ASSERT_MSG_GT(m_objectProbed, 0,
                        "Trace sink for object probe never called");
  NS_TEST_ASSERT_MSG_GT(m_pathProbed, 0,
                        "Trace sink for path probe never called");
  Simulator::Destroy();
}

class ProbeTestSuite : public TestSuite {
public:
  ProbeTestSuite();
};

ProbeTestSuite::ProbeTestSuite() : TestSuite("double-probe", UNIT) {
  AddTestCase(new ProbeTestCase1, TestCase::QUICK);
}

static ProbeTestSuite probeTestSuite;
