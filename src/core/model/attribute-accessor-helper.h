#ifndef ATTRIBUTE_ACCESSOR_HELPER_H
#define ATTRIBUTE_ACCESSOR_HELPER_H

#include "attribute.h"

namespace ns3 {

template <typename V, typename T1>
inline Ptr<const AttributeAccessor> MakeAccessorHelper(T1 a1);

template <typename V, typename T1, typename T2>
inline Ptr<const AttributeAccessor> MakeAccessorHelper(T1 a1, T2 a2);

} // namespace ns3

#include "type-traits.h"

namespace ns3 {

template <typename T> struct AccessorTrait {
  typedef
      typename TypeTraits<typename TypeTraits<T>::ReferencedType>::NonConstType
          Result;
};

template <typename T, typename U>
class AccessorHelper : public AttributeAccessor {
public:
  AccessorHelper() {}

  bool Set(ObjectBase *object, const AttributeValue &val) const override {
    const U *value = dynamic_cast<const U *>(&val);
    if (value == nullptr) {
      return false;
    }
    T *obj = dynamic_cast<T *>(object);
    if (obj == nullptr) {
      return false;
    }
    return DoSet(obj, value);
  }

  bool Get(const ObjectBase *object, AttributeValue &val) const override {
    U *value = dynamic_cast<U *>(&val);
    if (value == nullptr) {
      return false;
    }
    const T *obj = dynamic_cast<const T *>(object);
    if (obj == nullptr) {
      return false;
    }
    return DoGet(obj, value);
  }

private:
  virtual bool DoSet(T *object, const U *v) const = 0;
  virtual bool DoGet(const T *object, U *v) const = 0;
};

template <typename V, typename T, typename U>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperOne(U T::*memberVariable) {
  class MemberVariable : public AccessorHelper<T, V> {
  public:
    MemberVariable(U T::*memberVariable)
        : AccessorHelper<T, V>(), m_memberVariable(memberVariable) {}

  private:
    bool DoSet(T *object, const V *v) const override {
      typename AccessorTrait<U>::Result tmp;
      bool ok = v->GetAccessor(tmp);
      if (!ok) {
        return false;
      }
      (object->*m_memberVariable) = tmp;
      return true;
    }

    bool DoGet(const T *object, V *v) const override {
      v->Set(object->*m_memberVariable);
      return true;
    }

    bool HasGetter() const override { return true; }

    bool HasSetter() const override { return true; }

    U T::*m_memberVariable;
  };

  return Ptr<const AttributeAccessor>(new MemberVariable(memberVariable),
                                      false);
}

template <typename V, typename T, typename U>
inline Ptr<const AttributeAccessor> DoMakeAccessorHelperOne(U (T::*getter)()
                                                                const) {
  class MemberMethod : public AccessorHelper<T, V> {
  public:
    MemberMethod(U (T::*getter)() const)
        : AccessorHelper<T, V>(), m_getter(getter) {}

  private:
    bool DoSet(T *, const V *) const override { return false; }

    bool DoGet(const T *object, V *v) const override {
      v->Set((object->*m_getter)());
      return true;
    }

    bool HasGetter() const override { return true; }

    bool HasSetter() const override { return false; }

    U (T::*m_getter)() const;
  };

  return Ptr<const AttributeAccessor>(new MemberMethod(getter), false);
}

template <typename V, typename T, typename U>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperOne(void (T::*setter)(U)) {
  class MemberMethod : public AccessorHelper<T, V> {
  public:
    MemberMethod(void (T::*setter)(U))
        : AccessorHelper<T, V>(), m_setter(setter) {}

  private:
    bool DoSet(T *object, const V *v) const override {
      typename AccessorTrait<U>::Result tmp;
      bool ok = v->GetAccessor(tmp);
      if (!ok) {
        return false;
      }
      (object->*m_setter)(tmp);
      return true;
    }

    bool DoGet(const T *, V *) const override { return false; }

    bool HasGetter() const override { return false; }

    bool HasSetter() const override { return true; }

    void (T::*m_setter)(U);
  };

  return Ptr<const AttributeAccessor>(new MemberMethod(setter), false);
}

template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(void (T::*setter)(U), V (T::*getter)() const) {
  class MemberMethod : public AccessorHelper<T, W> {
  public:
    MemberMethod(void (T::*setter)(U), V (T::*getter)() const)
        : AccessorHelper<T, W>(), m_setter(setter), m_getter(getter) {}

  private:
    bool DoSet(T *object, const W *v) const override {
      typename AccessorTrait<U>::Result tmp;
      bool ok = v->GetAccessor(tmp);
      if (!ok) {
        return false;
      }
      (object->*m_setter)(tmp);
      return true;
    }

    bool DoGet(const T *object, W *v) const override {
      v->Set((object->*m_getter)());
      return true;
    }

    bool HasGetter() const override { return true; }

    bool HasSetter() const override { return true; }

    void (T::*m_setter)(U);
    V (T::*m_getter)() const;
  };

  return Ptr<const AttributeAccessor>(new MemberMethod(setter, getter), false);
}

template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(V (T::*getter)() const, void (T::*setter)(U)) {
  return DoMakeAccessorHelperTwo<W>(setter, getter);
}

template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(bool (T::*setter)(U), V (T::*getter)() const) {
  class MemberMethod : public AccessorHelper<T, W> {
  public:
    MemberMethod(bool (T::*setter)(U), V (T::*getter)() const)
        : AccessorHelper<T, W>(), m_setter(setter), m_getter(getter) {}

  private:
    bool DoSet(T *object, const W *v) const override {
      typename AccessorTrait<U>::Result tmp;
      bool ok = v->GetAccessor(tmp);
      if (!ok) {
        return false;
      }
      ok = (object->*m_setter)(tmp);
      return ok;
    }

    bool DoGet(const T *object, W *v) const override {
      v->Set((object->*m_getter)());
      return true;
    }

    bool HasGetter() const override { return true; }

    bool HasSetter() const override { return true; }

    bool (T::*m_setter)(U);
    V (T::*m_getter)() const;
  };

  return Ptr<const AttributeAccessor>(new MemberMethod(setter, getter), false);
}

template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(V (T::*getter)() const, bool (T::*setter)(U)) {
  return DoMakeAccessorHelperTwo<W>(setter, getter);
}

template <typename V, typename T1>
inline Ptr<const AttributeAccessor> MakeAccessorHelper(T1 a1) {
  return DoMakeAccessorHelperOne<V>(a1);
}

template <typename V, typename T1, typename T2>
inline Ptr<const AttributeAccessor> MakeAccessorHelper(T1 a1, T2 a2) {
  return DoMakeAccessorHelperTwo<V>(a1, a2);
}

} // namespace ns3

#endif
