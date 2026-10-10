
#ifndef TUPLE_H
#define TUPLE_H

#include "attribute-helper.h"
#include "string.h"

#include <algorithm>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ns3 {

template <class... Args>
std::ostream &operator<<(std::ostream &os, const std::tuple<Args...> &t) {
  std::apply(
      [&os](auto &&...args) {
        std::size_t n{0};
        ((os << args << (++n != sizeof...(Args) ? ", " : "")), ...);
      },
      t);
  return os;
}

template <class... Args> class TupleValue : public AttributeValue {
public:
  typedef std::tuple<Args...> value_type;
  typedef std::tuple<std::invoke_result_t<decltype(&Args::Get), Args>...>
      result_type;

  TupleValue();

  TupleValue(const result_type &value);

  Ptr<AttributeValue> Copy() const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;

  result_type Get() const;
  void Set(const result_type &value);

  value_type GetValue() const;

  template <typename T> bool GetAccessor(T &value) const;

private:
  template <std::size_t... Is>
  bool SetValueImpl(std::index_sequence<Is...>,
                    const std::vector<Ptr<AttributeValue>> &values);

  value_type m_value;
};

template <class T1, class T2> auto MakeTupleValue(T2 t);

class TupleChecker : public AttributeChecker {
public:
  virtual const std::vector<Ptr<const AttributeChecker>> &
  GetCheckers() const = 0;
};

template <class... Args, class... Ts>
Ptr<const AttributeChecker> MakeTupleChecker(Ts... checkers);

template <class... Args, class T1>
Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1);

template <class... Args, class T1, class T2>
Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1, T2 a2);

} // namespace ns3

namespace ns3 {

template <class... Args>
TupleValue<Args...>::TupleValue() : m_value(std::make_tuple(Args()...)) {}

template <class... Args>
TupleValue<Args...>::TupleValue(const result_type &value) {
  Set(value);
}

template <class... Args> Ptr<AttributeValue> TupleValue<Args...>::Copy() const {
  return Create<TupleValue<Args...>>(Get());
}

template <class... Args>
template <std::size_t... Is>
bool TupleValue<Args...>::SetValueImpl(
    std::index_sequence<Is...>,
    const std::vector<Ptr<AttributeValue>> &values) {
  auto valueTuple = std::make_tuple(DynamicCast<Args>(values[Is])...);

  bool ok = ((std::get<Is>(valueTuple) != nullptr) && ...);

  if (ok) {
    m_value = std::make_tuple(Args(*std::get<Is>(valueTuple))...);
  }
  return ok;
}

template <class... Args>
bool TupleValue<Args...>::DeserializeFromString(
    std::string value, Ptr<const AttributeChecker> checker) {
  auto tupleChecker = DynamicCast<const TupleChecker>(checker);
  if (!tupleChecker) {
    return false;
  }

  auto count = tupleChecker->GetCheckers().size();
  if (count != sizeof...(Args)) {
    return false;
  }

  if (value.empty() || value.front() != '{' || value.back() != '}') {
    return false;
  }

  value.erase(value.begin());
  value.pop_back();
  std::replace(value.data(), value.data() + value.size(), ',', ' ');

  std::istringstream iss(value);
  std::vector<Ptr<AttributeValue>> values;
  std::size_t i = 0;

  while (iss >> value) {
    if (i >= count) {
      return false;
    }
    values.push_back(tupleChecker->GetCheckers().at(i++)->CreateValidValue(
        StringValue(value)));
    if (!values.back()) {
      return false;
    }
  }

  if (i != count) {
    return false;
  }

  return SetValueImpl(std::index_sequence_for<Args...>{}, values);
}

template <class... Args>
std::string TupleValue<Args...>::SerializeToString(
    Ptr<const AttributeChecker> checker) const {
  std::ostringstream oss;
  oss << "{" << Get() << "}";
  return oss.str();
}

template <class... Args>
typename TupleValue<Args...>::result_type TupleValue<Args...>::Get() const {
  return std::apply(
      [](Args... values) { return std::make_tuple(values.Get()...); }, m_value);
}

template <class... Args>
void TupleValue<Args...>::Set(
    const typename TupleValue<Args...>::result_type &value) {
  m_value = std::apply(
      [](auto &&...args) { return std::make_tuple(Args(args)...); }, value);
}

template <class... Args>
typename TupleValue<Args...>::value_type TupleValue<Args...>::GetValue() const {
  return m_value;
}

template <class... Args>
template <typename T>
bool TupleValue<Args...>::GetAccessor(T &value) const {
  value = T(Get());
  return true;
}

namespace internal {

template <class... Args> class TupleChecker : public ns3::TupleChecker {
public:
  template <class... Ts>
  TupleChecker(Ts... checkers) : m_checkers{checkers...} {}

  const std::vector<Ptr<const AttributeChecker>> &GetCheckers() const override {
    return m_checkers;
  }

  bool Check(const AttributeValue &value) const override {
    const auto v = dynamic_cast<const TupleValue<Args...> *>(&value);
    if (v == nullptr) {
      return false;
    }
    return std::apply(
        [this](Args... values) {
          std::size_t n{0};
          return (m_checkers[n++]->Check(values) && ...);
        },
        v->GetValue());
  }

  std::string GetValueTypeName() const override { return "ns3::TupleValue"; }

  bool HasUnderlyingTypeInformation() const override { return false; }

  std::string GetUnderlyingTypeInformation() const override { return ""; }

  Ptr<AttributeValue> Create() const override {
    return ns3::Create<TupleValue<Args...>>();
  }

  bool Copy(const AttributeValue &source,
            AttributeValue &destination) const override {
    const auto src = dynamic_cast<const TupleValue<Args...> *>(&source);
    auto dst = dynamic_cast<TupleValue<Args...> *>(&destination);
    if (src == nullptr || dst == nullptr) {
      return false;
    }
    *dst = *src;
    return true;
  }

private:
  std::vector<Ptr<const AttributeChecker>> m_checkers;
};

template <class... Args> struct TupleHelper {
  template <class... Ts>
  static Ptr<const AttributeChecker> MakeTupleChecker(Ts... checkers) {
    return Create<internal::TupleChecker<Args...>>(checkers...);
  }

  template <class T1>
  static Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1) {
    return MakeAccessorHelper<TupleValue<Args...>>(a1);
  }

