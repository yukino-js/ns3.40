#ifndef NS_DOUBLE_H
#define NS_DOUBLE_H

#include "attribute-helper.h"
#include "attribute.h"

#include <limits>
#include <stdint.h>

namespace ns3 {

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(double, Double);
ATTRIBUTE_ACCESSOR_DEFINE(Double);

template <typename T> Ptr<const AttributeChecker> MakeDoubleChecker();

template <typename T> Ptr<const AttributeChecker> MakeDoubleChecker(double min);

template <typename T>
Ptr<const AttributeChecker> MakeDoubleChecker(double min, double max);

} // namespace ns3

#include "type-name.h"

namespace ns3 {

namespace internal {

Ptr<const AttributeChecker> MakeDoubleChecker(double min, double max,
                                              std::string name);

}

template <typename T> Ptr<const AttributeChecker> MakeDoubleChecker() {
  return internal::MakeDoubleChecker(-std::numeric_limits<T>::max(),
                                     std::numeric_limits<T>::max(),
                                     TypeNameGet<T>());
}

template <typename T>
Ptr<const AttributeChecker> MakeDoubleChecker(double min) {
  return internal::MakeDoubleChecker(min, std::numeric_limits<T>::max(),
                                     TypeNameGet<T>());
}

template <typename T>
Ptr<const AttributeChecker> MakeDoubleChecker(double min, double max) {
  return internal::MakeDoubleChecker(min, max, TypeNameGet<T>());
}

} // namespace ns3

#endif
