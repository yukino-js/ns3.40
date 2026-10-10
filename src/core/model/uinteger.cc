#include "uinteger.h"

#include "fatal-error.h"
#include "log.h"

#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Uinteger");

ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME(uint64_t, Uinteger);

namespace internal {

Ptr<const AttributeChecker> MakeUintegerChecker(uint64_t min, uint64_t max,
                                                std::string name) {
  NS_LOG_FUNCTION(min << max << name);

  struct Checker : public AttributeChecker {
    Checker(uint64_t minValue, uint64_t maxValue, std::string name)
        : m_minValue(minValue), m_maxValue(maxValue), m_name(name) {}

    bool Check(const AttributeValue &value) const override {
      NS_LOG_FUNCTION(&value);
      const auto v = dynamic_cast<const UintegerValue *>(&value);
      if (v == nullptr) {
        return false;
      }
      return v->Get() >= m_minValue && v->Get() <= m_maxValue;
    }

    std::string GetValueTypeName() const override {
      NS_LOG_FUNCTION_NOARGS();
      return "ns3::UintegerValue";
    }

    bool HasUnderlyingTypeInformation() const override {
      NS_LOG_FUNCTION_NOARGS();
      return true;
    }

    std::string GetUnderlyingTypeInformation() const override {
      NS_LOG_FUNCTION_NOARGS();
      std::ostringstream oss;
      oss << m_name << " " << m_minValue << ":" << m_maxValue;
      return oss.str();
    }

    Ptr<AttributeValue> Create() const override {
      NS_LOG_FUNCTION_NOARGS();
      return ns3::Create<UintegerValue>();
    }

    bool Copy(const AttributeValue &source,
              AttributeValue &destination) const override {
      NS_LOG_FUNCTION(&source << &destination);
      const auto src = dynamic_cast<const UintegerValue *>(&source);
      auto dst = dynamic_cast<UintegerValue *>(&destination);
      if (src == nullptr || dst == nullptr) {
        return false;
      }
      *dst = *src;
      return true;
    }

    uint64_t m_minValue;
    uint64_t m_maxValue;
    std::string m_name;
  } *checker = new Checker(min, max, name);

  return Ptr<const AttributeChecker>(checker, false);
}

} // namespace internal

} // namespace ns3
