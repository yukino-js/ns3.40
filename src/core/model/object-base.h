#ifndef OBJECT_BASE_H
#define OBJECT_BASE_H

#include "callback.h"
#include "type-id.h"
#include "warnings.h"

#include <list>
#include <string>

#define NS_OBJECT_ENSURE_REGISTERED(type)                                      \
  static struct Object##type##RegistrationClass {                              \
    Object##type##RegistrationClass() {                                        \
      NS_WARNING_PUSH_DEPRECATED;                                              \
      ns3::TypeId tid = type::GetTypeId();                                     \
      tid.SetSize(sizeof(type));                                               \
      tid.GetParent();                                                         \
      NS_WARNING_POP;                                                          \
    }                                                                          \
  } Object##type##RegistrationVariable

#define NS_OBJECT_TEMPLATE_CLASS_DEFINE(type, param)                           \
  template class type<param>;                                                  \
  template <> std::string DoGetTemplateClassName<type<param>>() {              \
    return std::string("ns3::") + std::string(#type) + std::string("<") +      \
           std::string(#param) + std::string(">");                             \
  }                                                                            \
  static struct Object##type##param##RegistrationClass {                       \
    Object##type##param##RegistrationClass() {                                 \
      ns3::TypeId tid = type<param>::GetTypeId();                              \
      tid.SetSize(sizeof(type<param>));                                        \
      tid.GetParent();                                                         \
    }                                                                          \
  } Object##type##param##RegistrationVariable

#define NS_OBJECT_TEMPLATE_CLASS_TWO_DEFINE(type, param1, param2)              \
  template class type<param1, param2>;                                         \
  template <> std::string DoGetTemplateClassName<type<param1, param2>>() {     \
    return std::string("ns3::") + std::string(#type) + std::string("<") +      \
           std::string(#param1) + std::string(",") + std::string(#param2) +    \
           std::string(">");                                                   \
  }                                                                            \
  static struct Object##type##param1##param2##RegistrationClass {              \
    Object##type##param1##param2##RegistrationClass() {                        \
      ns3::TypeId tid = type<param1, param2>::GetTypeId();                     \
      tid.SetSize(sizeof(type<param1, param2>));                               \
      tid.GetParent();                                                         \
    }                                                                          \
  } Object##type##param1##param2##RegistrationVariable

namespace ns3 {

template <typename T> std::string DoGetTemplateClassName();

template <typename T> std::string GetTemplateClassName() {
  return DoGetTemplateClassName<T>();
}

class AttributeConstructionList;

class ObjectBase {
public:
  static TypeId GetTypeId();

  virtual ~ObjectBase();

  virtual TypeId GetInstanceTypeId() const = 0;

  void SetAttribute(std::string name, const AttributeValue &value);
  bool SetAttributeFailSafe(std::string name, const AttributeValue &value);
  void GetAttribute(std::string name, AttributeValue &value) const;
  bool GetAttributeFailSafe(std::string name, AttributeValue &value) const;

  bool TraceConnect(std::string name, std::string context,
                    const CallbackBase &cb);
  bool TraceConnectWithoutContext(std::string name, const CallbackBase &cb);
  bool TraceDisconnect(std::string name, std::string context,
                       const CallbackBase &cb);
  bool TraceDisconnectWithoutContext(std::string name, const CallbackBase &cb);

protected:
  virtual void NotifyConstructionCompleted();
  void ConstructSelf(const AttributeConstructionList &attributes);

private:
  bool DoSet(Ptr<const AttributeAccessor> spec,
             Ptr<const AttributeChecker> checker, const AttributeValue &value);
};

extern template Callback<ObjectBase *>
MakeCallback<ObjectBase *>(ObjectBase *(*)());
extern template Callback<ObjectBase *>::Callback();
extern template class CallbackImpl<ObjectBase *>;

} // namespace ns3

#endif
