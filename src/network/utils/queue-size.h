
#ifndef QUEUE_SIZE_H
#define QUEUE_SIZE_H

#include "ns3/abort.h"
#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"

#include <iostream>
#include <string>

namespace ns3 {

enum QueueSizeUnit {
  PACKETS,
  BYTES,
};

class QueueSize {
public:
  QueueSize();
  QueueSize(QueueSizeUnit unit, uint32_t value);
  QueueSize(std::string size);

  bool operator<(const QueueSize &rhs) const;

  bool operator<=(const QueueSize &rhs) const;

  bool operator>(const QueueSize &rhs) const;

  bool operator>=(const QueueSize &rhs) const;

  bool operator==(const QueueSize &rhs) const;

  bool operator!=(const QueueSize &rhs) const;

  QueueSizeUnit GetUnit() const;

  uint32_t GetValue() const;

private:
  static bool DoParse(const std::string s, QueueSizeUnit *unit,
                      uint32_t *value);

  friend std::istream &operator>>(std::istream &is, QueueSize &size);

  QueueSizeUnit m_unit;
  uint32_t m_value;
};

std::ostream &operator<<(std::ostream &os, const QueueSize &size);

std::istream &operator>>(std::istream &is, QueueSize &size);

ATTRIBUTE_HELPER_HEADER(QueueSize);

template <typename Item>
QueueSize operator+(const QueueSize &lhs, const Ptr<Item> &rhs);
template <typename Item>
QueueSize operator+(const Ptr<Item> &lhs, const QueueSize &rhs);

template <typename Item>
QueueSize operator-(const QueueSize &lhs, const Ptr<Item> &rhs);
template <typename Item>
QueueSize operator-(const Ptr<Item> &lhs, const QueueSize &rhs);

template <typename Item>
QueueSize operator+(const QueueSize &lhs, const Ptr<Item> &rhs) {
  if (lhs.GetUnit() == QueueSizeUnit::PACKETS) {
    return QueueSize(lhs.GetUnit(), lhs.GetValue() + 1);
  }
  if (lhs.GetUnit() == QueueSizeUnit::BYTES) {
    return QueueSize(lhs.GetUnit(), lhs.GetValue() + rhs->GetSize());
  }
  NS_FATAL_ERROR("Unknown queue size mode");
}

template <typename Item>
QueueSize operator+(const Ptr<Item> &lhs, const QueueSize &rhs) {
  if (rhs.GetUnit() == QueueSizeUnit::PACKETS) {
    return QueueSize(rhs.GetUnit(), rhs.GetValue() + 1);
  }
  if (rhs.GetUnit() == QueueSizeUnit::BYTES) {
    return QueueSize(rhs.GetUnit(), rhs.GetValue() + lhs->GetSize());
  }
  NS_FATAL_ERROR("Unknown queue size mode");
}

template <typename Item>
QueueSize operator-(const QueueSize &lhs, const Ptr<Item> &rhs) {
  if (lhs.GetUnit() == QueueSizeUnit::PACKETS) {
    NS_ABORT_IF(lhs.GetValue() < 1);
    return QueueSize(lhs.GetUnit(), lhs.GetValue() - 1);
  }
  if (lhs.GetUnit() == QueueSizeUnit::BYTES) {
    NS_ABORT_IF(lhs.GetValue() < rhs->GetSize());
    return QueueSize(lhs.GetUnit(), lhs.GetValue() - rhs->GetSize());
  }
  NS_FATAL_ERROR("Unknown queue size mode");
}

template <typename Item>
QueueSize operator-(const Ptr<Item> &lhs, const QueueSize &rhs) {
  if (rhs.GetUnit() == QueueSizeUnit::PACKETS) {
    NS_ABORT_IF(rhs.GetValue() < 1);
    return QueueSize(rhs.GetUnit(), rhs.GetValue() - 1);
  }
  if (rhs.GetUnit() == QueueSizeUnit::BYTES) {
    NS_ABORT_IF(rhs.GetValue() < lhs->GetSize());
    return QueueSize(rhs.GetUnit(), rhs.GetValue() - lhs->GetSize());
  }
  NS_FATAL_ERROR("Unknown queue size mode");
}

} // namespace ns3

#endif
