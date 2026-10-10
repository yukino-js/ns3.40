#ifndef OBJECT_H
#define OBJECT_H

#include "attribute-construction-list.h"
#include "attribute.h"
#include "object-base.h"
#include "ptr.h"
#include "simple-ref-count.h"

#include <stdint.h>
#include <string>
#include <vector>

namespace ns3 {

class Object;
class AttributeAccessor;
class AttributeValue;
class TraceSourceAccessor;

struct ObjectDeleter {
  inline static void Delete(Object *object);
};

class Object : public SimpleRefCount<Object, ObjectBase, ObjectDeleter> {
public:
  static TypeId GetTypeId();

  class AggregateIterator {
  public:
    AggregateIterator();

    bool HasNext() const;

    Ptr<const Object> Next();

  private:
    friend class Object;
    AggregateIterator(Ptr<const Object> object);
    Ptr<const Object> m_object;
    uint32_t m_current;
  };

  Object();
  ~Object() override;

  TypeId GetInstanceTypeId() const override;

  template <typename T> inline Ptr<T> GetObject() const;
  template <typename T> Ptr<T> GetObject(TypeId tid) const;
  void Dispose();
  void AggregateObject(Ptr<Object> other);

  AggregateIterator GetAggregateIterator() const;

  void Initialize();

  bool IsInitialized() const;

protected:
  virtual void NotifyNewAggregate();
  virtual void DoInitialize();
  virtual void DoDispose();
  Object(const Object &o);

private:
  template <typename T> friend Ptr<T> CopyObject(Ptr<T> object);
  template <typename T> friend Ptr<T> CopyObject(Ptr<const T> object);

  template <typename T> friend Ptr<T> CompleteConstruct(T *object);

  friend class ObjectFactory;
  friend class AggregateIterator;
  friend struct ObjectDeleter;

  struct Aggregates {
    uint32_t n;
    Object *buffer[1];
  };

  Ptr<Object> DoGetObject(TypeId tid) const;
  bool Check() const;
  bool CheckLoose() const;
  void SetTypeId(TypeId tid);
  void Construct(const AttributeConstructionList &attributes);

  void UpdateSortedArray(Aggregates *aggregates, uint32_t i) const;
  void DoDelete();

  TypeId m_tid;
  bool m_disposed;
  bool m_initialized;
  Aggregates *m_aggregates;
  uint32_t m_getObjectCount;
};

template <typename T> Ptr<T> CopyObject(Ptr<const T> object);
template <typename T> Ptr<T> CopyObject(Ptr<T> object);

} // namespace ns3

namespace ns3 {

void ObjectDeleter::Delete(Object *object) { object->DoDelete(); }

template <typename T> Ptr<T> Object::GetObject() const {
  T *result = dynamic_cast<T *>(m_aggregates->buffer[0]);
  if (result != nullptr) {
    return Ptr<T>(result);
  }
  Ptr<Object> found = DoGetObject(T::GetTypeId());
  if (found) {
    return Ptr<T>(static_cast<T *>(PeekPointer(found)));
  }
  return nullptr;
}

template <> inline Ptr<Object> Object::GetObject() const {
  return Ptr<Object>(const_cast<Object *>(this));
}

template <typename T> Ptr<T> Object::GetObject(TypeId tid) const {
  Ptr<Object> found = DoGetObject(tid);
  if (found) {
    return Ptr<T>(static_cast<T *>(PeekPointer(found)));
  }
  return nullptr;
}

template <> inline Ptr<Object> Object::GetObject(TypeId tid) const {
  if (tid == Object::GetTypeId()) {
    return Ptr<Object>(const_cast<Object *>(this));
  } else {
    return DoGetObject(tid);
  }
}

template <typename T> Ptr<T> CopyObject(Ptr<T> object) {
  Ptr<T> p = Ptr<T>(new T(*PeekPointer(object)), false);
  NS_ASSERT(p->GetInstanceTypeId() == object->GetInstanceTypeId());
  return p;
}

template <typename T> Ptr<T> CopyObject(Ptr<const T> object) {
  Ptr<T> p = Ptr<T>(new T(*PeekPointer(object)), false);
  NS_ASSERT(p->GetInstanceTypeId() == object->GetInstanceTypeId());
  return p;
}

template <typename T> Ptr<T> CompleteConstruct(T *object) {
  object->SetTypeId(T::GetTypeId());
  object->Object::Construct(AttributeConstructionList());
  return Ptr<T>(object, false);
}

template <typename T, typename... Args> Ptr<T> CreateObject(Args &&...args) {
  return CompleteConstruct(new T(std::forward<Args>(args)...));
}

} // namespace ns3

#endif
