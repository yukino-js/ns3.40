#ifndef TRACE_SOURCE_ACCESSOR_H
#define TRACE_SOURCE_ACCESSOR_H

#include "callback.h"
#include "ptr.h"
#include "simple-ref-count.h"

#include <stdint.h>

namespace ns3 {

class ObjectBase;

class TraceSourceAccessor : public SimpleRefCount<TraceSourceAccessor> {
public:
  TraceSourceAccessor();
  virtual ~TraceSourceAccessor();

  virtual bool ConnectWithoutContext(ObjectBase *obj,
                                     const CallbackBase &cb) const = 0;
  virtual bool Connect(ObjectBase *obj, std::string context,
                       const CallbackBase &cb) const = 0;
  virtual bool DisconnectWithoutContext(ObjectBase *obj,
                                        const CallbackBase &cb) const = 0;
  virtual bool Disconnect(ObjectBase *obj, std::string context,
                          const CallbackBase &cb) const = 0;
};

template <typename T>
Ptr<const TraceSourceAccessor> MakeTraceSourceAccessor(T a);

static inline Ptr<const TraceSourceAccessor> MakeEmptyTraceSourceAccessor() {
  return Ptr<const TraceSourceAccessor>(nullptr);
}

} // namespace ns3

namespace ns3 {

template <typename T, typename SOURCE>
Ptr<const TraceSourceAccessor> DoMakeTraceSourceAccessor(SOURCE T::*a) {
  struct Accessor : public TraceSourceAccessor {
    bool ConnectWithoutContext(ObjectBase *obj,
                               const CallbackBase &cb) const override {
      T *p = dynamic_cast<T *>(obj);
      if (p == nullptr) {
        return false;
      }
      (p->*m_source).ConnectWithoutContext(cb);
      return true;
    }

    bool Connect(ObjectBase *obj, std::string context,
                 const CallbackBase &cb) const override {
      T *p = dynamic_cast<T *>(obj);
      if (p == nullptr) {
        return false;
      }
      (p->*m_source).Connect(cb, context);
      return true;
    }

    bool DisconnectWithoutContext(ObjectBase *obj,
                                  const CallbackBase &cb) const override {
      T *p = dynamic_cast<T *>(obj);
      if (p == nullptr) {
        return false;
      }
      (p->*m_source).DisconnectWithoutContext(cb);
      return true;
    }

    bool Disconnect(ObjectBase *obj, std::string context,
                    const CallbackBase &cb) const override {
      T *p = dynamic_cast<T *>(obj);
      if (p == nullptr) {
        return false;
      }
      (p->*m_source).Disconnect(cb, context);
      return true;
    }

    SOURCE T::*m_source;
  } *accessor = new Accessor();

  accessor->m_source = a;
  return Ptr<const TraceSourceAccessor>(accessor, false);
}

template <typename T>
Ptr<const TraceSourceAccessor> MakeTraceSourceAccessor(T a) {
  return DoMakeTraceSourceAccessor(a);
}

} // namespace ns3

#endif
