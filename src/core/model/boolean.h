#ifndef BOOLEAN_H
#define BOOLEAN_H

#include "attribute-helper.h"
#include "attribute.h"

namespace ns3 {

class BooleanValue : public AttributeValue {
public:
  BooleanValue();
  BooleanValue(bool value);
  void Set(bool value);
  bool Get() const;
  template <typename T> bool GetAccessor(T &v) const;

  operator bool() const;

  Ptr<AttributeValue> Copy() const override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;

private:
  bool m_value;
};

template <typename T> bool BooleanValue::GetAccessor(T &v) const {
  v = T(m_value);
  return true;
}

std::ostream &operator<<(std::ostream &os, const BooleanValue &value);

ATTRIBUTE_CHECKER_DEFINE(Boolean);
ATTRIBUTE_ACCESSOR_DEFINE(Boolean);

} // namespace ns3

#endif
