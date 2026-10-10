
#include "ns3/core-config.h"
#if !defined(INT64X64_CAIRO_H) && defined(INT64X64_USE_CAIRO) &&               \
    !defined(PYTHON_SCAN)
#define INT64X64_CAIRO_H

#include "cairo-wideint-private.h"

#include <cmath>

namespace ns3 {

class int64x64_t {
  static const uint64_t HPCAIRO_MASK_HI_BIT = (((uint64_t)1) << 63);
  static const uint64_t HP_MASK_LO = 0xffffffffffffffffULL;
#define HP_MAX_64 (std::pow(2.0L, 64))

public:
  enum impl_type {
    int128_impl,
    cairo_impl,
    ld_impl,
  };

  static const enum impl_type implementation = cairo_impl;

  inline int64x64_t() {
    _v.hi = 0;
    _v.lo = 0;
  }

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
    cairo_int64_t hi = (cairo_int64_t)fhi;
    const cairo_uint64_t lo = (cairo_uint64_t)flo;
    if (flo >= HP_MAX_64) {
      ++hi;
    }
    _v.hi = hi;
    _v.lo = lo;
    _v = negative ? _cairo_int128_negate(_v) : _v;
  }

  inline int64x64_t(const int v) {
    _v.hi = v;
    _v.lo = 0;
  }

  inline int64x64_t(const long int v) {
    _v.hi = v;
    _v.lo = 0;
  }

  inline int64x64_t(const long long int v) {
    _v.hi = v;
    _v.lo = 0;
  }

  inline int64x64_t(const unsigned int v) {
    _v.hi = v;
    _v.lo = 0;
  }

  inline int64x64_t(const unsigned long int v) {
    _v.hi = v;
    _v.lo = 0;
  }

  inline int64x64_t(const unsigned long long int v) {
    _v.hi = v;
    _v.lo = 0;
  }

  explicit inline int64x64_t(const int64_t hi, const uint64_t lo) {
    _v.hi = hi;
    _v.lo = lo;
  }

  inline int64x64_t(const int64x64_t &o) : _v(o._v) {}

  inline int64x64_t &operator=(const int64x64_t &o) {
    _v = o._v;
    return *this;
  }

  inline explicit operator bool() const { return (_v.hi != 0 || _v.lo != 0); }

  inline double GetDouble() const {
    const bool negative = _cairo_int128_negative(_v);
    const cairo_int128_t value = negative ? _cairo_int128_negate(_v) : _v;
    const long double fhi = static_cast<long double>(value.hi);
    const long double flo = value.lo / HP_MAX_64;
    long double retval = fhi;
    retval += flo;
    retval = negative ? -retval : retval;
    return static_cast<double>(retval);
  }

  inline int64_t GetHigh() const { return (int64_t)_v.hi; }

  inline uint64_t GetLow() const { return _v.lo; }

  int64_t GetInt() const {
    const bool negative = _cairo_int128_negative(_v);
    const cairo_int128_t value = negative ? _cairo_int128_negate(_v) : _v;
    int64_t retval = value.hi;
    retval = negative ? -retval : retval;
    return retval;
  }

  int64_t Round() const {
    const bool negative = _cairo_int128_negative(_v);
    cairo_uint128_t value = negative ? _cairo_int128_negate(_v) : _v;
    cairo_uint128_t half{1ULL << 63, 0};
    value = _cairo_uint128_add(value, half);
    int64_t retval = value.hi;
    retval = negative ? -retval : retval;
    return retval;
  }

  void MulByInvert(const int64x64_t &o);

  static int64x64_t Invert(const uint64_t v);

private:
  friend inline bool operator==(const int64x64_t &lhs, const int64x64_t &rhs) {
    return _cairo_int128_eq(lhs._v, rhs._v);
  }

  friend inline bool operator<(const int64x64_t &lhs, const int64x64_t &rhs) {
    return _cairo_int128_lt(lhs._v, rhs._v);
  }

  friend inline bool operator>(const int64x64_t &lhs, const int64x64_t &rhs) {
    return _cairo_int128_gt(lhs._v, rhs._v);
  }

  friend inline int64x64_t &operator+=(int64x64_t &lhs, const int64x64_t &rhs) {
    lhs._v = _cairo_int128_add(lhs._v, rhs._v);
    return lhs;
  }

  friend inline int64x64_t &operator-=(int64x64_t &lhs, const int64x64_t &rhs) {
    lhs._v = _cairo_int128_sub(lhs._v, rhs._v);
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
    int64x64_t tmp = lhs;
    tmp._v = _cairo_int128_negate(tmp._v);
    return tmp;
  }

  friend inline int64x64_t operator!(const int64x64_t &lhs) {
    return (lhs == int64x64_t()) ? int64x64_t(1, 0) : int64x64_t();
  }

  void Mul(const int64x64_t &o);
  void Div(const int64x64_t &o);
  static cairo_uint128_t Umul(const cairo_uint128_t a, const cairo_uint128_t b);
  static cairo_uint128_t Udiv(const cairo_uint128_t a, const cairo_uint128_t b);
  static cairo_uint128_t UmulByInvert(const cairo_uint128_t a,
                                      const cairo_uint128_t b);

  cairo_int128_t _v;
};

} // namespace ns3

#endif
