
#ifndef TRACED_CALLBACK_H
#define TRACED_CALLBACK_H

#include "callback.h"

#include <list>

namespace ns3 {

template <typename... Ts> class TracedCallback {
public:
  TracedCallback();
  void ConnectWithoutContext(const CallbackBase &callback);
  void Connect(const CallbackBase &callback, std::string path);
  void DisconnectWithoutContext(const CallbackBase &callback);
  void Disconnect(const CallbackBase &callback, std::string path);
  void operator()(Ts... args) const;
  bool IsEmpty() const;

  typedef void (*Uint32Callback)(const uint32_t value);

private:
  typedef std::list<Callback<void, Ts...>> CallbackList;
  CallbackList m_callbackList;
};

} // namespace ns3

namespace ns3 {

template <typename... Ts>
TracedCallback<Ts...>::TracedCallback() : m_callbackList() {}

template <typename... Ts>
void TracedCallback<Ts...>::ConnectWithoutContext(
    const CallbackBase &callback) {
  Callback<void, Ts...> cb;
  if (!cb.Assign(callback)) {
    NS_FATAL_ERROR_NO_MSG();
  }
  m_callbackList.push_back(cb);
}

template <typename... Ts>
void TracedCallback<Ts...>::Connect(const CallbackBase &callback,
                                    std::string path) {
  Callback<void, std::string, Ts...> cb;
  if (!cb.Assign(callback)) {
    NS_FATAL_ERROR("when connecting to " << path);
  }
  Callback<void, Ts...> realCb = cb.Bind(path);
  m_callbackList.push_back(realCb);
}

template <typename... Ts>
void TracedCallback<Ts...>::DisconnectWithoutContext(
    const CallbackBase &callback) {
  for (auto i = m_callbackList.begin(); i != m_callbackList.end();) {
    if ((*i).IsEqual(callback)) {
      i = m_callbackList.erase(i);
    } else {
      i++;
    }
  }
}

template <typename... Ts>
void TracedCallback<Ts...>::Disconnect(const CallbackBase &callback,
                                       std::string path) {
  Callback<void, std::string, Ts...> cb;
  if (!cb.Assign(callback)) {
    NS_FATAL_ERROR("when disconnecting from " << path);
  }
  Callback<void, Ts...> realCb = cb.Bind(path);
  DisconnectWithoutContext(realCb);
}

template <typename... Ts>
void TracedCallback<Ts...>::operator()(Ts... args) const {
  for (auto i = m_callbackList.begin(); i != m_callbackList.end(); i++) {
    (*i)(args...);
  }
}

template <typename... Ts> bool TracedCallback<Ts...>::IsEmpty() const {
  return m_callbackList.empty();
}

} // namespace ns3

#endif
