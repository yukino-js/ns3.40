
#ifndef ATTRIBUTE_CONTAINER_H
#define ATTRIBUTE_CONTAINER_H

#include "attribute-helper.h"
#include "string.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <sstream>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace ns3 {

class AttributeChecker;

template <class A, char Sep = ',', template <class...> class C = std::list>
class AttributeContainerValue : public AttributeValue {
public:
  typedef A attribute_type;
  typedef Ptr<A> value_type;
  typedef std::list<value_type> container_type;
  typedef typename container_type::const_iterator const_iterator;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::size_type size_type;
  typedef typename AttributeContainerValue::const_iterator Iterator;

  typedef typename std::invoke_result_t<decltype(&A::Get), A> item_type;
  typedef C<item_type> result_type;

  AttributeContainerValue();

  template <class CONTAINER> AttributeContainerValue(const CONTAINER &c);

  template <class ITER>
  AttributeContainerValue(const ITER begin, const ITER end);

  ~AttributeContainerValue() override;

  Ptr<AttributeValue> Copy() const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;

  result_type Get() const;
  template <class T> void Set(const T &c);
  template <typename T> bool GetAccessor(T &value) const;

  size_type GetN() const;
  Iterator Begin();
  Iterator End();

  size_type size() const;
  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

private:
  template <class ITER>
  Ptr<AttributeContainerValue<A, Sep, C>> CopyFrom(const ITER begin,
                                                   const ITER end);

  container_type m_container;
};

class AttributeContainerChecker : public AttributeChecker {
public:
  virtual void SetItemChecker(Ptr<const AttributeChecker> itemchecker) = 0;
  virtual Ptr<const AttributeChecker> GetItemChecker() const = 0;
};

template <class A, char Sep, template <class...> class C>
Ptr<AttributeChecker>
MakeAttributeContainerChecker(const AttributeContainerValue<A, Sep, C> &value);

template <class A, char Sep = ',', template <class...> class C = std::list>
Ptr<const AttributeChecker>
MakeAttributeContainerChecker(Ptr<const AttributeChecker> itemchecker);

template <class A, char Sep = ',', template <class...> class C = std::list>
Ptr<AttributeChecker> MakeAttributeContainerChecker();

template <typename A, char Sep = ',',
          template <typename...> class C = std::list, typename T1>
Ptr<const AttributeAccessor> MakeAttributeContainerAccessor(T1 a1);

template <typename A, char Sep = ',',
          template <typename...> class C = std::list, typename T1, typename T2>
Ptr<const AttributeAccessor> MakeAttributeContainerAccessor(T1 a1, T2 a2);

} // namespace ns3

