
#ifndef LOLLIPOP_COUNTER_H
#define LOLLIPOP_COUNTER_H

#include "ns3/abort.h"

#include <limits>

namespace ns3 {

template <class T> class LollipopCounter {
public:
  LollipopCounter() {
    NS_ABORT_MSG_UNLESS(
        std::is_unsigned<T>::value,
        "Lollipop counters must be defined on unsigned integer types");

    uint16_t numberofDigits = std::numeric_limits<T>::digits;
    m_sequenceWindow = 1 << (numberofDigits / 2);

    m_value = (m_maxValue - m_sequenceWindow) + 1;
  }

  LollipopCounter(T val) {
    uint16_t numberofDigits = std::numeric_limits<T>::digits;
    m_sequenceWindow = 1 << (numberofDigits / 2);

    m_value = val;
  }

  inline LollipopCounter &operator=(const LollipopCounter &o) {
    m_value = o.m_value;
    return *this;
  }

  void Reset() { m_value = (m_maxValue - m_sequenceWindow) + 1; }

  void SetSequenceWindowSize(uint16_t numberOfBits) {
    uint16_t numberofDigits = std::numeric_limits<T>::digits;

    NS_ABORT_MSG_IF(numberOfBits >= numberofDigits,
                    "The size of the Sequence Window should be less than the "
                    "counter size (which is "
                        << +m_maxValue << ")");

    m_sequenceWindow = 1 << numberOfBits;

    m_value = (m_maxValue - m_sequenceWindow) + 1;
  }

  bool IsComparable(const LollipopCounter &val) const {
    NS_ABORT_MSG_IF(m_sequenceWindow != val.m_sequenceWindow,
                    "Can not compare two Lollipop Counters with different "
                    "sequence windows");

    if ((m_value <= m_circularRegion && val.m_value <= m_circularRegion) ||
        (m_value > m_circularRegion && val.m_value > m_circularRegion)) {
      T absDiff = AbsoluteMagnitudeOfDifference(val);
      if (absDiff > m_sequenceWindow) {
        return false;
      }
    }
    return true;
  }

  bool IsInit() const { return m_value > m_circularRegion; }

  friend bool operator==(const LollipopCounter &lhs,
                         const LollipopCounter &rhs) {
    NS_ABORT_MSG_IF(lhs.m_sequenceWindow != rhs.m_sequenceWindow,
                    "Can not compare two Lollipop Counters with different "
                    "sequence windows");

    return lhs.m_value == rhs.m_value;
  }

  friend bool operator>(const LollipopCounter &lhs,
                        const LollipopCounter &rhs) {
    NS_ABORT_MSG_IF(lhs.m_sequenceWindow != rhs.m_sequenceWindow,
                    "Can not compare two Lollipop Counters with different "
                    "sequence windows");

    if (lhs.m_value == rhs.m_value) {
      return false;
    }

    if ((lhs.m_value <= m_circularRegion && rhs.m_value <= m_circularRegion) ||
        (lhs.m_value > m_circularRegion && rhs.m_value > m_circularRegion)) {

      T absDiff = lhs.AbsoluteMagnitudeOfDifference(rhs);
      if (absDiff > lhs.m_sequenceWindow) {
        return false;
      }

      T serialRegion = ((m_circularRegion >> 1) + 1);
      return (((lhs.m_value < rhs.m_value) &&
               ((rhs.m_value - lhs.m_value) > serialRegion)) ||
              ((lhs.m_value > rhs.m_value) &&
               ((lhs.m_value - rhs.m_value) < serialRegion)));
    }

    bool lhsIsHigher;
    T difference;

    if (lhs.m_value > m_circularRegion && rhs.m_value <= m_circularRegion) {
      lhsIsHigher = true;
      difference = lhs.m_value - rhs.m_value;
    } else {
      lhsIsHigher = false;
      difference = rhs.m_value - lhs.m_value;
    }

    T distance = (m_maxValue - difference) + 1;
    if (distance > lhs.m_sequenceWindow) {
      return lhsIsHigher;
    } else {
      return !lhsIsHigher;
    }

    return false;
  }

  friend bool operator<(const LollipopCounter &lhs,
                        const LollipopCounter &rhs) {
    if (!lhs.IsComparable(rhs)) {
      return false;
    }

    if (lhs > rhs) {
      return false;
    } else if (lhs == rhs) {
      return false;
    }

    return true;
  }

  friend LollipopCounter operator++(LollipopCounter &val) {
    val.m_value++;

    if (val.m_value == val.m_circularRegion + 1) {
      val.m_value = 0;
    }

    return val;
  }

  friend LollipopCounter operator++(LollipopCounter &val, int noop) {
    LollipopCounter ans = val;
    ++(val);
    return ans;
  }

  T GetValue() const { return m_value; }

  friend std::ostream &operator<<(std::ostream &os,
                                  const LollipopCounter &counter) {
    os << +counter.m_value;
    return os;
  }

private:
  T AbsoluteMagnitudeOfDifference(const LollipopCounter &val) const {

    T absDiffDirect =
        std::max(m_value, val.m_value) - std::min(m_value, val.m_value);
    T absDiffWrapped = (std::min(m_value, val.m_value) + m_circularRegion + 1) -
                       std::max(m_value, val.m_value);
    T absDiff = std::min(absDiffDirect, absDiffWrapped);
    return absDiff;
  }

  T m_value;
  T m_sequenceWindow;
  static constexpr T m_maxValue = std::numeric_limits<T>::max();
  static constexpr T m_circularRegion = m_maxValue >> 1;
};

typedef LollipopCounter<uint8_t> LollipopCounter8;
typedef LollipopCounter<uint16_t> LollipopCounter16;

} // namespace ns3

#endif
