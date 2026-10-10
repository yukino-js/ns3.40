
#include "ns3/core-config.h"
#if !defined(INT64X64_DOUBLE_H) &&                                             \
    (defined(INT64X64_USE_DOUBLE) || defined(PYTHON_SCAN))
#define INT64X64_DOUBLE_H

#include <cmath>
#include <stdint.h>
#include <utility>

namespace ns3 {

class int64x64_t {
  static const uint64_t HP_MASK_LO = 0xffffffffffffffffULL;
#define HP_MAX_64 (std::pow(2.0L, 64))

public:
  enum impl_type {
    int128_impl,
    cairo_impl,
    ld_impl,
  };

  static const enum impl_type implementation = ld_impl;

  inline int64x64_t() : _v(0) {}

  inline int64x64_t(double value) : _v(value) {}

  inline int64x64_t(long double value) : _v(value) {}

  inline int64x64_t(int v) : _v(v) {}

  inline int64x64_t(long int v) : _v(v) {}

  inline int64x64_t(long long int v) : _v(static_cast<long double>(v)) {}

  inline int64x64_t(unsigned int v) : _v(v) {}

  inline int64x64_t(unsigned long int v) : _v(v) {}

  inline int64x64_t(unsigned long long int v)
      : _v(static_cast<long double>(v)) {}

  explicit inline int64x64_t(int64_t hi, uint64_t lo) {
    const bool negative = hi < 0;
    const long double hild = static_cast<long double>(hi);
    const long double fhi = negative ? -hild : hild;
    const long double flo = lo / HP_MAX_64;
    _v = negative ? -fhi : fhi;
    _v += flo;
  }

  inline int64x64_t(const int64x64_t &o) : _v(o._v) {}

  inline int64x64_t &operator=(const int64x64_t &o) {
    _v = o._v;
    return *this;
  }

  inline explicit operator bool() const { return (_v != 0); }

  inline double GetDouble() const { return (double)_v; }

private:
  std::pair<int64_t, uint64_t> GetHighLow() const {
    const bool negative = _v < 0;
    const long double v = negative ? -_v : _v;

    long double fhi;
    long double flo = std::modf(v, &fhi);
    const long double round = 0.5;
    flo = flo * HP_MAX_64 + round;
    int64_t hi = static_cast<int64_t>(fhi);
    uint64_t lo = static_cast<uint64_t>(flo);
    if (flo >= HP_MAX_64) {
      ++hi;
    }
    if (negative) {
      lo = ~lo;
      hi = ~hi;
      if (++lo == 0) {
        ++hi;
      }
    }
    return std::make_pair(hi, lo);
  }

public:
  inline int64_t GetHigh() const { return GetHighLow().first; }

  inline uint64_t GetLow() const { return GetHighLow().second; }

  int64_t GetInt() const {
    int64_t retval = static_cast<int64_t>(_v);
    return retval;
  }

  int64_t Round() const {
    int64_t retval = std::round(_v);
    return retval;
  }

  inline void MulByInvert(const int64x64_t &o) { _v *= o._v; }

  static inline int64x64_t Invert(uint64_t v) {
    int64x64_t tmp((long double)1 / v);
    return tmp;
  }

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
    lhs._v *= rhs._v;
    return lhs;
  }

  friend inline int64x64_t &operator/=(int64x64_t &lhs, const int64x64_t &rhs) {
    lhs._v /= rhs._v;
    return lhs;
  }

  friend inline int64x64_t operator+(const int64x64_t &lhs) { return lhs; }

  friend inline int64x64_t operator-(const int64x64_t &lhs) {
    return int64x64_t(-lhs._v);
  }

  friend inline int64x64_t operator!(const int64x64_t &lhs) {
    return int64x64_t(!lhs._v);
  }

  long double _v;
};

} // namespace ns3

#endif
