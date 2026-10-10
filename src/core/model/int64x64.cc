
#include "int64x64.h"

#include "assert.h"
#include "log.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE_MASK("int64x64", LOG_PREFIX_TIME);

#define HEXHILOW(hi, lo)                                                       \
  std::hex << std::setfill('0') << std::right << " (0x" << std::setw(16) << hi \
           << " " << std::setw(16) << lo << std::dec << std::setfill(' ')      \
           << std::left << ")"

std::ostream &operator<<(std::ostream &os, const int64x64_t &value) {
  const bool negative = (value < 0);
  const int64x64_t absVal = (negative ? -value : value);

  int64_t hi = absVal.GetHigh();

  auto precision = static_cast<std::size_t>(os.precision());
  std::ios_base::fmtflags ff = os.flags();
  const bool floatfield = os.flags() & std::ios_base::floatfield;
  os << std::setw(1) << std::noshowpos;

  os << std::right << (negative ? "-" : "+");

  std::ostringstream oss;
  oss << hi << ".";

  int64x64_t low(0, absVal.GetLow());
  std::size_t places = 0;
  bool more = true;

  NS_LOG_LOGIC(std::endl
               << (floatfield ? " f" : "  ") << "[" << precision << "] " << hi
               << ". " << HEXHILOW(hi, low.GetLow()));

  int64_t digit;
  do {
    low *= 10;
    digit = low.GetHigh();
    NS_ASSERT_MSG((0 <= digit) && (digit <= 9),
                  "digit " << digit << " out of range [0,9] "
                           << " streaming out "
                           << HEXHILOW(value.GetHigh(), value.GetLow()));
    low -= digit;

    oss << std::setw(1) << digit;

    ++places;
    if (floatfield) {
      more = places < precision;
    } else {
      more = low.GetLow() && (places < 20);
    }

    NS_LOG_LOGIC((more ? "+" : " ")
                 << (floatfield ? "f" : " ") << "[" << places << "] " << digit
                 << HEXHILOW(low.GetHigh(), low.GetLow()) << std::dec
                 << std::setfill(' ') << std::left);

  } while (more);

  std::string digits = oss.str();
  low *= 10;
  int64_t nextDigit = low.GetHigh();
  if ((nextDigit > 5) || ((nextDigit == 5) && (digit % 2 == 1))) {
    bool carry = true;
    for (std::string::reverse_iterator rit = digits.rbegin();
         rit != digits.rend(); ++rit) {
      if (*rit == '.') {
        continue;
      }

      ++(*rit);
      if (*rit <= '9') {
        carry = false;
        break;
      } else {
        *rit = '0';
      }
    }
    if (carry) {
      digits.insert(digits.begin(), '1');
    }
  }
  os << digits;

  os.flags(ff);
  return os;
}

static uint64_t ReadHiDigits(std::string str) {
  const char *buf = str.c_str();
  uint64_t retval = 0;
  while (*buf != 0) {
    retval *= 10;
    retval += *buf - '0';
    buf++;
  }
  return retval;
}

static uint64_t ReadLoDigits(std::string str) {
  int64x64_t low;
  const int64x64_t round(0, 5);

  for (std::string::const_reverse_iterator rit = str.rbegin();
       rit != str.rend(); ++rit) {
    int digit = *rit - '0';
    NS_ASSERT_MSG((0 <= digit) && (digit <= 9),
                  "digit " << digit << " out of range [0,9]"
                           << " streaming in low digits \"" << str << "\"");
    low = (low + digit + round) / 10;
  }

  return low.GetLow();
}

std::istream &operator>>(std::istream &is, int64x64_t &value) {
  std::string str;

  is >> str;
  bool negative;
  std::string::size_type cur;
  cur = str.find_first_not_of(' ');
  std::string::size_type next;
  next = str.find('-', cur);
  if (next != std::string::npos) {
    negative = true;
    next++;
  } else {
    next = str.find('+', cur);
    if (next != std::string::npos) {
      next++;
    } else {
      next = cur;
    }
    negative = false;
  }
  cur = next;
  int64_t hi;
  uint64_t lo;
  next = str.find('.', cur);
  if (next != std::string::npos) {
    hi = ReadHiDigits(str.substr(cur, next - cur));
    lo = ReadLoDigits(str.substr(next + 1, str.size() - (next + 1)));
  } else if (cur != std::string::npos) {
    hi = ReadHiDigits(str.substr(cur, str.size() - cur));
    lo = 0;
  } else {
    hi = 0;
    lo = 0;
  }

  value = int64x64_t(hi, lo);
  value = negative ? -value : value;

  return is;
}

} // namespace ns3
