#include "int64x64-cairo.h"

#include "abort.h"
#include "assert.h"
#include "log.h"
#include "test.h"

#include <cmath>
#include <iostream>

extern "C" {
#include "cairo-wideint.c"
}

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("int64x64-cairo");

static inline bool output_sign(const cairo_int128_t sa, const cairo_int128_t sb,
                               cairo_uint128_t &ua, cairo_uint128_t &ub) {
  bool negA = _cairo_int128_negative(sa);
  bool negB = _cairo_int128_negative(sb);
  ua = _cairo_int128_to_uint128(sa);
  ub = _cairo_int128_to_uint128(sb);
  ua = negA ? _cairo_uint128_negate(ua) : ua;
  ub = negB ? _cairo_uint128_negate(ub) : ub;
  return (negA && !negB) || (!negA && negB);
}

void int64x64_t::Mul(const int64x64_t &o) {
  cairo_uint128_t a, b;
  bool sign = output_sign(_v, o._v, a, b);
  cairo_uint128_t result = Umul(a, b);
  _v = sign ? _cairo_uint128_negate(result) : result;
}

cairo_uint128_t int64x64_t::Umul(const cairo_uint128_t a,
                                 const cairo_uint128_t b) {
  cairo_uint128_t result;
  cairo_uint128_t hiPart, loPart, midPart;
  cairo_uint128_t res1, res2;

  loPart = _cairo_uint64x64_128_mul(a.lo, b.lo);
  midPart = _cairo_uint128_add(_cairo_uint64x64_128_mul(a.lo, b.hi),
                               _cairo_uint64x64_128_mul(a.hi, b.lo));
  hiPart = _cairo_uint64x64_128_mul(a.hi, b.hi);
  NS_ABORT_MSG_IF(
      hiPart.hi != 0,
      "High precision 128 bits multiplication error: multiplication overflow.");

  res1 = _cairo_uint64_to_uint128(loPart.hi);
  res2 = _cairo_uint64_to_uint128(midPart.lo);
  result = _cairo_uint128_add(res1, res2);

  res1 = _cairo_uint64_to_uint128(midPart.hi);
  res2 = _cairo_uint64_to_uint128(hiPart.lo);
  res1 = _cairo_uint128_add(res1, res2);
  res1 = _cairo_uint128_lsl(res1, 64);

  result = _cairo_uint128_add(result, res1);

  return result;
}

void int64x64_t::Div(const int64x64_t &o) {
  cairo_uint128_t a, b;
  bool sign = output_sign(_v, o._v, a, b);
  cairo_uint128_t result = Udiv(a, b);
  _v = sign ? _cairo_uint128_negate(result) : result;
}

cairo_uint128_t int64x64_t::Udiv(const cairo_uint128_t a,
                                 const cairo_uint128_t b) {
  cairo_uint128_t den = b;
  cairo_uquorem128_t qr = _cairo_uint128_divrem(a, b);
  cairo_uint128_t result = qr.quo;
  cairo_uint128_t rem = qr.rem;

  const uint64_t DIGITS = 64;
  const cairo_uint128_t ZERO = _cairo_uint32_to_uint128((uint32_t)0);

  NS_ASSERT_MSG(_cairo_uint128_lt(rem, den), "Remainder not less than divisor");

  uint64_t digis = 0;
  uint64_t shift = 0;

  while ((shift < DIGITS) && !(den.lo & 0x1)) {
    ++shift;
    den = _cairo_uint128_rsl(den, 1);
  }

  while ((digis < DIGITS) && !(_cairo_uint128_eq(rem, ZERO))) {
    while ((digis + shift < DIGITS) && !(rem.hi & HPCAIRO_MASK_HI_BIT)) {
      ++shift;
      rem = _cairo_int128_lsl(rem, 1);
    }

    while ((digis + shift < DIGITS) &&
           (!(den.lo & 0x1) || _cairo_uint128_lt(rem, den))) {
      ++shift;
      den = _cairo_uint128_rsl(den, 1);
    }

    qr = _cairo_uint128_divrem(rem, den);

    result = _cairo_uint128_lsl(result, static_cast<int>(shift));
    result = _cairo_uint128_add(result, qr.quo);
    rem = qr.rem;
    digis += shift;
    shift = 0;
  }
  if (digis < DIGITS) {
    shift = DIGITS - digis;
    result = _cairo_uint128_lsl(result, static_cast<int>(shift));
  }

  return result;
}

void int64x64_t::MulByInvert(const int64x64_t &o) {
  bool sign = _cairo_int128_negative(_v);
  cairo_uint128_t a = sign ? _cairo_int128_negate(_v) : _v;
  cairo_uint128_t result = UmulByInvert(a, o._v);

  _v = sign ? _cairo_int128_negate(result) : result;
}

cairo_uint128_t int64x64_t::UmulByInvert(const cairo_uint128_t a,
                                         const cairo_uint128_t b) {
  cairo_uint128_t result;
  cairo_uint128_t hi, mid;
  hi = _cairo_uint64x64_128_mul(a.hi, b.hi);
  mid = _cairo_uint128_add(_cairo_uint64x64_128_mul(a.hi, b.lo),
                           _cairo_uint64x64_128_mul(a.lo, b.hi));
  mid.lo = mid.hi;
  mid.hi = 0;
  result = _cairo_uint128_add(hi, mid);
  return result;
}

int64x64_t int64x64_t::Invert(const uint64_t v) {
  NS_ASSERT(v > 1);
  cairo_uint128_t a, factor;
  a.hi = 1;
  a.lo = 0;
  factor.hi = 0;
  factor.lo = v;
  int64x64_t result;
  result._v = Udiv(a, factor);
  int64x64_t tmp = int64x64_t(v, 0);
  tmp.MulByInvert(result);
  if (tmp.GetHigh() != 1) {
    cairo_uint128_t one = {1, 0};
    result._v = _cairo_uint128_add(result._v, one);
  }
  return result;
}

} // namespace ns3