namespace ns3 {

namespace internal {

template <class A, char Sep, template <class...> class C>
class AttributeContainerChecker : public ns3::AttributeContainerChecker {
public:
  AttributeContainerChecker();
  explicit AttributeContainerChecker(Ptr<const AttributeChecker> itemchecker);
  void SetItemChecker(Ptr<const AttributeChecker> itemchecker) override;
  Ptr<const AttributeChecker> GetItemChecker() const override;

private:
  Ptr<const AttributeChecker> m_itemchecker;
};

template <class A, char Sep, template <class...> class C>
AttributeContainerChecker<A, Sep, C>::AttributeContainerChecker()
    : m_itemchecker(nullptr) {}

template <class A, char Sep, template <class...> class C>
AttributeContainerChecker<A, Sep, C>::AttributeContainerChecker(
    Ptr<const AttributeChecker> itemchecker)
    : m_itemchecker(itemchecker) {}

template <class A, char Sep, template <class...> class C>
void AttributeContainerChecker<A, Sep, C>::SetItemChecker(
    Ptr<const AttributeChecker> itemchecker) {
  m_itemchecker = itemchecker;
}

template <class A, char Sep, template <class...> class C>
Ptr<const AttributeChecker>
AttributeContainerChecker<A, Sep, C>::GetItemChecker() const {
  return m_itemchecker;
}

} // namespace internal

template <class A, char Sep, template <class...> class C>
Ptr<AttributeChecker>
MakeAttributeContainerChecker(const AttributeContainerValue<A, Sep, C> &value) {
  return MakeAttributeContainerChecker<A, Sep, C>();
}

template <class A, char Sep, template <class...> class C>
Ptr<const AttributeChecker>
MakeAttributeContainerChecker(Ptr<const AttributeChecker> itemchecker) {
  auto checker = MakeAttributeContainerChecker<A, Sep, C>();
  auto acchecker = DynamicCast<AttributeContainerChecker>(checker);
  acchecker->SetItemChecker(itemchecker);
  return checker;
}

template <class A, char Sep, template <class...> class C>
Ptr<AttributeChecker> MakeAttributeContainerChecker() {
  std::string containerType;
  std::string underlyingType;
  typedef AttributeContainerValue<A, Sep, C> T;
  {
    std::ostringstream oss;
    oss << "ns3::AttributeContainerValue<"
        << typeid(typename T::attribute_type).name() << ", "
        << typeid(typename T::container_type).name() << ">";
    containerType = oss.str();
  }

  {
    std::ostringstream oss;
    oss << "ns3::Ptr<" << typeid(typename T::attribute_type).name() << ">";
    underlyingType = oss.str();
  }

  return MakeSimpleAttributeChecker<
      T, internal::AttributeContainerChecker<A, Sep, C>>(containerType,
                                                         underlyingType);
}

template <class A, char Sep, template <class...> class C>
AttributeContainerValue<A, Sep, C>::AttributeContainerValue() {}

template <class A, char Sep, template <class...> class C>
template <class CONTAINER>
AttributeContainerValue<A, Sep, C>::AttributeContainerValue(const CONTAINER &c)
    : AttributeContainerValue<A, Sep, C>(c.begin(), c.end()) {}

template <class A, char Sep, template <class...> class C>
template <class ITER>
AttributeContainerValue<A, Sep, C>::AttributeContainerValue(const ITER begin,
                                                            const ITER end)
    : AttributeContainerValue() {
  CopyFrom(begin, end);
}

template <class A, char Sep, template <class...> class C>
AttributeContainerValue<A, Sep, C>::~AttributeContainerValue() {
  m_container.clear();
}

template <class A, char Sep, template <class...> class C>
Ptr<AttributeValue> AttributeContainerValue<A, Sep, C>::Copy() const {
  auto c = Create<AttributeContainerValue<A, Sep, C>>();
  c->m_container = m_container;
  return c;
}

template <class A, char Sep, template <class...> class C>
bool AttributeContainerValue<A, Sep, C>::DeserializeFromString(
    std::string value, Ptr<const AttributeChecker> checker) {
  auto acchecker = DynamicCast<const AttributeContainerChecker>(checker);
  if (!acchecker) {
    return false;
  }

  std::istringstream iss(value);
  while (std::getline(iss, value, Sep)) {
    auto avalue =
        acchecker->GetItemChecker()->CreateValidValue(StringValue(value));
    if (!avalue) {
      return false;
    }

    auto attr = DynamicCast<A>(avalue);
    if (!attr) {
      return false;
    }

    m_container.push_back(attr);
  }
  return true;
}

template <class A, char Sep, template <class...> class C>
std::string AttributeContainerValue<A, Sep, C>::SerializeToString(
    Ptr<const AttributeChecker> checker) const {
  std::ostringstream oss;
  bool first = true;
  for (auto attr : *this) {
    if (!first) {
      oss << Sep;
    }
    oss << attr->SerializeToString(checker);
    first = false;
  }
  return oss.str();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::result_type
AttributeContainerValue<A, Sep, C>::Get() const {
  result_type c;
  for (const value_type &a : *this) {
    c.insert(c.end(), a->Get());
  }
  return c;
}

template <class A, char Sep, template <class...> class C>
template <typename T>
bool AttributeContainerValue<A, Sep, C>::GetAccessor(T &value) const {
  result_type src = Get();
  value.clear();
  std::copy(src.begin(), src.end(), std::inserter(value, value.end()));
  return true;
}

template <class A, char Sep, template <class...> class C>
template <class T>
void AttributeContainerValue<A, Sep, C>::Set(const T &c) {
  m_container.clear();
  CopyFrom(c.begin(), c.end());
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::size_type
AttributeContainerValue<A, Sep, C>::GetN() const {
  return size();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::Iterator
AttributeContainerValue<A, Sep, C>::Begin() {
  return begin();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::Iterator
AttributeContainerValue<A, Sep, C>::End() {
  return end();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::size_type
AttributeContainerValue<A, Sep, C>::size() const {
  return m_container.size();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::iterator
AttributeContainerValue<A, Sep, C>::begin() {
  return m_container.begin();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::iterator
AttributeContainerValue<A, Sep, C>::end() {
  return m_container.end();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::const_iterator
AttributeContainerValue<A, Sep, C>::begin() const {
  return m_container.cbegin();
}

template <class A, char Sep, template <class...> class C>
typename AttributeContainerValue<A, Sep, C>::const_iterator
AttributeContainerValue<A, Sep, C>::end() const {
  return m_container.cend();
}

template <class A, char Sep, template <class...> class C>
template <class ITER>
Ptr<AttributeContainerValue<A, Sep, C>>
AttributeContainerValue<A, Sep, C>::CopyFrom(const ITER begin, const ITER end) {
  for (ITER iter = begin; iter != end; ++iter) {
    m_container.push_back(Create<A>(*iter));
  }
  return this;
}

template <typename A, char Sep, template <typename...> class C, typename T1>
Ptr<const AttributeAccessor> MakeAttributeContainerAccessor(T1 a1) {
  return MakeAccessorHelper<AttributeContainerValue<A, Sep, C>>(a1);
}

template <typename A, char Sep, template <typename...> class C, typename T1,
          typename T2>
Ptr<const AttributeAccessor> MakeAttributeContainerAccessor(T1 a1, T2 a2) {
  return MakeAccessorHelper<AttributeContainerValue<A, Sep, C>>(a1, a2);
}

} // namespace ns3

#endif
