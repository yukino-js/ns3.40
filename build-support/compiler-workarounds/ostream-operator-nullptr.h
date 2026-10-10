
#ifndef OSTREAM_OPERATOR_NULLPTR_H
#define OSTREAM_OPERATOR_NULLPTR_H

#include <cstddef>
#include <iostream>

namespace std {
inline std::ostream &operator<<(std::ostream &os, std::nullptr_t) {
  return os << "nullptr";
}
} // namespace std

#endif
