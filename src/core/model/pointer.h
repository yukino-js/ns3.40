#ifndef NS_POINTER_H
#define NS_POINTER_H

#include "attribute.h"
#include "object.h"

namespace ns3 {

class PointerValue : public AttributeValue {
public:
  PointerValue();

  PointerValue(Ptr<Object> object);

  void SetObject(Ptr<Object> object);

  Ptr<Object> GetObject() const;

  template <typename T> PointerValue(const Ptr<T> &object);

  template <typename T> operator Ptr<T>() const;

  template <typename T> void Set(const Ptr<T> &value);

  template <typename T> Ptr<T> Get() const;

  template <typename T> bool GetAccessor(Ptr<T> &value) const;

  Ptr<AttributeValue> Copy() const override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;

private:
  Ptr<Object> m_value;
};

class PointerChecker : public AttributeChecker {
public:
  virtual TypeId GetPointeeTypeId() const = 0;
};

template <typename T> Ptr<AttributeChecker> MakePointerChecker();

} // namespace ns3

namespace ns3 {

namespace internal {

template <typename T> class PointerChecker : public ns3::PointerChecker {
  bool Check(const AttributeValue &val) const override {
    const auto value = dynamic_cast<const PointerValue *>(&val);
    if (value == nullptr) {
      return false;
    }
    if (!value->GetObject()) {
      return true;
    }
    T *ptr = dynamic_cast<T *>(PeekPointer(value->GetObject()));
    return ptr;
  }

  std::string GetValueTypeName() const override { return "ns3::PointerValue"; }

  bool HasUnderlyingTypeInformation() const override { return true; }

  std::string GetUnderlyingTypeInformation() const override {
    TypeId tid = T::GetTypeId();
    return "ns3::Ptr< " + tid.GetName() + " >";
  }

  Ptr<AttributeValue> Create() const override {
    return ns3::Create<PointerValue>();
  }

  bool Copy(const AttributeValue &source,
            AttributeValue &destination) const override {
    const auto src = dynamic_cast<const PointerValue *>(&source);
    auto dst = dynamic_cast<PointerValue *>(&destination);
    if (src == nullptr || dst == nullptr) {
      return false;
    }
    *dst = *src;
    return true;
  }

  TypeId GetPointeeTypeId() const override { return T::GetTypeId(); }
};

} // namespace internal

template <typename T> PointerValue::PointerValue(const Ptr<T> &object) {
  m_value = object;
}

template <typename T> void PointerValue::Set(const Ptr<T> &object) {
  m_value = object;
}

template <typename T> Ptr<T> PointerValue::Get() const {
  T *v = dynamic_cast<T *>(PeekPointer(m_value));
  return v;
}

template <typename T> PointerValue::operator Ptr<T>() const { return Get<T>(); }

template <typename T> bool PointerValue::GetAccessor(Ptr<T> &v) const {
  Ptr<T> ptr = dynamic_cast<T *>(PeekPointer(m_value));
  if (!ptr) {
    return false;
  }
  v = ptr;
  return true;
}

ATTRIBUTE_ACCESSOR_DEFINE(Pointer);

template <typename T> Ptr<AttributeChecker> MakePointerChecker() {
  return Create<internal::PointerChecker<T>>();
}

} // namespace ns3

#endif
