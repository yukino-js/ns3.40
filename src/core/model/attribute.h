#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "ptr.h"
#include "simple-ref-count.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class AttributeAccessor;
class AttributeChecker;
class Attribute;
class ObjectBase;

class AttributeValue : public SimpleRefCount<AttributeValue> {
public:
  AttributeValue();
  virtual ~AttributeValue();

  virtual Ptr<AttributeValue> Copy() const = 0;
  virtual std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const = 0;
  virtual bool DeserializeFromString(std::string value,
                                     Ptr<const AttributeChecker> checker) = 0;
};

class AttributeAccessor : public SimpleRefCount<AttributeAccessor> {
public:
  AttributeAccessor();
  virtual ~AttributeAccessor();

  virtual bool Set(ObjectBase *object, const AttributeValue &value) const = 0;
  virtual bool Get(const ObjectBase *object,
                   AttributeValue &attribute) const = 0;

  virtual bool HasGetter() const = 0;
  virtual bool HasSetter() const = 0;
};

class AttributeChecker : public SimpleRefCount<AttributeChecker> {
public:
  AttributeChecker();
  virtual ~AttributeChecker();

  Ptr<AttributeValue> CreateValidValue(const AttributeValue &value) const;
  virtual bool Check(const AttributeValue &value) const = 0;
  virtual std::string GetValueTypeName() const = 0;
  virtual bool HasUnderlyingTypeInformation() const = 0;
  virtual std::string GetUnderlyingTypeInformation() const = 0;
  virtual Ptr<AttributeValue> Create() const = 0;
  virtual bool Copy(const AttributeValue &source,
                    AttributeValue &destination) const = 0;
};

class EmptyAttributeValue : public AttributeValue {
public:
  EmptyAttributeValue();

private:
  Ptr<AttributeValue> Copy() const override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;
};

class EmptyAttributeAccessor : public AttributeAccessor {
public:
  EmptyAttributeAccessor();
  ~EmptyAttributeAccessor() override;
  bool Set(ObjectBase *object, const AttributeValue &value) const override;
  bool Get(const ObjectBase *object, AttributeValue &attribute) const override;
  bool HasGetter() const override;
  bool HasSetter() const override;
};

static inline Ptr<const AttributeAccessor> MakeEmptyAttributeAccessor() {
  return Ptr<const AttributeAccessor>(new EmptyAttributeAccessor(), false);
}

class EmptyAttributeChecker : public AttributeChecker {
public:
  EmptyAttributeChecker();
  ~EmptyAttributeChecker() override;
  bool Check(const AttributeValue &value) const override;
  std::string GetValueTypeName() const override;
  bool HasUnderlyingTypeInformation() const override;
  std::string GetUnderlyingTypeInformation() const override;
  Ptr<AttributeValue> Create() const override;
  bool Copy(const AttributeValue &source,
            AttributeValue &destination) const override;
};

static inline Ptr<AttributeChecker> MakeEmptyAttributeChecker() {
  return Ptr<AttributeChecker>(new EmptyAttributeChecker(), false);
}

} // namespace ns3

#endif
