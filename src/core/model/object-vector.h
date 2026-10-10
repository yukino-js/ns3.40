#ifndef OBJECT_VECTOR_H
#define OBJECT_VECTOR_H

#include "attribute.h"
#include "object-ptr-container.h"
#include "object.h"
#include "ptr.h"

namespace ns3 {

typedef ObjectPtrContainerValue ObjectVectorValue;

template <typename T, typename U>
Ptr<const AttributeAccessor> MakeObjectVectorAccessor(U T::*memberVariable);

template <typename T> Ptr<const AttributeChecker> MakeObjectVectorChecker();

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor> MakeObjectVectorAccessor(Ptr<U> (T::*get)(INDEX)
                                                          const,
                                                      INDEX (T::*getN)() const);

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor> MakeObjectVectorAccessor(INDEX (T::*getN)() const,
                                                      Ptr<U> (T::*get)(INDEX)
                                                          const);

template <typename T, typename U>
Ptr<const AttributeAccessor> MakeObjectVectorAccessor(U T::*memberVector) {
  struct MemberStdContainer : public ObjectPtrContainerAccessor {
    bool DoGetN(const ObjectBase *object, std::size_t *n) const override {
      const T *obj = dynamic_cast<const T *>(object);
      if (obj == nullptr) {
        return false;
      }
      *n = (obj->*m_memberVector).size();
      return true;
    }

    Ptr<Object> DoGet(const ObjectBase *object, std::size_t i,
                      std::size_t *index) const override {
      const T *obj = static_cast<const T *>(object);
      auto begin = (obj->*m_memberVector).begin();
      auto end = (obj->*m_memberVector).end();
      std::size_t k = 0;
      for (auto j = begin; j != end; j++, k++) {
        if (k == i) {
          *index = k;
          return *j;
        }
      }
      NS_ASSERT(false);
      return nullptr;
    }

    U T::*m_memberVector;
  } *spec = new MemberStdContainer();

  spec->m_memberVector = memberVector;
  return Ptr<const AttributeAccessor>(spec, false);
}

template <typename T> Ptr<const AttributeChecker> MakeObjectVectorChecker() {
  return MakeObjectPtrContainerChecker<T>();
}

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor>
MakeObjectVectorAccessor(Ptr<U> (T::*get)(INDEX) const,
                         INDEX (T::*getN)() const) {
  return MakeObjectPtrContainerAccessor<T, U, INDEX>(get, getN);
}

template <typename T, typename U, typename INDEX>
Ptr<const AttributeAccessor> MakeObjectVectorAccessor(INDEX (T::*getN)() const,
                                                      Ptr<U> (T::*get)(INDEX)
                                                          const) {
  return MakeObjectPtrContainerAccessor<T, U, INDEX>(get, getN);
}

} // namespace ns3

#endif
