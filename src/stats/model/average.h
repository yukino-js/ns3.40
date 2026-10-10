
#ifndef AVERAGE_H
#define AVERAGE_H

#include "basic-data-calculators.h"

#include <cmath>
#include <limits>
#include <ostream>
#include <stdint.h>

namespace ns3 {

template <typename T = double> class Average {
public:
  Average() : m_size(0), m_min(std::numeric_limits<T>::max()), m_max(0) {}

  void Update(const T &x) {
    m_varianceCalculator.Update(x);

    m_min = std::min(x, m_min);
    m_max = std::max(x, m_max);
    m_size++;
  }

  void Reset() {
    m_varianceCalculator.Reset();

    m_size = 0;
    m_min = std::numeric_limits<T>::max();
    m_max = 0;
  }

  uint32_t Count() const { return m_size; }

  T Min() const { return m_min; }

  T Max() const { return m_max; }

  double Avg() const { return m_varianceCalculator.getMean(); }

  double Mean() const { return Avg(); }

  double Var() const { return m_varianceCalculator.getVariance(); }

  double Stddev() const { return std::sqrt(Var()); }

  double Error90() const { return 1.645 * std::sqrt(Var() / Count()); }

  double Error95() const { return 1.960 * std::sqrt(Var() / Count()); }

  double Error99() const { return 2.576 * std::sqrt(Var() / Count()); }

private:
  uint32_t m_size;
  T m_min;
  T m_max;
  MinMaxAvgTotalCalculator<double> m_varianceCalculator;
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const Average<T> &x) {
  if (x.Count() != 0) {
    os << x.Avg() << " (" << x.Stddev() << ") [" << x.Min() << ", " << x.Max()
       << "]";
  } else {
    os << "NA";
  }
  return os;
}
} // namespace ns3
#endif
