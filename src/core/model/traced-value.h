#ifndef TRACED_VALUE_H
#define TRACED_VALUE_H

#include "boolean.h"
#include "double.h"
#include "enum.h"
#include "integer.h"
#include "traced-callback.h"
#include "uinteger.h"

#define TRACED_VALUE_DEBUG(x)

namespace ns3 {

namespace TracedValueCallback {

typedef void (*Bool)(bool oldValue, bool newValue);
typedef void (*Int8)(int8_t oldValue, int8_t newValue);
typedef void (*Uint8)(uint8_t oldValue, uint8_t newValue);
typedef void (*Int16)(int16_t oldValue, int16_t newValue);
typedef void (*Uint16)(uint16_t oldValue, uint16_t newValue);
typedef void (*Int32)(int32_t oldValue, int32_t newValue);
typedef void (*Uint32)(uint32_t oldValue, uint32_t newValue);
typedef void (*Int64)(int64_t oldValue, int64_t newValue);
typedef void (*Uint64)(uint64_t oldValue, uint64_t newValue);
typedef void (*Double)(double oldValue, double newValue);
typedef void (*Void)();

} // namespace TracedValueCallback

template <typename T> class TracedValue {
public:
  TracedValue() : m_v() {}

  TracedValue(const TracedValue &o) : m_v(o.m_v) {}

  TracedValue(const T &v) : m_v(v) {}

  operator T() const { return m_v; }

  TracedValue &operator=(const TracedValue &o) {
    TRACED_VALUE_DEBUG("x=");
    Set(o.m_v);
    return *this;
  }

  template <typename U>
  TracedValue(const TracedValue<U> &other) : m_v(other.Get()) {}

  template <typename U> TracedValue(const U &other) : m_v((T)other) {}

  void ConnectWithoutContext(const CallbackBase &cb) {
    m_cb.ConnectWithoutContext(cb);
  }

  void Connect(const CallbackBase &cb, std::string path) {
    m_cb.Connect(cb, path);
  }

  void DisconnectWithoutContext(const CallbackBase &cb) {
    m_cb.DisconnectWithoutContext(cb);
  }

  void Disconnect(const CallbackBase &cb, std::string path) {
    m_cb.Disconnect(cb, path);
  }

  void Set(const T &v) {
    if (m_v != v) {
      m_cb(m_v, v);
      m_v = v;
    }
  }

  T Get() const { return m_v; }

  TracedValue &operator++() {
    TRACED_VALUE_DEBUG("++x");
    T tmp = Get();
    ++tmp;
    Set(tmp);
    return *this;
  }

  TracedValue &operator--() {
    TRACED_VALUE_DEBUG("--x");
    T tmp = Get();
    --tmp;
    Set(tmp);
    return *this;
  }

  TracedValue operator++(int) {
    TRACED_VALUE_DEBUG("x++");
    TracedValue old(*this);
    T tmp = Get();
    tmp++;
    Set(tmp);
    return old;
  }

  TracedValue operator--(int) {
    TRACED_VALUE_DEBUG("x--");
    TracedValue old(*this);
    T tmp = Get();
    tmp--;
    Set(tmp);
    return old;
  }

private:
  T m_v;
  TracedCallback<T, T> m_cb;
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const TracedValue<T> &rhs) {
  return os << rhs.Get();
}

template <typename T, typename U>
bool operator==(const TracedValue<T> &lhs, const TracedValue<U> &rhs) {
  TRACED_VALUE_DEBUG("x==x");
  return lhs.Get() == rhs.Get();
}

template <typename T, typename U>
bool operator==(const TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x==");
  return lhs.Get() == rhs;
}

template <typename T, typename U>
bool operator==(const U &lhs, const TracedValue<T> &rhs) {
  TRACED_VALUE_DEBUG("==x");
  return lhs == rhs.Get();
}

template <typename T, typename U>
bool operator!=(const TracedValue<T> &lhs, const TracedValue<U> &rhs) {
  TRACED_VALUE_DEBUG("x!=x");
  return lhs.Get() != rhs.Get();
}

template <typename T, typename U>
bool operator!=(const TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x!=");
  return lhs.Get() != rhs;
}

template <typename T, typename U>
bool operator!=(const U &lhs, const TracedValue<T> &rhs) {
  TRACED_VALUE_DEBUG("!=x");
  return lhs != rhs.Get();
}

template <typename T, typename U>
bool operator<=(const TracedValue<T> &lhs, const TracedValue<U> &rhs) {
  TRACED_VALUE_DEBUG("x<=x");
  return lhs.Get() <= rhs.Get();
}

template <typename T, typename U>
bool operator<=(const TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x<=");
  return lhs.Get() <= rhs;
}

template <typename T, typename U>
bool operator<=(const U &lhs, const TracedValue<T> &rhs) {
  TRACED_VALUE_DEBUG("<=x");
  return lhs <= rhs.Get();
}

template <typename T, typename U>
bool operator>=(const TracedValue<T> &lhs, const TracedValue<U> &rhs) {
  TRACED_VALUE_DEBUG("x>=x");
  return lhs.Get() >= rhs.Get();
}

template <typename T, typename U>
bool operator>=(const TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x>=");
  return lhs.Get() >= rhs;
}

template <typename T, typename U>
bool operator>=(const U &lhs, const TracedValue<T> &rhs) {
  TRACED_VALUE_DEBUG(">=x");
  return lhs >= rhs.Get();
}

template <typename T, typename U>
bool operator<(const TracedValue<T> &lhs, const TracedValue<U> &rhs) {
  TRACED_VALUE_DEBUG("x<x");
  return lhs.Get() < rhs.Get();
}

template <typename T, typename U>
bool operator<(const TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x<");
  return lhs.Get() < rhs;
}

template <typename T, typename U>
bool operator<(const U &lhs, const TracedValue<T> &rhs) {
  TRACED_VALUE_DEBUG("<x");
  return lhs < rhs.Get();
}

template <typename T, typename U>
bool operator>(const TracedValue<T> &lhs, const TracedValue<U> &rhs) {
  TRACED_VALUE_DEBUG("x>x");
  return lhs.Get() > rhs.Get();
}

template <typename T, typename U>
bool operator>(const TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x>");
  return lhs.Get() > rhs;
}

template <typename T, typename U>
bool operator>(const U &lhs, const TracedValue<T> &rhs) {
  TRACED_VALUE_DEBUG(">x");
  return lhs > rhs.Get();
}

template <typename T, typename U>
auto operator+(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() + rhs.Get())> {
  TRACED_VALUE_DEBUG("x+x");
  return TracedValue<decltype(lhs.Get() + rhs.Get())>(lhs.Get() + rhs.Get());
}

template <typename T, typename U>
auto operator+(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() + rhs)> {
  TRACED_VALUE_DEBUG("x+");
  return TracedValue<decltype(lhs.Get() + rhs)>(lhs.Get() + rhs);
}

template <typename T, typename U>
auto operator+(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs + rhs.Get())> {
  TRACED_VALUE_DEBUG("+x");
  return TracedValue<decltype(lhs + rhs.Get())>(lhs + rhs.Get());
}

template <typename T, typename U>
auto operator-(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() - rhs.Get())> {
  TRACED_VALUE_DEBUG("x-x");
  return TracedValue<decltype(lhs.Get() - rhs.Get())>(lhs.Get() - rhs.Get());
}

template <typename T, typename U>
auto operator-(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() - rhs)> {
  TRACED_VALUE_DEBUG("x-");
  return TracedValue<decltype(lhs.Get() - rhs)>(lhs.Get() - rhs);
}

template <typename T, typename U>
auto operator-(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs - rhs.Get())> {
  TRACED_VALUE_DEBUG("-x");
  return TracedValue<decltype(lhs - rhs.Get())>(lhs - rhs.Get());
}

template <typename T, typename U>
auto operator*(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() * rhs.Get())> {
  TRACED_VALUE_DEBUG("x*x");
  return TracedValue<decltype(lhs.Get() * rhs.Get())>(lhs.Get() * rhs.Get());
}

template <typename T, typename U>
auto operator*(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() * rhs)> {
  TRACED_VALUE_DEBUG("x*");
  return TracedValue<decltype(lhs.Get() * rhs)>(lhs.Get() * rhs);
}

template <typename T, typename U>
auto operator*(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs + rhs.Get())> {
  TRACED_VALUE_DEBUG("*x");
  return TracedValue<decltype(lhs + rhs.Get())>(lhs * rhs.Get());
}

template <typename T, typename U>
auto operator/(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() / rhs.Get())> {
  TRACED_VALUE_DEBUG("x/x");
  return TracedValue<decltype(lhs.Get() / rhs.Get())>(lhs.Get() / rhs.Get());
}

template <typename T, typename U>
auto operator/(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() / rhs)> {
  TRACED_VALUE_DEBUG("x/");
  return TracedValue<decltype(lhs.Get() / rhs)>(lhs.Get() / rhs);
}

template <typename T, typename U>
auto operator/(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs / rhs.Get())> {
  TRACED_VALUE_DEBUG("/x");
  return TracedValue<decltype(lhs / rhs.Get())>(lhs / rhs.Get());
}

template <typename T, typename U>
auto operator%(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() % rhs.Get())> {
  TRACED_VALUE_DEBUG("x%x");
  return TracedValue<decltype(lhs.Get() % rhs.Get())>(lhs.Get() % rhs.Get());
}

template <typename T, typename U>
auto operator%(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() % rhs)> {
  TRACED_VALUE_DEBUG("x%");
  return TracedValue<decltype(lhs.Get() % rhs)>(lhs.Get() % rhs);
}

template <typename T, typename U>
auto operator%(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs % rhs.Get())> {
  TRACED_VALUE_DEBUG("%x");
  return TracedValue<decltype(lhs % rhs.Get())>(lhs % rhs.Get());
}

template <typename T, typename U>
auto operator^(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() ^ rhs.Get())> {
  TRACED_VALUE_DEBUG("x^x");
  return TracedValue<decltype(lhs.Get() ^ rhs.Get())>(lhs.Get() ^ rhs.Get());
}

template <typename T, typename U>
auto operator^(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() ^ rhs)> {
  TRACED_VALUE_DEBUG("x^");
  return TracedValue<decltype(lhs.Get() ^ rhs)>(lhs.Get() ^ rhs);
}

template <typename T, typename U>
auto operator^(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs ^ rhs.Get())> {
  TRACED_VALUE_DEBUG("^x");
  return TracedValue<decltype(lhs ^ rhs.Get())>(lhs ^ rhs.Get());
}

template <typename T, typename U>
auto operator|(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() | rhs.Get())> {
  TRACED_VALUE_DEBUG("x|x");
  return TracedValue<decltype(lhs.Get() | rhs.Get())>(lhs.Get() | rhs.Get());
}

template <typename T, typename U>
auto operator|(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() | rhs)> {
  TRACED_VALUE_DEBUG("x|");
  return TracedValue<decltype(lhs.Get() | rhs)>(lhs.Get() | rhs);
}

template <typename T, typename U>
auto operator|(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs | rhs.Get())> {
  TRACED_VALUE_DEBUG("|x");
  return TracedValue<decltype(lhs | rhs.Get())>(lhs | rhs.Get());
}

template <typename T, typename U>
auto operator&(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() & rhs.Get())> {
  TRACED_VALUE_DEBUG("x&x");
  return TracedValue<decltype(lhs.Get() & rhs.Get())>(lhs.Get() & rhs.Get());
}

template <typename T, typename U>
auto operator&(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() & rhs)> {
  TRACED_VALUE_DEBUG("x&");
  return TracedValue<decltype(lhs.Get() & rhs)>(lhs.Get() & rhs);
}

template <typename T, typename U>
auto operator&(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs & rhs.Get())> {
  TRACED_VALUE_DEBUG("&x");
  return TracedValue<decltype(lhs & rhs.Get())>(lhs & rhs.Get());
}

template <typename T, typename U>
auto operator<<(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() << rhs.Get())> {
  TRACED_VALUE_DEBUG("x<<x");
  return TracedValue<decltype(lhs.Get() << rhs.Get())>(lhs.Get() << rhs.Get());
}

template <typename T, typename U>
auto operator<<(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() << rhs)> {
  TRACED_VALUE_DEBUG("x<<");
  return TracedValue<decltype(lhs.Get() << rhs)>(lhs.Get() << rhs);
}

template <typename T, typename U>
auto operator<<(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs << rhs.Get())> {
  TRACED_VALUE_DEBUG("<<x");
  return TracedValue<decltype(lhs << rhs.Get())>(lhs << rhs.Get());
}

template <typename T, typename U>
auto operator>>(const TracedValue<T> &lhs, const TracedValue<U> &rhs)
    -> TracedValue<decltype(lhs.Get() >> rhs.Get())> {
  TRACED_VALUE_DEBUG("x>>x");
  return TracedValue<decltype(lhs.Get() >> rhs.Get())>(lhs.Get() >> rhs.Get());
}

template <typename T, typename U>
auto operator>>(const TracedValue<T> &lhs, const U &rhs)
    -> TracedValue<decltype(lhs.Get() >> rhs)> {
  TRACED_VALUE_DEBUG("x>>");
  return TracedValue<decltype(lhs.Get() >> rhs)>(lhs.Get() >> rhs);
}

template <typename T, typename U>
auto operator>>(const U &lhs, const TracedValue<T> &rhs)
    -> TracedValue<decltype(lhs >> rhs.Get())> {
  TRACED_VALUE_DEBUG(">>x");
  return TracedValue<decltype(lhs >> rhs.Get())>(lhs >> rhs.Get());
}

template <typename T, typename U>
TracedValue<T> &operator+=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x+=");
  T tmp = lhs.Get();
  tmp += rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator-=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x-=");
  T tmp = lhs.Get();
  tmp -= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator*=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x*=");
  T tmp = lhs.Get();
  tmp *= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator/=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x/=");
  T tmp = lhs.Get();
  tmp /= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator%=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x%=");
  T tmp = lhs.Get();
  tmp %= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator<<=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x<<=");
  T tmp = lhs.Get();
  tmp <<= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator>>=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x>>=");
  T tmp = lhs.Get();
  tmp >>= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator&=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x&=");
  T tmp = lhs.Get();
  tmp &= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator|=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x|=");
  T tmp = lhs.Get();
  tmp |= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T, typename U>
TracedValue<T> &operator^=(TracedValue<T> &lhs, const U &rhs) {
  TRACED_VALUE_DEBUG("x^=");
  T tmp = lhs.Get();
  tmp ^= rhs;
  lhs.Set(tmp);
  return lhs;
}

template <typename T> TracedValue<T> operator+(const TracedValue<T> &lhs) {
  TRACED_VALUE_DEBUG("(+x)");
  return TracedValue<T>(+lhs.Get());
}

template <typename T> TracedValue<T> operator-(const TracedValue<T> &lhs) {
  TRACED_VALUE_DEBUG("(-x)");
  return TracedValue<T>(-lhs.Get());
}

template <typename T> TracedValue<T> operator~(const TracedValue<T> &lhs) {
  TRACED_VALUE_DEBUG("(~x)");
  return TracedValue<T>(~lhs.Get());
}

template <typename T> TracedValue<T> operator!(const TracedValue<T> &lhs) {
  TRACED_VALUE_DEBUG("(!x)");
  return TracedValue<T>(!lhs.Get());
}

} // namespace ns3

#endif
