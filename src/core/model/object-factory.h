#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "attribute-construction-list.h"
#include "object.h"
#include "type-id.h"

namespace ns3 {

class AttributeValue;

class ObjectFactory {
public:
  ObjectFactory();
  template <typename... Args>
  ObjectFactory(const std::string &typeId, Args &&...args);

  void SetTypeId(TypeId tid);
  void SetTypeId(std::string tid);

  bool IsTypeIdSet() const;

  template <typename... Args>
  void Set(const std::string &name, const AttributeValue &value,
           Args &&...args);

  void Set() {}

  TypeId GetTypeId() const;

  Ptr<Object> Create() const;
  template <typename T> Ptr<T> Create() const;

private:
  void DoSet(const std::string &name, const AttributeValue &value);
  friend std::ostream &operator<<(std::ostream &os,
                                  const ObjectFactory &factory);
  friend std::istream &operator>>(std::istream &is, ObjectFactory &factory);

  TypeId m_tid;
  AttributeConstructionList m_parameters;
};

std::ostream &operator<<(std::ostream &os, const ObjectFactory &factory);
std::istream &operator>>(std::istream &is, ObjectFactory &factory);

template <typename T, typename... Args>
Ptr<T> CreateObjectWithAttributes(Args... args);

ATTRIBUTE_HELPER_HEADER(ObjectFactory);

} // namespace ns3

namespace ns3 {

template <typename T> Ptr<T> ObjectFactory::Create() const {
  Ptr<Object> object = Create();
  return object->GetObject<T>();
}

template <typename... Args>
ObjectFactory::ObjectFactory(const std::string &typeId, Args &&...args) {
  SetTypeId(typeId);
  Set(args...);
}

template <typename... Args>
void ObjectFactory::Set(const std::string &name, const AttributeValue &value,
                        Args &&...args) {
  DoSet(name, value);
  Set(args...);
}

template <typename T, typename... Args>
Ptr<T> CreateObjectWithAttributes(Args... args) {
  ObjectFactory factory;
  factory.SetTypeId(T::GetTypeId());
  factory.Set(args...);
  return factory.Create<T>();
}

} // namespace ns3

#endif
