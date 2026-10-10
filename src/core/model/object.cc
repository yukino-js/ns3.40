
#include "object.h"

#include "assert.h"
#include "attribute.h"
#include "log.h"
#include "object-factory.h"
#include "string.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Object");

NS_OBJECT_ENSURE_REGISTERED(Object);

Object::AggregateIterator::AggregateIterator()
    : m_object(nullptr), m_current(0) {
  NS_LOG_FUNCTION(this);
}

bool Object::AggregateIterator::HasNext() const {
  NS_LOG_FUNCTION(this);
  return m_current < m_object->m_aggregates->n;
}

Ptr<const Object> Object::AggregateIterator::Next() {
  NS_LOG_FUNCTION(this);
  Object *object = m_object->m_aggregates->buffer[m_current];
  m_current++;
  return object;
}

Object::AggregateIterator::AggregateIterator(Ptr<const Object> object)
    : m_object(object), m_current(0) {
  NS_LOG_FUNCTION(this << object);
}

TypeId Object::GetInstanceTypeId() const {
  NS_LOG_FUNCTION(this);
  return m_tid;
}

TypeId Object::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::Object").SetParent<ObjectBase>().SetGroupName("Core");
  return tid;
}

Object::Object()
    : m_tid(Object::GetTypeId()), m_disposed(false), m_initialized(false),
      m_aggregates((Aggregates *)std::malloc(sizeof(Aggregates))),
      m_getObjectCount(0) {
  NS_LOG_FUNCTION(this);
  m_aggregates->n = 1;
  m_aggregates->buffer[0] = this;
}

Object::~Object() {
  NS_LOG_FUNCTION(this);
  uint32_t n = m_aggregates->n;
  for (uint32_t i = 0; i < n; i++) {
    Object *current = m_aggregates->buffer[i];
    if (current == this) {
      std::memmove(&m_aggregates->buffer[i], &m_aggregates->buffer[i + 1],
                   sizeof(Object *) * (m_aggregates->n - (i + 1)));
      m_aggregates->n--;
    }
  }
  if (m_aggregates->n == 0) {
    std::free(m_aggregates);
  }
  m_aggregates = nullptr;
}

Object::Object(const Object &o)
    : m_tid(o.m_tid), m_disposed(false), m_initialized(false),
      m_aggregates((Aggregates *)std::malloc(sizeof(Aggregates))),
      m_getObjectCount(0) {
  m_aggregates->n = 1;
  m_aggregates->buffer[0] = this;
}

void Object::Construct(const AttributeConstructionList &attributes) {
  NS_LOG_FUNCTION(this << &attributes);
  ConstructSelf(attributes);
}

Ptr<Object> Object::DoGetObject(TypeId tid) const {
  NS_LOG_FUNCTION(this << tid);
  NS_ASSERT(CheckLoose());

  uint32_t n = m_aggregates->n;
  TypeId objectTid = Object::GetTypeId();
  for (uint32_t i = 0; i < n; i++) {
    Object *current = m_aggregates->buffer[i];
    TypeId cur = current->GetInstanceTypeId();
    while (cur != tid && cur != objectTid) {
      cur = cur.GetParent();
    }
    if (cur == tid) {
#ifndef NS3_MTP

      current->m_getObjectCount++;
      UpdateSortedArray(m_aggregates, i);
#endif
      return const_cast<Object *>(current);
    }
  }
  return nullptr;
}

void Object::Initialize() {
  NS_LOG_FUNCTION(this);
restart:
  uint32_t n = m_aggregates->n;
  for (uint32_t i = 0; i < n; i++) {
    Object *current = m_aggregates->buffer[i];
    if (!current->m_initialized) {
      current->DoInitialize();
      current->m_initialized = true;
      goto restart;
    }
  }
}

bool Object::IsInitialized() const {
  NS_LOG_FUNCTION(this);
  return m_initialized;
}

void Object::Dispose() {
  NS_LOG_FUNCTION(this);
restart:
  uint32_t n = m_aggregates->n;
  for (uint32_t i = 0; i < n; i++) {
    Object *current = m_aggregates->buffer[i];
    if (!current->m_disposed) {
      current->DoDispose();
      current->m_disposed = true;
      goto restart;
    }
  }
}

