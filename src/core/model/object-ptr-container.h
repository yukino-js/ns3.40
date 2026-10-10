#ifndef OBJECT_PTR_CONTAINER_H
#define OBJECT_PTR_CONTAINER_H

#include "attribute.h"
#include "object.h"
#include "ptr.h"

#include <map>

namespace ns3 {

class ObjectPtrContainerValue : public AttributeValue {
public:
  typedef std::map<std::size_t, Ptr<Object>>::const_iterator Iterator;

  ObjectPtrContainerValue();

  Iterator Begin() const;
  Iterator End() const;
  std::size_t GetN() const;
  Ptr<Object> Get(std::size_t i) const;

  Ptr<AttributeValue> Copy() const override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;

private:
  friend class ObjectPtrContainerAccessor;
  std::map<std::size_t, Ptr<Object>> m_objects;
};

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor>
MakeObjectPtrContainerAccessor(Ptr<U> (T::*get)(INDEX) const,
                               INDEX (T::*getN)() const);

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor>
MakeObjectPtrContainerAccessor(INDEX (T::*getN)() const,
                               Ptr<U> (T::*get)(INDEX) const);

class ObjectPtrContainerChecker : public AttributeChecker {
public:
  virtual TypeId GetItemTypeId() const = 0;
};

template <typename T>
Ptr<const AttributeChecker> MakeObjectPtrContainerChecker();

} // namespace ns3

namespace ns3 {

namespace internal {

template <typename T>
class ObjectPtrContainerChecker : public ns3::ObjectPtrContainerChecker {
public:
  TypeId GetItemTypeId() const override { return T::GetTypeId(); }

  bool Check(const AttributeValue &value) const override {
    return dynamic_cast<const ObjectPtrContainerValue *>(&value) != nullptr;
  }

  std::string GetValueTypeName() const override {
    return "ns3::ObjectPtrContainerValue";
  }

  bool HasUnderlyingTypeInformation() const override { return true; }

  std::string GetUnderlyingTypeInformation() const override {
    return "ns3::Ptr< " + T::GetTypeId().GetName() + " >";
  }

  Ptr<AttributeValue> Create() const override {
    return ns3::Create<ObjectPtrContainerValue>();
  }

  bool Copy(const AttributeValue &source,
            AttributeValue &destination) const override {
    const auto src = dynamic_cast<const ObjectPtrContainerValue *>(&source);
    auto dst = dynamic_cast<ObjectPtrContainerValue *>(&destination);
    if (src == nullptr || dst == nullptr) {
      return false;
    }
    *dst = *src;
    return true;
  }
};

} // namespace internal

class ObjectPtrContainerAccessor : public AttributeAccessor {
public:
  bool Set(ObjectBase *object, const AttributeValue &value) const override;
  bool Get(const ObjectBase *object, AttributeValue &value) const override;
  bool HasGetter() const override;
  bool HasSetter() const override;

private:
  virtual bool DoGetN(const ObjectBase *object, std::size_t *n) const = 0;
  virtual Ptr<Object> DoGet(const ObjectBase *object, std::size_t i,
                            std::size_t *index) const = 0;
};

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor>
MakeObjectPtrContainerAccessor(Ptr<U> (T::*get)(INDEX) const,
                               INDEX (T::*getN)() const) {
  struct MemberGetters : public ObjectPtrContainerAccessor {
    bool DoGetN(const ObjectBase *object, std::size_t *n) const override {
      const T *obj = dynamic_cast<const T *>(object);
      if (obj == nullptr) {
        return false;
      }
      *n = (obj->*m_getN)();
      return true;
    }

    Ptr<Object> DoGet(const ObjectBase *object, std::size_t i,
                      std::size_t *index) const override {
      const T *obj = static_cast<const T *>(object);
      *index = i;
      return (obj->*m_get)(i);
    }

    Ptr<U> (T::*m_get)(INDEX) const;
    INDEX (T::*m_getN)() const;
  } *spec = new MemberGetters();

  spec->m_get = get;
  spec->m_getN = getN;
  return Ptr<const AttributeAccessor>(spec, false);
}

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor>
MakeObjectPtrContainerAccessor(INDEX (T::*getN)() const,
                               Ptr<U> (T::*get)(INDEX) const) {
  return MakeObjectPtrContainerAccessor(get, getN);
}

template <typename T>
Ptr<const AttributeChecker> MakeObjectPtrContainerChecker() {
  return Create<internal::ObjectPtrContainerChecker<T>>();
}

} // namespace ns3

#endif
