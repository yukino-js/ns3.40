#include "ns3/calendar-scheduler.h"
#include "ns3/config.h"
#include "ns3/heap-scheduler.h"
#include "ns3/list-scheduler.h"
#include "ns3/map-scheduler.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"

#include <chrono>
#include <ctime>
#include <list>
#include <thread>
#include <utility>

using namespace ns3;

constexpr int MAXTHREADS = 64;

class ThreadedSimulatorEventsTestCase : public TestCase {
public:
  ThreadedSimulatorEventsTestCase(ObjectFactory schedulerFactory,
                                  const std::string &simulatorType,
                                  unsigned int threads);
  void EventA(int a);
  void EventB(int b);
  void EventC(int c);
  void EventD(int d);
  void DoNothing(unsigned int threadno);
  static void SchedulingThread(
      std::pair<ThreadedSimulatorEventsTestCase *, unsigned int> context);
  void End();
  uint64_t m_a;
  uint64_t m_b;
  uint64_t m_c;
  uint64_t m_d;
  unsigned int m_threads;
  bool m_threadWaiting[MAXTHREADS];
  bool m_stop;
  ObjectFactory m_schedulerFactory;
  std::string m_simulatorType;
  std::string m_error;
  std::list<std::thread> m_threadlist;

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;
};

ThreadedSimulatorEventsTestCase::ThreadedSimulatorEventsTestCase(
    ObjectFactory schedulerFactory, const std::string &simulatorType,
    unsigned int threads)
    : TestCase("Check threaded event handling with " + std::to_string(threads) +
               " threads, " + schedulerFactory.GetTypeId().GetName() +
               " scheduler, in " + simulatorType),
      m_threads(threads), m_schedulerFactory(schedulerFactory),
      m_simulatorType(simulatorType) {}

void ThreadedSimulatorEventsTestCase::End() {
  m_stop = true;
  for (auto &thread : m_threadlist) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void ThreadedSimulatorEventsTestCase::SchedulingThread(
    std::pair<ThreadedSimulatorEventsTestCase *, unsigned int> context) {
  ThreadedSimulatorEventsTestCase *me = context.first;
  unsigned int threadno = context.second;

  while (!me->m_stop) {
    me->m_threadWaiting[threadno] = true;
    Simulator::ScheduleWithContext(threadno, MicroSeconds(1),
                                   &ThreadedSimulatorEventsTestCase::DoNothing,
                                   me, threadno);
    while (!me->m_stop && me->m_threadWaiting[threadno]) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(500));
    }
  }
}

void ThreadedSimulatorEventsTestCase::DoNothing(unsigned int threadno) {
  if (!m_error.empty()) {
    m_error = "Bad threaded scheduling";
  }
  m_threadWaiting[threadno] = false;
}

void ThreadedSimulatorEventsTestCase::EventA(int a) {
  if (m_a != m_b || m_a != m_c || m_a != m_d) {
    m_error = "Bad scheduling";
    Simulator::Stop();
  }
  ++m_a;
  Simulator::Schedule(MicroSeconds(10),
                      &ThreadedSimulatorEventsTestCase::EventB, this, a + 1);
}

void ThreadedSimulatorEventsTestCase::EventB(int b) {
  if (m_a != (m_b + 1) || m_a != (m_c + 1) || m_a != (m_d + 1)) {
    m_error = "Bad scheduling";
    Simulator::Stop();
  }
  ++m_b;
  Simulator::Schedule(MicroSeconds(10),
                      &ThreadedSimulatorEventsTestCase::EventC, this, b + 1);
}

void ThreadedSimulatorEventsTestCase::EventC(int c) {
  if (m_a != m_b || m_a != (m_c + 1) || m_a != (m_d + 1)) {
    m_error = "Bad scheduling";
    Simulator::Stop();
  }
  ++m_c;
  Simulator::Schedule(MicroSeconds(10),
                      &ThreadedSimulatorEventsTestCase::EventD, this, c + 1);
}

void ThreadedSimulatorEventsTestCase::EventD(int d) {
  if (m_a != m_b || m_a != m_c || m_a != (m_d + 1)) {
    m_error = "Bad scheduling";
    Simulator::Stop();
  }
  ++m_d;
  if (m_stop) {
    Simulator::Stop();
  } else {
    Simulator::Schedule(MicroSeconds(10),
                        &ThreadedSimulatorEventsTestCase::EventA, this, d + 1);
  }
}

void ThreadedSimulatorEventsTestCase::DoSetup() {
  if (!m_simulatorType.empty()) {
    Config::SetGlobal("SimulatorImplementationType",
                      StringValue(m_simulatorType));
  }

  m_error = "";

  m_a = m_b = m_c = m_d = 0;
}

void ThreadedSimulatorEventsTestCase::DoTeardown() {
  m_threadlist.clear();

  Config::SetGlobal("SimulatorImplementationType",
                    StringValue("ns3::DefaultSimulatorImpl"));
}

void ThreadedSimulatorEventsTestCase::DoRun() {
  m_stop = false;
  Simulator::SetScheduler(m_schedulerFactory);

  Simulator::Schedule(MicroSeconds(10),
                      &ThreadedSimulatorEventsTestCase::EventA, this, 1);
  Simulator::Schedule(Seconds(1), &ThreadedSimulatorEventsTestCase::End, this);

  for (unsigned int i = 0; i < m_threads; ++i) {
    m_threadlist.emplace_back(
        &ThreadedSimulatorEventsTestCase::SchedulingThread,
        std::pair<ThreadedSimulatorEventsTestCase *, unsigned int>(this, i));
  }

  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_EXPECT_MSG_EQ(m_error.empty(), true, m_error);
  NS_TEST_EXPECT_MSG_EQ(m_a, m_b, "Bad scheduling");
  NS_TEST_EXPECT_MSG_EQ(m_a, m_c, "Bad scheduling");
  NS_TEST_EXPECT_MSG_EQ(m_a, m_d, "Bad scheduling");
}

class ThreadedSimulatorTestSuite : public TestSuite {
public:
  ThreadedSimulatorTestSuite() : TestSuite("threaded-simulator") {
    std::string simulatorTypes[] = {
        "ns3::RealtimeSimulatorImpl",
        "ns3::DefaultSimulatorImpl",
    };
    std::string schedulerTypes[] = {
        "ns3::ListScheduler",
        "ns3::HeapScheduler",
        "ns3::MapScheduler",
        "ns3::CalendarScheduler",
    };
    unsigned int threadCounts[] = {0, 2, 10, 20};
    ObjectFactory factory;

    for (auto &simulatorType : simulatorTypes) {
      for (auto &schedulerType : schedulerTypes) {
        for (auto &threadCount : threadCounts) {
          factory.SetTypeId(schedulerType);
          AddTestCase(new ThreadedSimulatorEventsTestCase(
                          factory, simulatorType, threadCount),
                      TestCase::QUICK);
        }
      }
    }
  }
};

static ThreadedSimulatorTestSuite g_threadedSimulatorTestSuite;
