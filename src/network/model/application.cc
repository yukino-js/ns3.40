

#include "application.h"

#include "node.h"

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Application");

NS_OBJECT_ENSURE_REGISTERED(Application);

TypeId Application::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::Application")
          .SetParent<Object>()
          .SetGroupName("Network")
          .AddAttribute("StartTime", "Time at which the application will start",
                        TimeValue(Seconds(0.0)),
                        MakeTimeAccessor(&Application::m_startTime),
                        MakeTimeChecker())
          .AddAttribute("StopTime", "Time at which the application will stop",
                        TimeValue(TimeStep(0)),
                        MakeTimeAccessor(&Application::m_stopTime),
                        MakeTimeChecker());
  return tid;
}

Application::Application() { NS_LOG_FUNCTION(this); }

Application::~Application() { NS_LOG_FUNCTION(this); }

void Application::SetStartTime(Time start) {
  NS_LOG_FUNCTION(this << start);
  m_startTime = start;
}

void Application::SetStopTime(Time stop) {
  NS_LOG_FUNCTION(this << stop);
  m_stopTime = stop;
}

void Application::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_node = nullptr;
  m_startEvent.Cancel();
  m_stopEvent.Cancel();
  Object::DoDispose();
}

void Application::DoInitialize() {
  NS_LOG_FUNCTION(this);
  m_startEvent =
      Simulator::Schedule(m_startTime, &Application::StartApplication, this);
  if (m_stopTime != TimeStep(0)) {
    m_stopEvent =
        Simulator::Schedule(m_stopTime, &Application::StopApplication, this);
  }
  Object::DoInitialize();
}

Ptr<Node> Application::GetNode() const {
  NS_LOG_FUNCTION(this);
  return m_node;
}

void Application::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION(this);
  m_node = node;
}

void Application::StartApplication() { NS_LOG_FUNCTION(this); }

void Application::StopApplication() { NS_LOG_FUNCTION(this); }

} // namespace ns3
