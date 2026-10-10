
#include "lte-rlc-sequence-number.h"

namespace ns3 {

std::ostream &operator<<(std::ostream &os, const SequenceNumber10 &val) {
  os << val.m_value;
  return os;
}

} // namespace ns3
