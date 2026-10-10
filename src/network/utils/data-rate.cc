
#include "data-rate.h"

#include "ns3/fatal-error.h"
#include "ns3/log.h"
#include "ns3/nstime.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DataRate");

ATTRIBUTE_HELPER_CPP(DataRate);

bool DataRate::DoParse(const std::string s, uint64_t *v) {
  NS_LOG_FUNCTION(s << v);
  std::string::size_type n = s.find_first_not_of("0123456789.");
  if (n != std::string::npos) {
    std::istringstream iss;
    iss.str(s.substr(0, n));
    double r;
    iss >> r;
    std::string trailer = s.substr(n, std::string::npos);
    if (trailer == "bps") {
      *v = (uint64_t)r;
    } else if (trailer == "b/s") {
      *v = (uint64_t)r;
    } else if (trailer == "Bps") {
      *v = (uint64_t)(r * 8);
    } else if (trailer == "B/s") {
      *v = (uint64_t)(r * 8);
    } else if (trailer == "kbps") {
      *v = (uint64_t)(r * 1000);
    } else if (trailer == "kb/s") {
      *v = (uint64_t)(r * 1000);
    } else if (trailer == "Kbps") {
      *v = (uint64_t)(r * 1000);
    } else if (trailer == "Kb/s") {
      *v = (uint64_t)(r * 1000);
    } else if (trailer == "kBps") {
      *v = (uint64_t)(r * 8000);
    } else if (trailer == "kB/s") {
      *v = (uint64_t)(r * 8000);
    } else if (trailer == "KBps") {
      *v = (uint64_t)(r * 8000);
    } else if (trailer == "KB/s") {
      *v = (uint64_t)(r * 8000);
    } else if (trailer == "Kib/s") {
      *v = (uint64_t)(r * 1024);
    } else if (trailer == "KiB/s") {
      *v = (uint64_t)(r * 8192);
    } else if (trailer == "Mbps") {
      *v = (uint64_t)(r * 1000000);
    } else if (trailer == "Mb/s") {
      *v = (uint64_t)(r * 1000000);
    } else if (trailer == "MBps") {
      *v = (uint64_t)(r * 8000000);
    } else if (trailer == "MB/s") {
      *v = (uint64_t)(r * 8000000);
    } else if (trailer == "Mib/s") {
      *v = (uint64_t)(r * 1048576);
    } else if (trailer == "MiB/s") {
      *v = (uint64_t)(r * 1048576 * 8);
    } else if (trailer == "Gbps") {
      *v = (uint64_t)(r * 1000000000);
    } else if (trailer == "Gb/s") {
      *v = (uint64_t)(r * 1000000000);
    } else if (trailer == "GBps") {
      *v = (uint64_t)(r * 8 * 1000000000);
    } else if (trailer == "GB/s") {
      *v = (uint64_t)(r * 8 * 1000000000);
    } else if (trailer == "Gib/s") {
      *v = (uint64_t)(r * 1048576 * 1024);
    } else if (trailer == "GiB/s") {
      *v = (uint64_t)(r * 1048576 * 1024 * 8);
    } else {
      return false;
    }
    return true;
  }
  std::istringstream iss;
  iss.str(s);
  iss >> *v;
  return true;
}

DataRate::DataRate() : m_bps(0) { NS_LOG_FUNCTION(this); }

DataRate::DataRate(uint64_t bps) : m_bps(bps) { NS_LOG_FUNCTION(this << bps); }

DataRate DataRate::operator+(DataRate rhs) const {
  return DataRate(m_bps + rhs.m_bps);
}

DataRate &DataRate::operator+=(DataRate rhs) {
  m_bps += rhs.m_bps;
  return *this;
}

DataRate DataRate::operator-(DataRate rhs) const {
  NS_ASSERT_MSG(m_bps >= rhs.m_bps, "Data Rate cannot be negative.");
  return DataRate(m_bps - rhs.m_bps);
}

DataRate &DataRate::operator-=(DataRate rhs) {
  NS_ASSERT_MSG(m_bps >= rhs.m_bps, "Data Rate cannot be negative.");
  m_bps -= rhs.m_bps;
  return *this;
}

DataRate DataRate::operator*(double rhs) const {
  return DataRate(((uint64_t)(m_bps * rhs)));
}

DataRate &DataRate::operator*=(double rhs) {
  m_bps *= rhs;
  return *this;
}

DataRate DataRate::operator*(uint64_t rhs) const {
  return DataRate(m_bps * rhs);
}

DataRate &DataRate::operator*=(uint64_t rhs) {
  m_bps *= rhs;
  return *this;
}

bool DataRate::operator<(const DataRate &rhs) const {
  return m_bps < rhs.m_bps;
}

bool DataRate::operator<=(const DataRate &rhs) const {
  return m_bps <= rhs.m_bps;
}

bool DataRate::operator>(const DataRate &rhs) const {
  return m_bps > rhs.m_bps;
}

bool DataRate::operator>=(const DataRate &rhs) const {
  return m_bps >= rhs.m_bps;
}

bool DataRate::operator==(const DataRate &rhs) const {
  return m_bps == rhs.m_bps;
}

bool DataRate::operator!=(const DataRate &rhs) const {
  return m_bps != rhs.m_bps;
}

Time DataRate::CalculateBytesTxTime(uint32_t bytes) const {
  NS_LOG_FUNCTION(this << bytes);
  return CalculateBitsTxTime(bytes * 8);
}

Time DataRate::CalculateBitsTxTime(uint32_t bits) const {
  NS_LOG_FUNCTION(this << bits);
  return Seconds(int64x64_t(bits) / m_bps);
}

uint64_t DataRate::GetBitRate() const {
  NS_LOG_FUNCTION(this);
  return m_bps;
}

DataRate::DataRate(std::string rate) {
  NS_LOG_FUNCTION(this << rate);
  bool ok = DoParse(rate, &m_bps);
  if (!ok) {
    NS_FATAL_ERROR("Could not parse rate: " << rate);
  }
}

std::ostream &operator<<(std::ostream &os, const DataRate &rate) {
  os << rate.GetBitRate() << "bps";
  return os;
}

std::istream &operator>>(std::istream &is, DataRate &rate) {
  std::string value;
  is >> value;
  uint64_t v;
  bool ok = DataRate::DoParse(value, &v);
  if (!ok) {
    is.setstate(std::ios_base::failbit);
  }
  rate = DataRate(v);
  return is;
}

double operator*(const DataRate &lhs, const Time &rhs) {
  return rhs.GetSeconds() * lhs.GetBitRate();
}

double operator*(const Time &lhs, const DataRate &rhs) {
  return lhs.GetSeconds() * rhs.GetBitRate();
}

} // namespace ns3
