
#ifndef TYPE_NAME_H
#define TYPE_NAME_H

#include "fatal-error.h"

#include <string>

namespace ns3 {

template <typename T> std::string TypeNameGet() {
  NS_FATAL_ERROR("Type name not defined.");
  return "unknown";
}

#define TYPENAMEGET_DEFINE(T)                                                  \
  template <> inline std::string TypeNameGet<T>() { return #T; }

TYPENAMEGET_DEFINE(bool);
TYPENAMEGET_DEFINE(int8_t);
TYPENAMEGET_DEFINE(int16_t);
TYPENAMEGET_DEFINE(int32_t);
TYPENAMEGET_DEFINE(int64_t);
TYPENAMEGET_DEFINE(uint8_t);
TYPENAMEGET_DEFINE(uint16_t);
TYPENAMEGET_DEFINE(uint32_t);
TYPENAMEGET_DEFINE(uint64_t);
TYPENAMEGET_DEFINE(float);
TYPENAMEGET_DEFINE(double);
TYPENAMEGET_DEFINE(long double);

} // namespace ns3

#endif