  template <class T1, class T2>
  static Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1, T2 a2) {
    return MakeAccessorHelper<TupleValue<Args...>>(a1, a2);
  }
};

template <class... Args> struct TupleHelper<std::tuple<Args...>> {
  static TupleValue<Args...>
  MakeTupleValue(const typename TupleValue<Args...>::result_type &t) {
    return TupleValue<Args...>(t);
  }

  template <class... Ts>
  static Ptr<const AttributeChecker> MakeTupleChecker(Ts... checkers) {
    return Create<internal::TupleChecker<Args...>>(checkers...);
  }

  template <class T1>
  static Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1) {
    return MakeAccessorHelper<TupleValue<Args...>>(a1);
  }

  template <class T1, class T2>
  static Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1, T2 a2) {
    return MakeAccessorHelper<TupleValue<Args...>>(a1, a2);
  }
};

} // namespace internal

template <class T1, class T2> auto MakeTupleValue(T2 t) {
  return internal::TupleHelper<T1>::MakeTupleValue(t);
}

template <class... Args, class... Ts>
Ptr<const AttributeChecker> MakeTupleChecker(Ts... checkers) {
  return internal::TupleHelper<Args...>::template MakeTupleChecker<Ts...>(
      checkers...);
}

template <class... Args, class T1>
Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1) {
  return internal::TupleHelper<Args...>::template MakeTupleAccessor<T1>(a1);
}

template <class... Args, class T1, class T2>
Ptr<const AttributeAccessor> MakeTupleAccessor(T1 a1, T2 a2) {
  return internal::TupleHelper<Args...>::template MakeTupleAccessor<T1, T2>(a1,
                                                                            a2);
}

} // namespace ns3

#endif
