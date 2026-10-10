
#include "queue-size.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("QueueSize");

ATTRIBUTE_HELPER_CPP(QueueSize);

bool QueueSize::DoParse(const std::string s, QueueSizeUnit *unit,
                        uint32_t *value) {
  NS_LOG_FUNCTION(s << unit << value);
  std::string::size_type n = s.find_first_not_of("0123456789.");
  if (n != std::string::npos) {
    std::istringstream iss;
    iss.str(s.substr(0, n));
    double r;
    iss >> r;
    std::string trailer = s.substr(n, std::string::npos);
    if (trailer == "B") {
      *unit = QueueSizeUnit::BYTES;
      *value = static_cast<uint32_t>(r);
    } else if (trailer == "kB" || trailer == "KB") {
      *unit = QueueSizeUnit::BYTES;
      *value = static_cast<uint32_t>(r * 1000);
    } else if (trailer == "KiB") {
      *unit = QueueSizeUnit::BYTES;
      *value = static_cast<uint32_t>(r * 1024);
    } else if (trailer == "MB") {
      *unit = QueueSizeUnit::BYTES;
      *value = static_cast<uint32_t>(r * 1000000);
    } else if (trailer == "MiB") {
      *unit = QueueSizeUnit::BYTES;
      *value = static_cast<uint32_t>(r * 1048576);
    } else if (trailer == "p") {
      *unit = QueueSizeUnit::PACKETS;
      *value = static_cast<uint32_t>(r);
    } else if (trailer == "kp" || trailer == "Kp") {
      *unit = QueueSizeUnit::PACKETS;
      *value = static_cast<uint32_t>(r * 1000);
    } else if (trailer == "Kip") {
      *unit = QueueSizeUnit::PACKETS;
      *value = static_cast<uint32_t>(r * 1024);
    } else if (trailer == "Mp") {
      *unit = QueueSizeUnit::PACKETS;
      *value = static_cast<uint32_t>(r * 1000000);
    } else if (trailer == "Mip") {
      *unit = QueueSizeUnit::PACKETS;
      *value = static_cast<uint32_t>(r * 1048576);
    } else {
      return false;
    }
    return true;
  }
  return false;
}

QueueSize::QueueSize() : m_unit(QueueSizeUnit::PACKETS), m_value(0) {
  NS_LOG_FUNCTION(this);
}

QueueSize::QueueSize(QueueSizeUnit unit, uint32_t value)
    : m_unit(unit), m_value(value) {
  NS_LOG_FUNCTION(this << static_cast<uint16_t>(unit) << value);
}

bool QueueSize::operator<(const QueueSize &rhs) const {
  NS_ABORT_MSG_IF(m_unit != rhs.GetUnit(),
                  "Cannot compare heterogeneous sizes");

  return m_value < rhs.m_value;
}

bool QueueSize::operator<=(const QueueSize &rhs) const {
  NS_ABORT_MSG_IF(m_unit != rhs.GetUnit(),
                  "Cannot compare heterogeneous sizes");

  return m_value <= rhs.m_value;
}

bool QueueSize::operator>(const QueueSize &rhs) const {
  NS_ABORT_MSG_IF(m_unit != rhs.GetUnit(),
                  "Cannot compare heterogeneous sizes");

  return m_value > rhs.m_value;
}

bool QueueSize::operator>=(const QueueSize &rhs) const {
  NS_ABORT_MSG_IF(m_unit != rhs.GetUnit(),
                  "Cannot compare heterogeneous sizes");

  return m_value >= rhs.m_value;
}

bool QueueSize::operator==(const QueueSize &rhs) const {
  NS_ABORT_MSG_IF(m_unit != rhs.GetUnit(),
                  "Cannot compare heterogeneous sizes");

  return m_value == rhs.m_value;
}

bool QueueSize::operator!=(const QueueSize &rhs) const {
  NS_ABORT_MSG_IF(m_unit != rhs.GetUnit(),
                  "Cannot compare heterogeneous sizes");

  return m_value != rhs.m_value;
}

QueueSizeUnit QueueSize::GetUnit() const {
  NS_LOG_FUNCTION(this);
  return m_unit;
}

uint32_t QueueSize::GetValue() const {
  NS_LOG_FUNCTION(this);
  return m_value;
}

QueueSize::QueueSize(std::string size) {
  NS_LOG_FUNCTION(this << size);
  bool ok = DoParse(size, &m_unit, &m_value);
  NS_ABORT_MSG_IF(!ok, "Could not parse queue size: " << size);
}

std::ostream &operator<<(std::ostream &os, const QueueSize &size) {
  os << size.GetValue()
     << (size.GetUnit() == QueueSizeUnit::PACKETS ? "p" : "B");
  return os;
}

std::istream &operator>>(std::istream &is, QueueSize &size) {
  std::string value;
  is >> value;
  QueueSizeUnit m;
  uint32_t l;
  bool ok = QueueSize::DoParse(value, &m, &l);
  if (!ok) {
    is.setstate(std::ios_base::failbit);
  }
  size = QueueSize(m, l);
  return is;
}

} // namespace ns3
