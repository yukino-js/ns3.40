#ifndef UINTEGER_H
#define UINTEGER_H

#include "attribute-helper.h"
#include "attribute.h"

#include <limits>
#include <stdint.h>

namespace ns3 {

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(uint64_t, Uinteger);
ATTRIBUTE_ACCESSOR_DEFINE(Uinteger);

template <typename T> Ptr<const AttributeChecker> MakeUintegerChecker();

template <typename T>
Ptr<const AttributeChecker> MakeUintegerChecker(uint64_t min);

template <typename T>
Ptr<const AttributeChecker> MakeUintegerChecker(uint64_t min, uint64_t max);

} // namespace ns3

#include "type-name.h"

namespace ns3 {

namespace internal {

Ptr<const AttributeChecker> MakeUintegerChecker(uint64_t min, uint64_t max,
                                                std::string name);

}

template <typename T> Ptr<const AttributeChecker> MakeUintegerChecker() {
  return internal::MakeUintegerChecker(std::numeric_limits<T>::min(),
                                       std::numeric_limits<T>::max(),
                                       TypeNameGet<T>());
}

template <typename T>
Ptr<const AttributeChecker> MakeUintegerChecker(uint64_t min) {
  return internal::MakeUintegerChecker(min, std::numeric_limits<T>::max(),
                                       TypeNameGet<T>());
}

template <typename T>
Ptr<const AttributeChecker> MakeUintegerChecker(uint64_t min, uint64_t max) {
  return internal::MakeUintegerChecker(min, max, TypeNameGet<T>());
}

} // namespace ns3

#endif
