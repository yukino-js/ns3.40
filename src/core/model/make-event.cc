
#include "make-event.h"

#include "log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MakeEvent");

EventImpl *MakeEvent(void (*f)()) {
  NS_LOG_FUNCTION(f);

  class EventFunctionImpl0 : public EventImpl {
  public:
    typedef void (*F)();

    EventFunctionImpl0(F function) : m_function(function) {}

    ~EventFunctionImpl0() override {}

  protected:
    void Notify() override { (*m_function)(); }

  private:
    F m_function;
  } *ev = new EventFunctionImpl0(f);

  return ev;
}

} // namespace ns3
