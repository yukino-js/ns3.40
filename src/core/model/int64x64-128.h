
#include "ns3/core-config.h"

#if !defined(INT64X64_128_H) && defined(INT64X64_USE_128) &&                   \
    !defined(PYTHON_SCAN)
#define INT64X64_128_H

#include <cmath>
#include <stdint.h>

#if defined(HAVE___UINT128_T) && !defined(HAVE_UINT128_T)
typedef __uint128_t uint128_t;
typedef __int128_t int128_t;
#endif

namespace ns3 {

class int64x64_t {
  static const uint128_t HP128_MASK_HI_BIT = (((int128_t)1) << 127);
  static const uint64_t HP_MASK_LO = 0xffffffffffffffffULL;
  static const uint64_t HP_MASK_HI = ~HP_MASK_LO;
#define HP_MAX_64 (std::pow(2.0L, 64))

public:
  enum impl_type {
    int128_impl,
    cairo_impl,
    ld_impl,
  };

  static const enum impl_type implementation = int128_impl;

  inline int64x64_t() : _v(0) {}

  inline int64x64_t(const double value) {
    const int64x64_t tmp((long double)value);
    _v = tmp._v;
  }

  inline int64x64_t(const long double value) {
    const bool negative = value < 0;
    const long double v = negative ? -value : value;

    long double fhi;
    long double flo = std::modf(v, &fhi);
    const long double round = 0.5;
    flo = flo * HP_MAX_64 + round;
    int128_t hi = fhi;
    const uint64_t lo = flo;
    if (flo >= HP_MAX_64) {
      ++hi;
    }
    _v = hi << 64;
    _v |= lo;
    _v = negative ? -_v : _v;
  }

  inline int64x64_t(const int v) : _v(v) { _v <<= 64; }

  inline int64x64_t(const long int v) : _v(v) { _v <<= 64; }

  inline int64x64_t(const long long int v) : _v(v) { _v <<= 64; }

  inline int64x64_t(const unsigned int v) : _v(v) { _v <<= 64; }

  inline int64x64_t(const unsigned long int v) : _v(v) { _v <<= 64; }

  inline int64x64_t(const unsigned long long int v) : _v(v) { _v <<= 64; }

  inline int64x64_t(const int128_t v) : _v(v) {}

  explicit inline int64x64_t(const int64_t hi, const uint64_t lo) {
    _v = (int128_t)hi << 64;
    _v |= lo;
  }

  inline int64x64_t(const int64x64_t &o) : _v(o._v) {}

  inline int64x64_t &operator=(const int64x64_t &o) {
    _v = o._v;
    return *this;
  }

  inline explicit operator bool() const { return (_v != 0); }

  inline double GetDouble() const {
    const bool negative = _v < 0;
    const uint128_t value = negative ? -_v : _v;
    const long double fhi = value >> 64;
    const long double flo = (value & HP_MASK_LO) / HP_MAX_64;
    long double retval = fhi;
    retval += flo;
    retval = negative ? -retval : retval;
    return retval;
  }

  inline int64_t GetHigh() const {
    const int128_t retval = _v >> 64;
    return retval;
  }

  inline uint64_t GetLow() const {
    const uint128_t retval = _v & HP_MASK_LO;
    return retval;
  }

  int64_t GetInt() const {
    const bool negative = _v < 0;
    const uint128_t value = negative ? -_v : _v;
    int64_t retval = value >> 64;
    retval = negative ? -retval : retval;
    return retval;
  }

  int64_t Round() const {
    const bool negative = _v < 0;
    int64x64_t value = (negative ? -(*this) : *this);
    const int64x64_t half(0, 1LL << 63);
    value += half;
    int64_t retval = value.GetHigh();
    retval = negative ? -retval : retval;
    return retval;
  }

  void MulByInvert(const int64x64_t &o);

  static int64x64_t Invert(const uint64_t v);

private:
  friend inline bool operator==(const int64x64_t &lhs, const int64x64_t &rhs) {
    return lhs._v == rhs._v;
  }

  friend inline bool operator<(const int64x64_t &lhs, const int64x64_t &rhs) {
    return lhs._v < rhs._v;
  }

  friend inline bool operator>(const int64x64_t &lhs, const int64x64_t &rhs) {
    return lhs._v > rhs._v;
  }

  friend inline int64x64_t &operator+=(int64x64_t &lhs, const int64x64_t &rhs) {
    lhs._v += rhs._v;
    return lhs;
  }

  friend inline int64x64_t &operator-=(int64x64_t &lhs, const int64x64_t &rhs) {
    lhs._v -= rhs._v;
    return lhs;
  }

  friend inline int64x64_t &operator*=(int64x64_t &lhs, const int64x64_t &rhs) {
    lhs.Mul(rhs);
    return lhs;
  }

  friend inline int64x64_t &operator/=(int64x64_t &lhs, const int64x64_t &rhs) {
    lhs.Div(rhs);
    return lhs;
  }

  friend inline int64x64_t operator+(const int64x64_t &lhs) { return lhs; }

  friend inline int64x64_t operator-(const int64x64_t &lhs) {
    return int64x64_t(-lhs._v);
  }

  friend inline int64x64_t operator!(const int64x64_t &lhs) {
    return int64x64_t(!lhs._v);
  }

  void Mul(const int64x64_t &o);
  void Div(const int64x64_t &o);
  static uint128_t Umul(const uint128_t a, const uint128_t b);
  static uint128_t Udiv(const uint128_t a, const uint128_t b);
  static uint128_t UmulByInvert(const uint128_t a, const uint128_t b);

  int128_t _v;
};

} // namespace ns3

#endif
