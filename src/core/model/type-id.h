#ifndef TYPE_ID_H
#define TYPE_ID_H

#include "attribute-accessor-helper.h"
#include "attribute-helper.h"
#include "attribute.h"
#include "callback.h"
#include "hash.h"
#include "trace-source-accessor.h"

#include <stdint.h>
#include <string>

namespace ns3 {

class ObjectBase;

class TypeId {
public:
  enum AttributeFlag {
    ATTR_GET = 1 << 0,
    ATTR_SET = 1 << 1,
    ATTR_CONSTRUCT = 1 << 2,
    ATTR_SGC = ATTR_GET | ATTR_SET | ATTR_CONSTRUCT,
  };

  enum SupportLevel { SUPPORTED, DEPRECATED, OBSOLETE };

  struct AttributeInformation {
    std::string name;
    std::string help;
    uint32_t flags;
    Ptr<const AttributeValue> originalInitialValue;
    Ptr<const AttributeValue> initialValue;
    Ptr<const AttributeAccessor> accessor;
    Ptr<const AttributeChecker> checker;
    TypeId::SupportLevel supportLevel;
    std::string supportMsg;
  };

  struct TraceSourceInformation {
    std::string name;
    std::string help;
    std::string callback;
    Ptr<const TraceSourceAccessor> accessor;
    TypeId::SupportLevel supportLevel;
    std::string supportMsg;
  };

  typedef uint32_t hash_t;

  static TypeId LookupByName(std::string name);
  static bool LookupByNameFailSafe(std::string name, TypeId *tid);
  static TypeId LookupByHash(hash_t hash);
  static bool LookupByHashFailSafe(hash_t hash, TypeId *tid);

  static uint16_t GetRegisteredN();
  static TypeId GetRegistered(uint16_t i);

  explicit TypeId(const std::string &name);

  TypeId GetParent() const;

  bool HasParent() const;

  bool IsChildOf(TypeId other) const;

  std::string GetGroupName() const;

  std::string GetName() const;

  hash_t GetHash() const;

  std::size_t GetSize() const;

  bool HasConstructor() const;

  std::size_t GetAttributeN() const;
  TypeId::AttributeInformation GetAttribute(std::size_t i) const;
  std::string GetAttributeFullName(std::size_t i) const;

  Callback<ObjectBase *> GetConstructor() const;

  bool MustHideFromDocumentation() const;

  std::size_t GetTraceSourceN() const;
  TypeId::TraceSourceInformation GetTraceSource(std::size_t i) const;

  TypeId SetParent(TypeId tid);
  template <typename T> TypeId SetParent();

  TypeId SetGroupName(std::string groupName);

  TypeId SetSize(std::size_t size);

  template <typename T> TypeId AddConstructor();

  TypeId AddAttribute(std::string name, std::string help,
                      const AttributeValue &initialValue,
                      Ptr<const AttributeAccessor> accessor,
                      Ptr<const AttributeChecker> checker,
                      SupportLevel supportLevel = SUPPORTED,
                      const std::string &supportMsg = "");

  bool SetAttributeInitialValue(std::size_t i,
                                Ptr<const AttributeValue> initialValue);

  TypeId AddAttribute(std::string name, std::string help, uint32_t flags,
                      const AttributeValue &initialValue,
                      Ptr<const AttributeAccessor> accessor,
                      Ptr<const AttributeChecker> checker,
                      SupportLevel supportLevel = SUPPORTED,
                      const std::string &supportMsg = "");

  TypeId AddTraceSource(std::string name, std::string help,
                        Ptr<const TraceSourceAccessor> accessor,
                        std::string callback,
                        SupportLevel supportLevel = SUPPORTED,
                        const std::string &supportMsg = "");

  TypeId HideFromDocumentation();

  bool LookupAttributeByName(std::string name,
                             AttributeInformation *info) const;
  Ptr<const TraceSourceAccessor>
  LookupTraceSourceByName(std::string name) const;
  Ptr<const TraceSourceAccessor>
  LookupTraceSourceByName(std::string name, TraceSourceInformation *info) const;

  uint16_t GetUid() const;
  void SetUid(uint16_t uid);

  inline TypeId();
  inline TypeId(const TypeId &o);
  inline TypeId &operator=(const TypeId &o);
  inline ~TypeId();

private:
  friend inline bool operator==(TypeId a, TypeId b);
  friend inline bool operator!=(TypeId a, TypeId b);
  friend bool operator<(TypeId a, TypeId b);

  explicit TypeId(uint16_t tid);
  void DoAddConstructor(Callback<ObjectBase *> callback);

  uint16_t m_tid;
};

std::ostream &operator<<(std::ostream &os, TypeId tid);
std::istream &operator>>(std::istream &is, TypeId &tid);

inline bool operator==(TypeId a, TypeId b);
inline bool operator!=(TypeId a, TypeId b);
bool operator<(TypeId a, TypeId b);

ATTRIBUTE_HELPER_HEADER(TypeId);

} // namespace ns3

namespace ns3 {

TypeId::TypeId() : m_tid(0) {}

TypeId::TypeId(const TypeId &o) : m_tid(o.m_tid) {}

TypeId &TypeId::operator=(const TypeId &o) {
  m_tid = o.m_tid;
  return *this;
}

TypeId::~TypeId() {}

inline bool operator==(TypeId a, TypeId b) { return a.m_tid == b.m_tid; }

inline bool operator!=(TypeId a, TypeId b) { return a.m_tid != b.m_tid; }

template <typename T> TypeId TypeId::SetParent() {
  return SetParent(T::GetTypeId());
}

template <typename T> TypeId TypeId::AddConstructor() {
  struct Maker {
    static ObjectBase *Create() {
      ObjectBase *base = new T();
      return base;
    }
  };

  Callback<ObjectBase *> cb = MakeCallback(&Maker::Create);
  DoAddConstructor(cb);
  return *this;
}

} // namespace ns3

#endif
