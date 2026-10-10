
#ifndef CALLBACK_H
#define CALLBACK_H

#include "attribute-helper.h"
#include "attribute.h"
#include "fatal-error.h"
#include "ptr.h"
#include "simple-ref-count.h"

#include <functional>
#include <memory>
#include <typeinfo>
#include <utility>
#include <vector>

namespace ns3 {

class CallbackImplBase : public SimpleRefCount<CallbackImplBase> {
public:
  virtual ~CallbackImplBase() {}

  virtual bool IsEqual(Ptr<const CallbackImplBase> other) const = 0;
  virtual std::string GetTypeid() const = 0;

protected:
  static std::string Demangle(const std::string &mangled);

  template <typename T> static std::string GetCppTypeid() {
    std::string typeName;
    try {
      typeName = typeid(T).name();
      typeName = Demangle(typeName);
    } catch (const std::bad_typeid &e) {
      typeName = e.what();
    }
    return typeName;
  }
};

class CallbackComponentBase {
public:
  virtual ~CallbackComponentBase() {}

  virtual bool
  IsEqual(std::shared_ptr<const CallbackComponentBase> other) const = 0;
};

template <typename T, bool isComparable = true>
class CallbackComponent : public CallbackComponentBase {
public:
  CallbackComponent(const T &t) : m_comp(t) {}

  bool
  IsEqual(std::shared_ptr<const CallbackComponentBase> other) const override {
    auto p = std::dynamic_pointer_cast<const CallbackComponent<T>>(other);

    return !(p == nullptr || p->m_comp != m_comp);
  }

private:
  T m_comp;
};

template <typename T>
class CallbackComponent<T, false> : public CallbackComponentBase {
public:
  CallbackComponent(const T &t) {}

  bool
  IsEqual(std::shared_ptr<const CallbackComponentBase> other) const override {
    return false;
  }
};

typedef std::vector<std::shared_ptr<CallbackComponentBase>>
    CallbackComponentVector;

template <typename R, typename... UArgs>
class CallbackImpl : public CallbackImplBase {
public:
  CallbackImpl(std::function<R(UArgs...)> func,
               const CallbackComponentVector &components)
      : m_func(func), m_components(components) {}

  const std::function<R(UArgs...)> &GetFunction() const { return m_func; }

  const CallbackComponentVector &GetComponents() const { return m_components; }

  R operator()(UArgs... uargs) const { return m_func(uargs...); }

  bool IsEqual(Ptr<const CallbackImplBase> other) const override {
    const auto otherDerived =
        dynamic_cast<const CallbackImpl<R, UArgs...> *>(PeekPointer(other));

    if (otherDerived == nullptr) {
      return false;
    }

    if (m_components.size() != otherDerived->GetComponents().size()) {
      return false;
    }

    if (!m_components.at(0)->IsEqual(otherDerived->GetComponents().at(0)) &&
        m_components.at(0) != otherDerived->GetComponents().at(0)) {
      return false;
    }

    for (std::size_t i = 1; i < m_components.size(); i++) {
      if (!m_components.at(i)->IsEqual(otherDerived->GetComponents().at(i))) {
        return false;
      }
    }

    return true;
  }

  std::string GetTypeid() const override { return DoGetTypeid(); }

  static std::string DoGetTypeid() {
    static std::vector<std::string> vec = {GetCppTypeid<R>(),
                                           GetCppTypeid<UArgs>()...};

    static std::string id("CallbackImpl<");
    for (auto &s : vec) {
      id.append(s + ",");
    }
    if (id.back() == ',') {
      id.pop_back();
    }
    id.push_back('>');

    return id;
  }

private:
  std::function<R(UArgs...)> m_func;

  std::vector<std::shared_ptr<CallbackComponentBase>> m_components;
};

class CallbackBase {
public:
  CallbackBase() : m_impl() {}

  Ptr<CallbackImplBase> GetImpl() const { return m_impl; }

protected:
  CallbackBase(Ptr<CallbackImplBase> impl) : m_impl(impl) {}

