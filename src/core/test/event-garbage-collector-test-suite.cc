
#include "ns3/event-garbage-collector.h"
#include "ns3/test.h"

namespace ns3 {

namespace tests {

class EventGarbageCollectorTestCase : public TestCase {
  int m_counter;
  EventGarbageCollector *m_events;

  void EventGarbageCollectorCallback();

public:
  EventGarbageCollectorTestCase();
  ~EventGarbageCollectorTestCase() override;
  void DoRun() override;
};

EventGarbageCollectorTestCase::EventGarbageCollectorTestCase()
    : TestCase("EventGarbageCollector"), m_counter(0), m_events(nullptr) {}

EventGarbageCollectorTestCase::~EventGarbageCollectorTestCase() {}

void EventGarbageCollectorTestCase::EventGarbageCollectorCallback() {
  m_counter++;
  if (m_counter == 50) {
    delete m_events;
    m_events = nullptr;
  }
}

void EventGarbageCollectorTestCase::DoRun() {
  m_events = new EventGarbageCollector();

  for (int n = 0; n < 100; n++) {
    m_events->Track(Simulator::Schedule(
        Simulator::Now(),
        &EventGarbageCollectorTestCase::EventGarbageCollectorCallback, this));
  }
  Simulator::Run();
  NS_TEST_EXPECT_MSG_EQ(m_events, 0, "");
  NS_TEST_EXPECT_MSG_EQ(m_counter, 50, "");
  Simulator::Destroy();
}

class EventGarbageCollectorTestSuite : public TestSuite {
public:
  EventGarbageCollectorTestSuite() : TestSuite("event-garbage-collector") {
    AddTestCase(new EventGarbageCollectorTestCase());
  }
};

static EventGarbageCollectorTestSuite g_eventGarbageCollectorTestSuite;

} // namespace tests

} // namespace ns3
