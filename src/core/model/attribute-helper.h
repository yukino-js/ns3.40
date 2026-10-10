#ifndef ATTRIBUTE_HELPER_H
#define ATTRIBUTE_HELPER_H

#include "abort.h"
#include "attribute-accessor-helper.h"
#include "attribute.h"

#include <sstream>

namespace ns3 {

template <typename T, typename BASE>
Ptr<AttributeChecker> MakeSimpleAttributeChecker(std::string name,
                                                 std::string underlying) {
  struct SimpleAttributeChecker : public BASE {
    bool Check(const AttributeValue &value) const override {
      return dynamic_cast<const T *>(&value) != nullptr;
    }

    std::string GetValueTypeName() const override {
      if (m_type.rfind("ns3::", 0) == 0) {
        return m_type;
      }
      return "ns3::" + m_type;
    }

    bool HasUnderlyingTypeInformation() const override { return true; }

    std::string GetUnderlyingTypeInformation() const override {
      return m_underlying;
    }

    Ptr<AttributeValue> Create() const override { return ns3::Create<T>(); }

    bool Copy(const AttributeValue &source,
              AttributeValue &destination) const override {
      const T *src = dynamic_cast<const T *>(&source);
      T *dst = dynamic_cast<T *>(&destination);
      if (src == nullptr || dst == nullptr) {
        return false;
      }
      *dst = *src;
      return true;
    }

    std::string m_type;
    std::string m_underlying;
  } *checker = new SimpleAttributeChecker();

  checker->m_type = name;
  checker->m_underlying = underlying;
  return Ptr<AttributeChecker>(checker, false);
}

} // namespace ns3

#define ATTRIBUTE_ACCESSOR_DEFINE(type)                                        \
  template <typename T1>                                                       \
  Ptr<const AttributeAccessor> Make##type##Accessor(T1 a1) {                   \
    return MakeAccessorHelper<type##Value>(a1);                                \
  }                                                                            \
  template <typename T1, typename T2>                                          \
  Ptr<const AttributeAccessor> Make##type##Accessor(T1 a1, T2 a2) {            \
    return MakeAccessorHelper<type##Value>(a1, a2);                            \
  }

#define ATTRIBUTE_VALUE_DEFINE_WITH_NAME(type, name)                           \
  class name##Value : public AttributeValue {                                  \
  public:                                                                      \
    name##Value();                                                             \
    name##Value(const type &value);                                            \
    void Set(const type &value);                                               \
    type Get() const;                                                          \
    template <typename T> bool GetAccessor(T &value) const {                   \
      value = T(m_value);                                                      \
      return true;                                                             \
    }                                                                          \
    Ptr<AttributeValue> Copy() const override;                                 \
    std::string                                                                \
    SerializeToString(Ptr<const AttributeChecker> checker) const override;     \
    bool DeserializeFromString(std::string value,                              \
                               Ptr<const AttributeChecker> checker) override;  \
                                                                               \
  private:                                                                     \
    type m_value;                                                              \
  }

#define ATTRIBUTE_VALUE_DEFINE(name)                                           \
  ATTRIBUTE_VALUE_DEFINE_WITH_NAME(name, name)

#define ATTRIBUTE_CONVERTER_DEFINE(type)

#define ATTRIBUTE_CHECKER_DEFINE(type)                                         \
  class type##Checker : public AttributeChecker {};                            \
  Ptr<const AttributeChecker> Make##type##Checker()

#define ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME(type, name)                        \
  name##Value::name##Value() : m_value() {}                                    \
  name##Value::name##Value(const type &value) : m_value(value) {}              \
  void name##Value::Set(const type &v) { m_value = v; }                        \
  type name##Value::Get() const { return m_value; }                            \
  Ptr<AttributeValue> name##Value::Copy() const {                              \
    return ns3::Create<name##Value>(*this);                                    \
  }                                                                            \
  std::string name##Value::SerializeToString(                                  \
      Ptr<const AttributeChecker> checker) const {                             \
    std::ostringstream oss;                                                    \
    oss << m_value;                                                            \
    return oss.str();                                                          \
  }                                                                            \
  bool name##Value::DeserializeFromString(                                     \
      std::string value, Ptr<const AttributeChecker> checker) {                \
    std::istringstream iss;                                                    \
    iss.str(value);                                                            \
    iss >> m_value;                                                            \
    NS_ABORT_MSG_UNLESS(iss.eof(), "Attribute value "                          \
                                       << "\"" << value << "\""                \
                                       << " is not properly formatted");       \
    return !iss.bad() && !iss.fail();                                          \
  }

#define ATTRIBUTE_VALUE_IMPLEMENT(type)                                        \
  ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME(type, type)

#define ATTRIBUTE_CHECKER_IMPLEMENT(type)                                      \
  Ptr<const AttributeChecker> Make##type##Checker() {                          \
    return MakeSimpleAttributeChecker<type##Value, type##Checker>(             \
        #type "Value", #type);                                                 \
  }

#define ATTRIBUTE_CHECKER_IMPLEMENT_WITH_NAME(type, name)                      \
  Ptr<const AttributeChecker> Make##type##Checker() {                          \
    return MakeSimpleAttributeChecker<type##Value, type##Checker>(             \
        #type "Value", name);                                                  \
  }

#define ATTRIBUTE_HELPER_HEADER(type)                                          \
  ATTRIBUTE_VALUE_DEFINE(type);                                                \
  ATTRIBUTE_ACCESSOR_DEFINE(type);                                             \
  ATTRIBUTE_CHECKER_DEFINE(type)

#define ATTRIBUTE_HELPER_CPP(type)                                             \
  ATTRIBUTE_CHECKER_IMPLEMENT(type);                                           \
  ATTRIBUTE_VALUE_IMPLEMENT(type)

#endif