  Ptr<CallbackImplBase> m_impl;
};

template <typename R, typename... UArgs> class Callback : public CallbackBase {
  template <typename ROther, typename... UArgsOther> friend class Callback;

public:
  Callback() {}

  Callback(const Ptr<CallbackImpl<R, UArgs...>> &impl) : CallbackBase(impl) {}

  template <typename... BArgs>
  Callback(const Callback<R, BArgs..., UArgs...> &cb, BArgs... bargs) {
    auto f = cb.DoPeekImpl()->GetFunction();

    CallbackComponentVector components(cb.DoPeekImpl()->GetComponents());
    components.insert(
        components.end(),
        {std::make_shared<CallbackComponent<std::decay_t<BArgs>>>(bargs)...});

    m_impl = Create<CallbackImpl<R, UArgs...>>(
        [f, bargs...](auto &&...uargs) -> R {
          return f(bargs..., std::forward<decltype(uargs)>(uargs)...);
        },
        components);
  }

  template <typename T,
            std::enable_if_t<!std::is_base_of_v<CallbackBase, T>, int> = 0,
            typename... BArgs>
  Callback(T func, BArgs... bargs) {
    std::function<R(BArgs..., UArgs...)> f(func);

    constexpr bool isComp = std::is_function_v<std::remove_pointer_t<T>> ||
                            std::is_member_pointer_v<T>;

    CallbackComponentVector components(
        {std::make_shared<CallbackComponent<T, isComp>>(func),
         std::make_shared<CallbackComponent<std::decay_t<BArgs>>>(bargs)...});

    m_impl = Create<CallbackImpl<R, UArgs...>>(
        [f, bargs...](auto &&...uargs) -> R {
          return f(bargs..., std::forward<decltype(uargs)>(uargs)...);
        },
        components);
  }

private:
  template <std::size_t... INDEX, typename... BoundArgs>
  auto BindImpl(std::index_sequence<INDEX...> seq, BoundArgs &&...bargs) {
    Callback<R, std::tuple_element_t<sizeof...(bargs) + INDEX,
                                     std::tuple<UArgs...>>...>
        cb;

    const auto f = DoPeekImpl()->GetFunction();

    CallbackComponentVector components(DoPeekImpl()->GetComponents());
    components.insert(
        components.end(),
        {std::make_shared<CallbackComponent<std::decay_t<BoundArgs>>>(
            bargs)...});

    cb.m_impl = Create<std::remove_pointer_t<decltype(cb.DoPeekImpl())>>(
        [f, bargs...](auto &&...uargs) mutable {
          return f(bargs..., std::forward<decltype(uargs)>(uargs)...);
        },
        components);

    return cb;
  }

public:
  template <typename... BoundArgs> auto Bind(BoundArgs &&...bargs) {
    static_assert(sizeof...(UArgs) > 0);
    return BindImpl(
        std::make_index_sequence<sizeof...(UArgs) - sizeof...(BoundArgs)>{},
        std::forward<BoundArgs>(bargs)...);
  }

  bool IsNull() const { return (DoPeekImpl() == nullptr); }

  void Nullify() { m_impl = nullptr; }

  R operator()(UArgs... uargs) const { return (*(DoPeekImpl()))(uargs...); }

  bool IsEqual(const CallbackBase &other) const {
    return m_impl->IsEqual(other.GetImpl());
  }

  bool CheckType(const CallbackBase &other) const {
    return DoCheckType(other.GetImpl());
  }

  bool Assign(const CallbackBase &other) {
    auto otherImpl = other.GetImpl();
    if (!DoCheckType(otherImpl)) {
      std::string othTid = otherImpl->GetTypeid();
      std::string myTid = CallbackImpl<R, UArgs...>::DoGetTypeid();
      NS_FATAL_ERROR_CONT(
          "Incompatible types. (feed to \"c++filt -t\" if needed)"
          << std::endl
          << "got=" << othTid << std::endl
          << "expected=" << myTid);
      return false;
    }
    m_impl = const_cast<CallbackImplBase *>(PeekPointer(otherImpl));
    return true;
  }

private:
  CallbackImpl<R, UArgs...> *DoPeekImpl() const {
    return static_cast<CallbackImpl<R, UArgs...> *>(PeekPointer(m_impl));
  }

  bool DoCheckType(Ptr<const CallbackImplBase> other) const {
    if (other && dynamic_cast<const CallbackImpl<R, UArgs...> *>(
                     PeekPointer(other)) != nullptr) {
      return true;
    } else if (!other) {
      return true;
    } else {
      return false;
    }
  }
};

template <typename R, typename... Args>
bool operator!=(Callback<R, Args...> a, Callback<R, Args...> b) {
  return !a.IsEqual(b);
}

template <typename T, typename OBJ, typename R, typename... Args>
Callback<R, Args...> MakeCallback(R (T::*memPtr)(Args...), OBJ objPtr) {
  return Callback<R, Args...>(memPtr, objPtr);
}

template <typename T, typename OBJ, typename R, typename... Args>
Callback<R, Args...> MakeCallback(R (T::*memPtr)(Args...) const, OBJ objPtr) {
  return Callback<R, Args...>(memPtr, objPtr);
}

template <typename R, typename... Args>
Callback<R, Args...> MakeCallback(R (*fnPtr)(Args...)) {
  return Callback<R, Args...>(fnPtr);
}

template <typename R, typename... Args>
Callback<R, Args...> MakeNullCallback() {
  return Callback<R, Args...>();
}

template <typename R, typename... Args, typename... BArgs>
auto MakeBoundCallback(R (*fnPtr)(Args...), BArgs &&...bargs) {
  return Callback<R, Args...>(fnPtr).Bind(std::forward<BArgs>(bargs)...);
}

template <typename T, typename OBJ, typename R, typename... Args,
          typename... BArgs>
auto MakeCallback(R (T::*memPtr)(Args...), OBJ objPtr, BArgs... bargs) {
  return Callback<R, Args...>(memPtr, objPtr).Bind(bargs...);
}

template <typename T, typename OBJ, typename R, typename... Args,
          typename... BArgs>
auto MakeCallback(R (T::*memPtr)(Args...) const, OBJ objPtr, BArgs... bargs) {
  return Callback<R, Args...>(memPtr, objPtr).Bind(bargs...);
}

} // namespace ns3

namespace ns3 {

class CallbackValue : public AttributeValue {
public:
  CallbackValue();
  CallbackValue(const CallbackBase &base);
  ~CallbackValue() override;
  void Set(CallbackBase base);
  template <typename T> bool GetAccessor(T &value) const;
  Ptr<AttributeValue> Copy() const override;
  std::string
  SerializeToString(Ptr<const AttributeChecker> checker) const override;
  bool DeserializeFromString(std::string value,
                             Ptr<const AttributeChecker> checker) override;

private:
  CallbackBase m_value;
};

ATTRIBUTE_ACCESSOR_DEFINE(Callback);
ATTRIBUTE_CHECKER_DEFINE(Callback);

} // namespace ns3

namespace ns3 {

template <typename T> bool CallbackValue::GetAccessor(T &value) const {
  if (value.CheckType(m_value)) {
    if (!value.Assign(m_value)) {
      NS_FATAL_ERROR_NO_MSG();
    }
    return true;
  }
  return false;
}

} // namespace ns3

#endif
