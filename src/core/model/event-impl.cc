
#include "event-impl.h"

#include "log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("EventImpl");

EventImpl::~EventImpl() { NS_LOG_FUNCTION(this); }

EventImpl::EventImpl() : m_cancel(false) { NS_LOG_FUNCTION(this); }

void EventImpl::Invoke() {
  NS_LOG_FUNCTION(this);
  if (!m_cancel) {
    Notify();
  }
}

void EventImpl::Cancel() {
  NS_LOG_FUNCTION(this);
  m_cancel = true;
}

bool EventImpl::IsCancelled() {
  NS_LOG_FUNCTION(this);
  return m_cancel;
}

} // namespace ns3
