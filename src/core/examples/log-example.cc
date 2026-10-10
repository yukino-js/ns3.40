

#define NS_LOG_APPEND_CONTEXT                                                  \
  {                                                                            \
    std::clog << "(local context) ";                                           \
  }

#include "ns3/core-module.h"
#include "ns3/network-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LogExample");

namespace {

void FreeEvent() {
  NS_LOG_FUNCTION_NOARGS();

  NS_LOG_ERROR("FreeEvent: error msg");
  NS_LOG_WARN("FreeEvent: warning msg");
  NS_LOG_INFO("FreeEvent: info msg");
  NS_LOG_LOGIC("FreeEvent: logic msg");
  NS_LOG_DEBUG("FreeEvent: debug msg");
}

class MyEventObject : public Object {
public:
  static TypeId GetTypeId() {
    static TypeId tid = TypeId("MyEventObject")
                            .SetParent<Object>()
                            .AddConstructor<MyEventObject>();
    return tid;
  }

  MyEventObject() { NS_LOG_FUNCTION(this); }

  ~MyEventObject() override { NS_LOG_FUNCTION(this); }

  void Event() {
    NS_LOG_FUNCTION(this);

    NS_LOG_ERROR("MyEventObject:Event: error msg");
    NS_LOG_WARN("MyEventObject:Event: warning msg");
    NS_LOG_INFO("MyEventObject:Event: info msg");
    NS_LOG_LOGIC("MyEventObject:Event: logic msg");
    NS_LOG_DEBUG("MyEventObject:Event: debug msg");
  }
};

NS_OBJECT_ENSURE_REGISTERED(MyEventObject);

} // namespace

int main(int argc, char **argv) {
  CommandLine cmd;
  cmd.Parse(argc, argv);

  NS_LOG_DEBUG("Creating a Node");
  auto node = CreateObject<Node>();

  NS_LOG_DEBUG("Creating MyEventObject");
  auto myObj = CreateObject<MyEventObject>();

  NS_LOG_DEBUG("Aggregating MyEventObject to Node");
  node->AggregateObject(myObj);

  NS_LOG_INFO("Scheduling the MyEventObject::Event with node context");
  Simulator::ScheduleWithContext(node->GetId(), Seconds(3),
                                 &MyEventObject::Event, &(*myObj));

  NS_LOG_INFO("Scheduling FreeEvent");
  Simulator::Schedule(Seconds(5), FreeEvent);

  NS_LOG_DEBUG("Starting run...");
  Simulator::Run();
  NS_LOG_DEBUG("... run complete");
  Simulator::Destroy();
  NS_LOG_DEBUG("Goodbye");

  return 0;
}
