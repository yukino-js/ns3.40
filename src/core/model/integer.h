#ifndef INTEGER_H
#define INTEGER_H

#include "attribute-helper.h"
#include "attribute.h"

#include <limits>
#include <stdint.h>

namespace ns3 {

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(int64_t, Integer);
ATTRIBUTE_ACCESSOR_DEFINE(Integer);

template <typename T> Ptr<const AttributeChecker> MakeIntegerChecker();

template <typename T>
Ptr<const AttributeChecker> MakeIntegerChecker(int64_t min);

template <typename T>
Ptr<const AttributeChecker> MakeIntegerChecker(int64_t min, int64_t max);

} // namespace ns3

#include "type-name.h"

namespace ns3 {

namespace internal {

Ptr<const AttributeChecker> MakeIntegerChecker(int64_t min, int64_t max,
                                               std::string name);

}

template <typename T>
Ptr<const AttributeChecker> MakeIntegerChecker(int64_t min, int64_t max) {
  return internal::MakeIntegerChecker(min, max, TypeNameGet<T>());
}

template <typename T>
Ptr<const AttributeChecker> MakeIntegerChecker(int64_t min) {
  return internal::MakeIntegerChecker(min, std::numeric_limits<T>::max(),
                                      TypeNameGet<T>());
}

template <typename T> Ptr<const AttributeChecker> MakeIntegerChecker() {
  return internal::MakeIntegerChecker(std::numeric_limits<T>::min(),
                                      std::numeric_limits<T>::max(),
                                      TypeNameGet<T>());
}

} // namespace ns3

#endif
