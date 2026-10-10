
#ifndef PTR_H
#define PTR_H

#include "assert.h"
#include "deprecated.h"

#include <iostream>
#include <stdint.h>

namespace ns3 {

template <typename T> class Ptr {
private:
  T *m_ptr;

  class Tester {
  public:
    void operator delete(void *) = delete;
  };

  friend class Ptr<const T>;

  template <typename U> friend U *GetPointer(const Ptr<U> &p);
  template <typename U> friend U *PeekPointer(const Ptr<U> &p);

  inline void Acquire() const;

public:
  Ptr();
  Ptr(T *ptr);
  Ptr(T *ptr, bool ref);
  Ptr(const Ptr &o);
  template <typename U> Ptr(const Ptr<U> &o);
  ~Ptr();
  Ptr<T> &operator=(const Ptr &o);
  T *operator->() const;
  T *operator->();
  T &operator*() const;
  T &operator*();

  NS_DEPRECATED_3_37("see operator bool")
  operator Tester *() const;
  explicit operator bool() const;
};

template <typename T, typename... Ts> Ptr<T> Create(Ts &&...args);

template <typename T>
std::ostream &operator<<(std::ostream &os, const Ptr<T> &p);

template <typename T1, typename T2>
bool operator==(const Ptr<T1> &lhs, T2 const *rhs);

template <typename T1, typename T2>
bool operator==(T1 const *lhs, Ptr<T2> &rhs);

template <typename T1, typename T2>
bool operator==(const Ptr<T1> &lhs, const Ptr<T2> &rhs);

template <typename T1, typename T2>
std::enable_if_t<std::is_same_v<T2, std::nullptr_t>, bool>
operator==(const Ptr<T1> &lhs, T2 rhs);

template <typename T1, typename T2>
bool operator!=(const Ptr<T1> &lhs, T2 const *rhs);

template <typename T1, typename T2>
bool operator!=(T1 const *lhs, Ptr<T2> &rhs);

template <typename T1, typename T2>
bool operator!=(const Ptr<T1> &lhs, const Ptr<T2> &rhs);

template <typename T1, typename T2>
std::enable_if_t<std::is_same_v<T2, std::nullptr_t>, bool>
operator!=(const Ptr<T1> &lhs, T2 rhs);

template <typename T> bool operator<(const Ptr<T> &lhs, const Ptr<T> &rhs);
template <typename T>
bool operator<(const Ptr<T> &lhs, const Ptr<const T> &rhs);
template <typename T>
bool operator<(const Ptr<const T> &lhs, const Ptr<T> &rhs);
template <typename T> bool operator<=(const Ptr<T> &lhs, const Ptr<T> &rhs);
template <typename T> bool operator>(const Ptr<T> &lhs, const Ptr<T> &rhs);
template <typename T> bool operator>=(const Ptr<T> &lhs, const Ptr<T> &rhs);

template <typename T1, typename T2>
Ptr<T1> const_pointer_cast(const Ptr<T2> &p);

template <typename T> struct CallbackTraits;

template <typename T> struct CallbackTraits<Ptr<T>> {
  static T &GetReference(const Ptr<T> p) { return *PeekPointer(p); }
};

template <typename T> struct EventMemberImplObjTraits;

template <typename T> struct EventMemberImplObjTraits<Ptr<T>> {
  static T &GetReference(Ptr<T> p) { return *PeekPointer(p); }
};

} // namespace ns3

