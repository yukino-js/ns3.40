
#include "int64x64-128.h"

#include "abort.h"
#include "assert.h"
#include "log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("int64x64-128");

static inline bool output_sign(const int128_t sa, const int128_t sb,
                               uint128_t &ua, uint128_t &ub) {
  bool negA = sa < 0;
  bool negB = sb < 0;
  ua = negA ? -sa : sa;
  ub = negB ? -sb : sb;
  return negA != negB;
}

void int64x64_t::Mul(const int64x64_t &o) {
  uint128_t a;
  uint128_t b;
  bool negative = output_sign(_v, o._v, a, b);
  uint128_t result = Umul(a, b);
  _v = negative ? -result : result;
}

uint128_t int64x64_t::Umul(const uint128_t a, const uint128_t b) {
  uint128_t aL = a & HP_MASK_LO;
  uint128_t bL = b & HP_MASK_LO;
  uint128_t aH = (a >> 64) & HP_MASK_LO;
  uint128_t bH = (b >> 64) & HP_MASK_LO;

  uint128_t result;
  uint128_t hiPart;
  uint128_t loPart;
  uint128_t midPart;
  uint128_t res1;
  uint128_t res2;

  loPart = aL * bL;
  midPart = aL * bH + aH * bL;
  hiPart = aH * bH;
  NS_ABORT_MSG_IF(
      (hiPart & HP_MASK_HI) != 0,
      "High precision 128 bits multiplication error: multiplication overflow.");

  res1 = loPart >> 64;
  res2 = midPart & HP_MASK_LO;
  result = res1 + res2;

  res1 = midPart >> 64;
  res2 = hiPart & HP_MASK_LO;
  res1 += res2;
  res1 <<= 64;

  result += res1;

  return result;
}

void int64x64_t::Div(const int64x64_t &o) {
  uint128_t a;
  uint128_t b;
  bool negative = output_sign(_v, o._v, a, b);
  int128_t result = Udiv(a, b);
  _v = negative ? -result : result;
}

uint128_t int64x64_t::Udiv(const uint128_t a, const uint128_t b) {
  uint128_t rem = a;
  uint128_t den = b;
  uint128_t quo = rem / den;
  rem = rem % den;
  uint128_t result = quo;

  const uint64_t DIGITS = 64;
  const uint128_t ZERO = 0;

  NS_ASSERT_MSG(rem < den, "Remainder not less than divisor");

  uint64_t digis = 0;
  uint64_t shift = 0;

  while ((shift < DIGITS) && !(den & 0x1)) {
    ++shift;
    den >>= 1;
  }

  while ((digis < DIGITS) && (rem != ZERO)) {
    while ((digis + shift < DIGITS) && !(rem & HP128_MASK_HI_BIT)) {
      ++shift;
      rem <<= 1;
    }

    while ((digis + shift < DIGITS) && (!(den & 0x1) || (rem < den))) {
      ++shift;
      den >>= 1;
    }

    quo = rem / den;
    rem = rem % den;

    result <<= shift;
    result += quo;

    digis += shift;
    shift = 0;
  }
  if (digis < DIGITS) {
    shift = DIGITS - digis;
    result <<= shift;
  }

  return result;
}

void int64x64_t::MulByInvert(const int64x64_t &o) {
  bool negResult = _v < 0;
  uint128_t a = negResult ? -_v : _v;
  uint128_t result = UmulByInvert(a, o._v);

  _v = negResult ? -result : result;
}

uint128_t int64x64_t::UmulByInvert(const uint128_t a, const uint128_t b) {
  uint128_t result;
  uint128_t ah;
  uint128_t bh;
  uint128_t al;
  uint128_t bl;
  uint128_t hi;
  uint128_t mid;
  ah = a >> 64;
  bh = b >> 64;
  al = a & HP_MASK_LO;
  bl = b & HP_MASK_LO;
  hi = ah * bh;
  mid = ah * bl + al * bh;
  mid >>= 64;
  result = hi + mid;
  return result;
}

int64x64_t int64x64_t::Invert(const uint64_t v) {
  NS_ASSERT(v > 1);
  uint128_t a;
  a = 1;
  a <<= 64;
  int64x64_t result;
  result._v = Udiv(a, v);
  int64x64_t tmp = int64x64_t(v, false);
  tmp.MulByInvert(result);
  if (tmp.GetHigh() != 1) {
    result._v += 1;
  }
  return result;
}

} // namespace ns3
