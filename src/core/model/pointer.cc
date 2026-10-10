#include "pointer.h"

#include "log.h"
#include "object-factory.h"

#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Pointer");

PointerValue::PointerValue() : m_value() { NS_LOG_FUNCTION(this); }

PointerValue::PointerValue(Ptr<Object> object) : m_value(object) {
  NS_LOG_FUNCTION(object);
}

void PointerValue::SetObject(Ptr<Object> object) {
  NS_LOG_FUNCTION(object);
  m_value = object;
}

Ptr<Object> PointerValue::GetObject() const {
  NS_LOG_FUNCTION(this);
  return m_value;
}

Ptr<AttributeValue> PointerValue::Copy() const {
  NS_LOG_FUNCTION(this);
  return Create<PointerValue>(*this);
}

std::string
PointerValue::SerializeToString(Ptr<const AttributeChecker> checker) const {
  NS_LOG_FUNCTION(this << checker);
  std::ostringstream oss;
  oss << m_value;
  return oss.str();
}

bool PointerValue::DeserializeFromString(std::string value,
                                         Ptr<const AttributeChecker> checker) {
  NS_LOG_FUNCTION(this << value << checker);
  ObjectFactory factory;
  std::istringstream iss;
  iss.str(value);
  iss >> factory;
  if (iss.fail()) {
    return false;
  }
  m_value = factory.Create<Object>();
  return true;
}

} // namespace ns3
