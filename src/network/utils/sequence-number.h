
#ifndef NS3_SEQ_NUM_H
#define NS3_SEQ_NUM_H

#include "ns3/type-name.h"

#include <iostream>
#include <limits>
#include <stdint.h>

namespace ns3 {

template <typename NUMERIC_TYPE, typename SIGNED_TYPE> class SequenceNumber {
public:
  SequenceNumber() : m_value(0) {}

  explicit SequenceNumber(NUMERIC_TYPE value) : m_value(value) {}

  SequenceNumber(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &value)
      : m_value(value.m_value) {}

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &operator=(NUMERIC_TYPE value) {
    m_value = value;
    return *this;
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &
  operator=(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &value) {
    m_value = value.m_value;
    return *this;
  }

#if 0
  operator NUMERIC_TYPE () const
  {
    return m_value;
  }
#endif

  NUMERIC_TYPE GetValue() const { return m_value; }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> operator++() {
    m_value++;
    return *this;
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> operator++(int) {
    SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> retval(m_value);
    m_value++;
    return retval;
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> operator--() {
    m_value--;
    return *this;
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> operator--(int) {
    SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> retval(m_value);
    m_value--;
    return retval;
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &operator+=(SIGNED_TYPE value) {
    m_value += value;
    return *this;
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &operator-=(SIGNED_TYPE value) {
    m_value -= value;
    return *this;
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator+(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    return SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>(m_value + other.m_value);
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> operator+(SIGNED_TYPE delta) const {
    return SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>(m_value + delta);
  }

  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> operator-(SIGNED_TYPE delta) const {
    return SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>(m_value - delta);
  }

  SIGNED_TYPE
  operator-(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    static const NUMERIC_TYPE maxValue =
        std::numeric_limits<NUMERIC_TYPE>::max();
    static const NUMERIC_TYPE halfMaxValue =
        std::numeric_limits<NUMERIC_TYPE>::max() / 2;
    if (m_value > other.m_value) {
      NUMERIC_TYPE diff = m_value - other.m_value;
      if (diff < halfMaxValue) {
        return static_cast<SIGNED_TYPE>(diff);
      } else {
        return -(
            static_cast<SIGNED_TYPE>(maxValue - m_value + 1 + other.m_value));
      }
    } else {
      NUMERIC_TYPE diff = other.m_value - m_value;
      if (diff < halfMaxValue) {
        return -(static_cast<SIGNED_TYPE>(diff));
      } else {
        return static_cast<SIGNED_TYPE>(maxValue - other.m_value + 1 + m_value);
      }
    }
  }

  bool operator>(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    static const NUMERIC_TYPE halfMaxValue =
        std::numeric_limits<NUMERIC_TYPE>::max() / 2;

    return (((m_value > other.m_value) &&
             (m_value - other.m_value) <= halfMaxValue) ||
            ((other.m_value > m_value) &&
             (other.m_value - m_value) > halfMaxValue));
  }

  bool
  operator==(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    return (m_value == other.m_value);
  }

  bool
  operator!=(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    return (m_value != other.m_value);
  }

  bool
  operator<=(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    return (!this->operator>(other));
  }

  bool
  operator>=(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    return (this->operator>(other) || this->operator==(other));
  }

  bool operator<(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &other) const {
    return !this->operator>(other) && m_value != other.m_value;
  }

  template <typename NUMERIC_TYPE2, typename SIGNED_TYPE2>
  friend std::ostream &
  operator<<(std::ostream &os,
             const SequenceNumber<NUMERIC_TYPE2, SIGNED_TYPE2> &val);

  template <typename NUMERIC_TYPE2, typename SIGNED_TYPE2>
  friend std::istream &
  operator>>(std::istream &is,
             const SequenceNumber<NUMERIC_TYPE2, SIGNED_TYPE2> &val);

public:
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &
  operator+=(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &
  operator-=(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator*(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator/(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator%(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  bool operator!() const = delete;
  bool
  operator&&(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  bool
  operator||(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> operator~() const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator&(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator|(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator^(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator<<(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
  operator>>(const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &) const = delete;
  int operator*() = delete;

private:
  NUMERIC_TYPE m_value;
};

template <typename NUMERIC_TYPE, typename SIGNED_TYPE>
std::ostream &operator<<(std::ostream &os,
                         const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &val) {
  os << val.m_value;
  return os;
}

template <typename NUMERIC_TYPE, typename SIGNED_TYPE>
std::istream &operator>>(std::istream &is,
                         const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> &val) {
  is >> val.m_value;
  return is;
}

typedef SequenceNumber<uint32_t, int32_t> SequenceNumber32;
typedef SequenceNumber<uint16_t, int16_t> SequenceNumber16;
typedef SequenceNumber<uint8_t, int8_t> SequenceNumber8;

namespace TracedValueCallback {

typedef void (*SequenceNumber32)(SequenceNumber32 oldValue,
                                 SequenceNumber32 newValue);

}

TYPENAMEGET_DEFINE(SequenceNumber32);

} // namespace ns3

#endif
