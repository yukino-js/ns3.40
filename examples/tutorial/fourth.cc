

#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include "ns3/uinteger.h"

#include <iostream>

using namespace ns3;

class MyObject : public Object {
public:
  static TypeId GetTypeId() {
    static TypeId tid =
        TypeId("MyObject")
            .SetParent<Object>()
            .SetGroupName("Tutorial")
            .AddConstructor<MyObject>()
            .AddTraceSource("MyInteger", "An integer value to trace.",
                            MakeTraceSourceAccessor(&MyObject::m_myInt),
                            "ns3::TracedValueCallback::Int32");
    return tid;
  }

  MyObject() {}

  TracedValue<int32_t> m_myInt;
};

void IntTrace(int32_t oldValue, int32_t newValue) {
  std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
}

int main(int argc, char *argv[]) {
  Ptr<MyObject> myObject = CreateObject<MyObject>();
  myObject->TraceConnectWithoutContext("MyInteger", MakeCallback(&IntTrace));

  myObject->m_myInt = 1234;

  return 0;
}
