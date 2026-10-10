#include "object-base.h"

#include "assert.h"
#include "attribute-construction-list.h"
#include "environment-variable.h"
#include "log.h"
#include "string.h"
#include "trace-source-accessor.h"

#include "ns3/core-config.h"

namespace ns3 {
template Callback<ObjectBase *> MakeCallback<ObjectBase *>(ObjectBase *(*)());
template Callback<ObjectBase *>::Callback();
template class CallbackImpl<ObjectBase *>;

NS_LOG_COMPONENT_DEFINE("ObjectBase");

NS_OBJECT_ENSURE_REGISTERED(ObjectBase);

static TypeId GetObjectIid() {
  NS_LOG_FUNCTION_NOARGS();
  TypeId tid = TypeId("ns3::ObjectBase");
  tid.SetParent(tid);
  tid.SetGroupName("Core");
  return tid;
}

TypeId ObjectBase::GetTypeId() {
  NS_LOG_FUNCTION_NOARGS();
  static TypeId tid = GetObjectIid();
  return tid;
}

ObjectBase::~ObjectBase() { NS_LOG_FUNCTION(this); }

void ObjectBase::NotifyConstructionCompleted() { NS_LOG_FUNCTION(this); }

void ObjectBase::ConstructSelf(const AttributeConstructionList &attributes) {
  NS_LOG_FUNCTION(this << &attributes);
  TypeId tid = GetInstanceTypeId();
  do {
    NS_LOG_DEBUG("construct tid=" << tid.GetName()
                                  << ", params=" << tid.GetAttributeN());
    for (uint32_t i = 0; i < tid.GetAttributeN(); i++) {
      TypeId::AttributeInformation info = tid.GetAttribute(i);
      NS_LOG_DEBUG("try to construct \"" << tid.GetName() << "::" << info.name
                                         << "\"");
      Ptr<const AttributeValue> value = attributes.Find(info.checker);
      std::string where = "argument";

      if (!(info.flags & TypeId::ATTR_CONSTRUCT)) {
        if (!value) {
          NS_LOG_DEBUG("skipping, not settable at construction");
          continue;
        } else {
          NS_FATAL_ERROR("Attribute name="
                         << info.name << " tid=" << tid.GetName()
                         << ": initial value cannot be set using attributes");
        }
      }

      if (!value) {
        NS_LOG_DEBUG(
            "trying to set from environment variable NS_ATTRIBUTE_DEFAULT");
        auto [found, val] = EnvironmentVariable::Get(
            "NS_ATTRIBUTE_DEFAULT", tid.GetAttributeFullName(i));
        if (found) {
          NS_LOG_DEBUG("found in environment: " << val);
          value = Create<StringValue>(val);
          where = "env var";
        }
      }

      bool initial{false};
      if (!value) {
        NS_LOG_DEBUG("falling back to initial value from tid");
        value = info.initialValue;
        where = "initial value";
        initial = true;
      }

      if (DoSet(info.accessor, info.checker, *value) || initial) {
        NS_LOG_DEBUG("construct \"" << tid.GetName() << "::" << info.name
                                    << "\" from " << where);
      } else {
      }
    }
    tid = tid.GetParent();
  } while (tid != ObjectBase::GetTypeId());
  NotifyConstructionCompleted();
}

bool ObjectBase::DoSet(Ptr<const AttributeAccessor> accessor,
                       Ptr<const AttributeChecker> checker,
                       const AttributeValue &value) {
  NS_LOG_FUNCTION(this << accessor << checker << &value);
  Ptr<AttributeValue> v = checker->CreateValidValue(value);
  if (!v) {
    return false;
  }
  bool ok = accessor->Set(this, *v);
  return ok;
}

void ObjectBase::SetAttribute(std::string name, const AttributeValue &value) {
  NS_LOG_FUNCTION(this << name << &value);
  TypeId::AttributeInformation info;
  TypeId tid = GetInstanceTypeId();
  if (!tid.LookupAttributeByName(name, &info)) {
    NS_FATAL_ERROR("Attribute name=" << name
                                     << " does not exist for this object: tid="
                                     << tid.GetName());
  }
  if (!(info.flags & TypeId::ATTR_SET) || !info.accessor->HasSetter()) {
    NS_FATAL_ERROR("Attribute name=" << name
                                     << " is not settable for this object: tid="
                                     << tid.GetName());
  }
  if (!DoSet(info.accessor, info.checker, value)) {
    NS_FATAL_ERROR("Attribute name="
                   << name << " could not be set for this object: tid="
                   << tid.GetName());
  }
}

bool ObjectBase::SetAttributeFailSafe(std::string name,
                                      const AttributeValue &value) {
  NS_LOG_FUNCTION(this << name << &value);
  TypeId::AttributeInformation info;
  TypeId tid = GetInstanceTypeId();
  if (!tid.LookupAttributeByName(name, &info)) {
    return false;
  }
  if (!(info.flags & TypeId::ATTR_SET) || !info.accessor->HasSetter()) {
    return false;
  }
  return DoSet(info.accessor, info.checker, value);
}

void ObjectBase::GetAttribute(std::string name, AttributeValue &value) const {
  NS_LOG_FUNCTION(this << name << &value);
  TypeId::AttributeInformation info;
  TypeId tid = GetInstanceTypeId();
  if (!tid.LookupAttributeByName(name, &info)) {
    NS_FATAL_ERROR("Attribute name=" << name
                                     << " does not exist for this object: tid="
                                     << tid.GetName());
  }
  if (!(info.flags & TypeId::ATTR_GET) || !info.accessor->HasGetter()) {
    NS_FATAL_ERROR("Attribute name=" << name
                                     << " is not gettable for this object: tid="
                                     << tid.GetName());
  }
  bool ok = info.accessor->Get(this, value);
  if (ok) {
    return;
  }
  auto str = dynamic_cast<StringValue *>(&value);
  if (str == nullptr) {
    NS_FATAL_ERROR("Attribute name=" << name << " tid=" << tid.GetName()
                                     << ": input value is not a string");
  }
  Ptr<AttributeValue> v = info.checker->Create();
  ok = info.accessor->Get(this, *PeekPointer(v));
  if (!ok) {
    NS_FATAL_ERROR("Attribute name=" << name << " tid=" << tid.GetName()
                                     << ": could not get value");
  }
  str->Set(v->SerializeToString(info.checker));
}

bool ObjectBase::GetAttributeFailSafe(std::string name,
                                      AttributeValue &value) const {
  NS_LOG_FUNCTION(this << name << &value);
  TypeId::AttributeInformation info;
  TypeId tid = GetInstanceTypeId();
  if (!tid.LookupAttributeByName(name, &info)) {
    return false;
  }
  if (!(info.flags & TypeId::ATTR_GET) || !info.accessor->HasGetter()) {
    return false;
  }
  bool ok = info.accessor->Get(this, value);
  if (ok) {
    return true;
  }
  auto str = dynamic_cast<StringValue *>(&value);
  if (str == nullptr) {
    return false;
  }
  Ptr<AttributeValue> v = info.checker->Create();
  ok = info.accessor->Get(this, *PeekPointer(v));
  if (!ok) {
    return false;
  }
  str->Set(v->SerializeToString(info.checker));
  return true;
}

bool ObjectBase::TraceConnectWithoutContext(std::string name,
                                            const CallbackBase &cb) {
  NS_LOG_FUNCTION(this << name << &cb);
  TypeId tid = GetInstanceTypeId();
  Ptr<const TraceSourceAccessor> accessor = tid.LookupTraceSourceByName(name);
  if (!accessor) {
    return false;
  }
  bool ok = accessor->ConnectWithoutContext(this, cb);
  return ok;
}

bool ObjectBase::TraceConnect(std::string name, std::string context,
                              const CallbackBase &cb) {
  NS_LOG_FUNCTION(this << name << context << &cb);
  TypeId tid = GetInstanceTypeId();
  Ptr<const TraceSourceAccessor> accessor = tid.LookupTraceSourceByName(name);
  if (!accessor) {
    return false;
  }
  bool ok = accessor->Connect(this, context, cb);
  return ok;
}

bool ObjectBase::TraceDisconnectWithoutContext(std::string name,
                                               const CallbackBase &cb) {
  NS_LOG_FUNCTION(this << name << &cb);
  TypeId tid = GetInstanceTypeId();
  Ptr<const TraceSourceAccessor> accessor = tid.LookupTraceSourceByName(name);
  if (!accessor) {
    return false;
  }
  bool ok = accessor->DisconnectWithoutContext(this, cb);
  return ok;
}

bool ObjectBase::TraceDisconnect(std::string name, std::string context,
                                 const CallbackBase &cb) {
  NS_LOG_FUNCTION(this << name << context << &cb);
  TypeId tid = GetInstanceTypeId();
  Ptr<const TraceSourceAccessor> accessor = tid.LookupTraceSourceByName(name);
  if (!accessor) {
    return false;
  }
  bool ok = accessor->Disconnect(this, context, cb);
  return ok;
}

} // namespace ns3
