
#ifndef DATA_RATE_H
#define DATA_RATE_H

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"
#include "ns3/nstime.h"

#include <iostream>
#include <stdint.h>
#include <string>

namespace ns3 {

class DataRate {
public:
  DataRate();
  DataRate(uint64_t bps);
  DataRate(std::string rate);

  DataRate operator+(DataRate rhs) const;

  DataRate &operator+=(DataRate rhs);

  DataRate operator-(DataRate rhs) const;

  DataRate &operator-=(DataRate rhs);

  DataRate operator*(double rhs) const;

  DataRate &operator*=(double rhs);

  DataRate operator*(uint64_t rhs) const;

  DataRate &operator*=(uint64_t rhs);

  bool operator<(const DataRate &rhs) const;

  bool operator<=(const DataRate &rhs) const;

  bool operator>(const DataRate &rhs) const;

  bool operator>=(const DataRate &rhs) const;

  bool operator==(const DataRate &rhs) const;

  bool operator!=(const DataRate &rhs) const;

  Time CalculateBytesTxTime(uint32_t bytes) const;

  Time CalculateBitsTxTime(uint32_t bits) const;

  uint64_t GetBitRate() const;

private:
  static bool DoParse(const std::string s, uint64_t *v);

  friend std::istream &operator>>(std::istream &is, DataRate &rate);

  uint64_t m_bps;
};

std::ostream &operator<<(std::ostream &os, const DataRate &rate);

std::istream &operator>>(std::istream &is, DataRate &rate);

ATTRIBUTE_HELPER_HEADER(DataRate);

double operator*(const DataRate &lhs, const Time &rhs);
double operator*(const Time &lhs, const DataRate &rhs);

namespace TracedValueCallback {

typedef void (*DataRate)(DataRate oldValue, DataRate newValue);

}

} // namespace ns3

#endif
