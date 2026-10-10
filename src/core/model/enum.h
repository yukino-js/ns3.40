#ifndef ENUM_VALUE_H
#define ENUM_VALUE_H

#include "attribute-accessor-helper.h"
#include "attribute.h"

#include <list>

namespace ns3 {

class EnumValue : public AttributeValue {
public:
  EnumValue();
  EnumValue(int value);
  void Set(int value);
  int Get() const;
  template <typename T> bool GetAccessor(T &value) const;

  Ptr<AttributeValue> Copy() const override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;

private:
  int m_value;
};

template <typename T> bool EnumValue::GetAccessor(T &value) const {
  value = T(m_value);
  return true;
}

class EnumChecker : public AttributeChecker {
public:
  EnumChecker();

  void AddDefault(int value, std::string name);
  void Add(int value, std::string name);

  std::string GetName(int value) const;

  int GetValue(const std::string name) const;

  bool Check(const AttributeValue &value) const override;
  std::string GetValueTypeName() const override;
  bool HasUnderlyingTypeInformation() const override;
  std::string GetUnderlyingTypeInformation() const override;
  Ptr<AttributeValue> Create() const override;
  bool Copy(const AttributeValue &src, AttributeValue &dst) const override;

private:
  typedef std::pair<int, std::string> Value;
  typedef std::list<Value> ValueSet;
  ValueSet m_valueSet;
};

template <typename T1> Ptr<const AttributeAccessor> MakeEnumAccessor(T1 a1);

template <typename T1, typename T2>
Ptr<const AttributeAccessor> MakeEnumAccessor(T1 a1, T2 a2);

template <typename... Ts>
Ptr<const AttributeChecker> MakeEnumChecker(int v, std::string n, Ts... args) {
  Ptr<EnumChecker> checker = Create<EnumChecker>();
  checker->AddDefault(v, n);
  return MakeEnumChecker(checker, args...);
}

template <typename... Ts>
Ptr<const AttributeChecker> MakeEnumChecker(Ptr<EnumChecker> checker, int v,
                                            std::string n, Ts... args) {
  checker->Add(v, n);
  return MakeEnumChecker(checker, args...);
}

inline Ptr<const AttributeChecker> MakeEnumChecker(Ptr<EnumChecker> checker) {
  return checker;
}

template <typename T1> Ptr<const AttributeAccessor> MakeEnumAccessor(T1 a1) {
  return MakeAccessorHelper<EnumValue>(a1);
}

template <typename T1, typename T2>
Ptr<const AttributeAccessor> MakeEnumAccessor(T1 a1, T2 a2) {
  return MakeAccessorHelper<EnumValue>(a1, a2);
}

} // namespace ns3

#endif