namespace ns3 {

template <typename T, typename... Ts> Ptr<T> Create(Ts &&...args) {
  return Ptr<T>(new T(std::forward<Ts>(args)...), false);
}

template <typename U> U *PeekPointer(const Ptr<U> &p) { return p.m_ptr; }

template <typename U> U *GetPointer(const Ptr<U> &p) {
  p.Acquire();
  return p.m_ptr;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Ptr<T> &p) {
  os << PeekPointer(p);
  return os;
}

template <typename T1, typename T2>
bool operator==(const Ptr<T1> &lhs, T2 const *rhs) {
  return PeekPointer(lhs) == rhs;
}

template <typename T1, typename T2>
bool operator==(T1 const *lhs, Ptr<T2> &rhs) {
  return lhs == PeekPointer(rhs);
}

template <typename T1, typename T2>
bool operator!=(const Ptr<T1> &lhs, T2 const *rhs) {
  return PeekPointer(lhs) != rhs;
}

template <typename T1, typename T2>
bool operator!=(T1 const *lhs, Ptr<T2> &rhs) {
  return lhs != PeekPointer(rhs);
}

template <typename T1, typename T2>
bool operator==(const Ptr<T1> &lhs, const Ptr<T2> &rhs) {
  return PeekPointer(lhs) == PeekPointer(rhs);
}

template <typename T1, typename T2>
bool operator!=(const Ptr<T1> &lhs, const Ptr<T2> &rhs) {
  return PeekPointer(lhs) != PeekPointer(rhs);
}

template <typename T1, typename T2>
std::enable_if_t<std::is_same_v<T2, std::nullptr_t>, bool>
operator==(const Ptr<T1> &lhs, T2 rhs) {
  return PeekPointer(lhs) == nullptr;
}

template <typename T1, typename T2>
std::enable_if_t<std::is_same_v<T2, std::nullptr_t>, bool>
operator!=(const Ptr<T1> &lhs, T2 rhs) {
  return PeekPointer(lhs) != nullptr;
}

template <typename T> bool operator<(const Ptr<T> &lhs, const Ptr<T> &rhs) {
  return PeekPointer<T>(lhs) < PeekPointer<T>(rhs);
}

template <typename T>
bool operator<(const Ptr<T> &lhs, const Ptr<const T> &rhs) {
  return PeekPointer<T>(lhs) < PeekPointer<const T>(rhs);
}

template <typename T>
bool operator<(const Ptr<const T> &lhs, const Ptr<T> &rhs) {
  return PeekPointer<const T>(lhs) < PeekPointer<T>(rhs);
}

template <typename T> bool operator<=(const Ptr<T> &lhs, const Ptr<T> &rhs) {
  return PeekPointer<T>(lhs) <= PeekPointer<T>(rhs);
}

template <typename T> bool operator>(const Ptr<T> &lhs, const Ptr<T> &rhs) {
  return PeekPointer<T>(lhs) > PeekPointer<T>(rhs);
}

template <typename T> bool operator>=(const Ptr<T> &lhs, const Ptr<T> &rhs) {
  return PeekPointer<T>(lhs) >= PeekPointer<T>(rhs);
}

template <typename T1, typename T2> Ptr<T1> ConstCast(const Ptr<T2> &p) {
  return Ptr<T1>(const_cast<T1 *>(PeekPointer(p)));
}

template <typename T1, typename T2> Ptr<T1> DynamicCast(const Ptr<T2> &p) {
  return Ptr<T1>(dynamic_cast<T1 *>(PeekPointer(p)));
}

template <typename T1, typename T2> Ptr<T1> StaticCast(const Ptr<T2> &p) {
  return Ptr<T1>(static_cast<T1 *>(PeekPointer(p)));
}

template <typename T> Ptr<T> Copy(Ptr<T> object) {
  Ptr<T> p = Ptr<T>(new T(*PeekPointer(object)), false);
  return p;
}

template <typename T> Ptr<T> Copy(Ptr<const T> object) {
  Ptr<T> p = Ptr<T>(new T(*PeekPointer(object)), false);
  return p;
}

template <typename T> void Ptr<T>::Acquire() const {
  if (m_ptr != nullptr) {
    m_ptr->Ref();
  }
}

template <typename T> Ptr<T>::Ptr() : m_ptr(nullptr) {}

template <typename T> Ptr<T>::Ptr(T *ptr) : m_ptr(ptr) { Acquire(); }

template <typename T> Ptr<T>::Ptr(T *ptr, bool ref) : m_ptr(ptr) {
  if (ref) {
    Acquire();
  }
}

template <typename T> Ptr<T>::Ptr(const Ptr &o) : m_ptr(nullptr) {
  T *ptr = PeekPointer(o);
  if (ptr != nullptr) {
    m_ptr = ptr;
    Acquire();
  }
}

template <typename T>
template <typename U>
Ptr<T>::Ptr(const Ptr<U> &o) : m_ptr(PeekPointer(o)) {
  Acquire();
}

template <typename T> Ptr<T>::~Ptr() {
  if (m_ptr != nullptr) {
    m_ptr->Unref();
  }
}

template <typename T> Ptr<T> &Ptr<T>::operator=(const Ptr &o) {
  if (&o == this) {
    return *this;
  }
  if (m_ptr != nullptr) {
    m_ptr->Unref();
  }
  m_ptr = o.m_ptr;
  Acquire();
  return *this;
}

template <typename T> T *Ptr<T>::operator->() {
  NS_ASSERT_MSG(m_ptr, "Attempted to dereference zero pointer");
  return m_ptr;
}

template <typename T> T *Ptr<T>::operator->() const {
  NS_ASSERT_MSG(m_ptr, "Attempted to dereference zero pointer");
  return m_ptr;
}

template <typename T> T &Ptr<T>::operator*() const {
  NS_ASSERT_MSG(m_ptr, "Attempted to dereference zero pointer");
  return *m_ptr;
}

template <typename T> T &Ptr<T>::operator*() {
  NS_ASSERT_MSG(m_ptr, "Attempted to dereference zero pointer");
  return *m_ptr;
}

template <typename T> Ptr<T>::operator Tester *() const {
  if (m_ptr == 0) {
    return 0;
  }
  static Tester test;
  return &test;
}

template <typename T> Ptr<T>::operator bool() const { return m_ptr != nullptr; }

} // namespace ns3

template <class T> struct std::hash<ns3::Ptr<T>> {
  std::size_t operator()(ns3::Ptr<T> p) const {
    return std::hash<const T *>()(ns3::PeekPointer(p));
  }
};

#endif
