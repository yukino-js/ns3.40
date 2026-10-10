
#ifndef INT64X64_H
#define INT64X64_H

#include "ns3/core-config.h"

#if defined(INT64X64_USE_128) && !defined(PYTHON_SCAN)
#include "int64x64-128.h"
#elif defined(INT64X64_USE_CAIRO) && !defined(PYTHON_SCAN)
#include "int64x64-cairo.h"
#elif defined(INT64X64_USE_DOUBLE) || defined(PYTHON_SCAN)
#include "int64x64-double.h"
#endif

#include <iostream>

namespace ns3 {

inline int64x64_t operator+(const int64x64_t &lhs, const int64x64_t &rhs) {
  int64x64_t tmp = lhs;
  tmp += rhs;
  return tmp;
}

inline int64x64_t operator-(const int64x64_t &lhs, const int64x64_t &rhs) {
  int64x64_t tmp = lhs;
  tmp -= rhs;
  return tmp;
}

inline int64x64_t operator*(const int64x64_t &lhs, const int64x64_t &rhs) {
  int64x64_t tmp = lhs;
  tmp *= rhs;
  return tmp;
}

inline int64x64_t operator/(const int64x64_t &lhs, const int64x64_t &rhs) {
  int64x64_t tmp = lhs;
  tmp /= rhs;
  return tmp;
}

inline bool operator!=(const int64x64_t &lhs, const int64x64_t &rhs) {
  return !(lhs == rhs);
}

inline bool operator<=(const int64x64_t &lhs, const int64x64_t &rhs) {
  return !(lhs > rhs);
}

inline bool operator>=(const int64x64_t &lhs, const int64x64_t &rhs) {
  return !(lhs < rhs);
}

std::ostream &operator<<(std::ostream &os, const int64x64_t &value);
std::istream &operator>>(std::istream &is, int64x64_t &value);

inline int64x64_t Abs(const int64x64_t &value) {
  return (value < 0) ? -value : value;
}

inline int64x64_t Min(const int64x64_t &a, const int64x64_t &b) {
  return (a < b) ? a : b;
}

inline int64x64_t Max(const int64x64_t &a, const int64x64_t &b) {
  return (a > b) ? a : b;
}

} // namespace ns3

#endif