void Object::UpdateSortedArray(Aggregates *aggregates, uint32_t j) const {
  NS_LOG_FUNCTION(this << aggregates << j);
  while (j > 0 && aggregates->buffer[j]->m_getObjectCount >
                      aggregates->buffer[j - 1]->m_getObjectCount) {
    Object *tmp = aggregates->buffer[j - 1];
    aggregates->buffer[j - 1] = aggregates->buffer[j];
    aggregates->buffer[j] = tmp;
    j--;
  }
}

void Object::AggregateObject(Ptr<Object> o) {
  NS_LOG_FUNCTION(this << o);
  NS_ASSERT(!m_disposed);
  NS_ASSERT(!o->m_disposed);
  NS_ASSERT(CheckLoose());
  NS_ASSERT(o->CheckLoose());

  Object *other = PeekPointer(o);
  uint32_t total = m_aggregates->n + other->m_aggregates->n;
  auto aggregates = (Aggregates *)std::malloc(sizeof(Aggregates) +
                                              (total - 1) * sizeof(Object *));
  aggregates->n = total;

  std::memcpy(&aggregates->buffer[0], &m_aggregates->buffer[0],
              m_aggregates->n * sizeof(Object *));

  for (uint32_t i = 0; i < other->m_aggregates->n; i++) {
    aggregates->buffer[m_aggregates->n + i] = other->m_aggregates->buffer[i];
    const TypeId typeId = other->m_aggregates->buffer[i]->GetInstanceTypeId();
    if (DoGetObject(typeId)) {
      NS_FATAL_ERROR("Object::AggregateObject(): "
                     "Multiple aggregation of objects of type "
                     << other->GetInstanceTypeId() << " on objects of type "
                     << typeId);
    }
    UpdateSortedArray(aggregates, m_aggregates->n + i);
  }

  Aggregates *a = m_aggregates;
  Aggregates *b = other->m_aggregates;

  uint32_t n = aggregates->n;
  for (uint32_t i = 0; i < n; i++) {
    Object *current = aggregates->buffer[i];
    current->m_aggregates = aggregates;
  }

  for (uint32_t i = 0; i < a->n; i++) {
    Object *current = a->buffer[i];
    current->NotifyNewAggregate();
  }
  for (uint32_t i = 0; i < b->n; i++) {
    Object *current = b->buffer[i];
    current->NotifyNewAggregate();
  }

  std::free(a);
  std::free(b);
}

void Object::NotifyNewAggregate() { NS_LOG_FUNCTION(this); }

Object::AggregateIterator Object::GetAggregateIterator() const {
  NS_LOG_FUNCTION(this);
  return AggregateIterator(this);
}

void Object::SetTypeId(TypeId tid) {
  NS_LOG_FUNCTION(this << tid);
  NS_ASSERT(Check());
  m_tid = tid;
}

void Object::DoDispose() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(!m_disposed);
}

void Object::DoInitialize() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(!m_initialized);
}

bool Object::Check() const {
  NS_LOG_FUNCTION(this);
  return (GetReferenceCount() > 0);
}

bool Object::CheckLoose() const {
  NS_LOG_FUNCTION(this);
  bool nonZeroRefCount = false;
  uint32_t n = m_aggregates->n;
  for (uint32_t i = 0; i < n; i++) {
    Object *current = m_aggregates->buffer[i];
    if (current->GetReferenceCount()) {
      nonZeroRefCount = true;
      break;
    }
  }
  return nonZeroRefCount;
}

void Object::DoDelete() {
  NS_LOG_FUNCTION(this);
  for (uint32_t i = 0; i < m_aggregates->n; i++) {
    Object *current = m_aggregates->buffer[i];
    if (current->GetReferenceCount() > 0) {
      return;
    }
  }

  uint32_t n = m_aggregates->n;
  for (uint32_t i = 0; i < n; i++) {
    Object *current = m_aggregates->buffer[i];
    if (!current->m_disposed) {
      current->DoDispose();
    }
  }

  Aggregates *aggregates = m_aggregates;
  for (uint32_t i = 0; i < n; i++) {
    Object *current = aggregates->buffer[0];
    delete current;
  }
}
} // namespace ns3
