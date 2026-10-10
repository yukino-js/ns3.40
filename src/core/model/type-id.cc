#include "type-id.h"

#include "hash.h"
#include "log.h"
#include "singleton.h"
#include "trace-source-accessor.h"

#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TypeId");

class IidManager : public Singleton<IidManager> {
public:
  uint16_t AllocateUid(std::string name);
  void SetParent(uint16_t uid, uint16_t parent);
  void SetGroupName(uint16_t uid, std::string groupName);
  void SetSize(uint16_t uid, std::size_t size);
  void AddConstructor(uint16_t uid, Callback<ObjectBase *> callback);
  void HideFromDocumentation(uint16_t uid);
  uint16_t GetUid(std::string name) const;
  uint16_t GetUid(TypeId::hash_t hash) const;
  std::string GetName(uint16_t uid) const;
  TypeId::hash_t GetHash(uint16_t uid) const;
  uint16_t GetParent(uint16_t uid) const;
  std::string GetGroupName(uint16_t uid) const;
  std::size_t GetSize(uint16_t uid) const;
  Callback<ObjectBase *> GetConstructor(uint16_t uid) const;
  bool HasConstructor(uint16_t uid) const;
  uint16_t GetRegisteredN() const;
  uint16_t GetRegistered(uint16_t i) const;
  void AddAttribute(uint16_t uid, std::string name, std::string help,
                    uint32_t flags, Ptr<const AttributeValue> initialValue,
                    Ptr<const AttributeAccessor> accessor,
                    Ptr<const AttributeChecker> checker,
                    TypeId::SupportLevel supportLevel = TypeId::SUPPORTED,
                    const std::string &supportMsg = "");
  void SetAttributeInitialValue(uint16_t uid, std::size_t i,
                                Ptr<const AttributeValue> initialValue);
  std::size_t GetAttributeN(uint16_t uid) const;
  TypeId::AttributeInformation GetAttribute(uint16_t uid, std::size_t i) const;
  void AddTraceSource(uint16_t uid, std::string name, std::string help,
                      Ptr<const TraceSourceAccessor> accessor,
                      std::string callback,
                      TypeId::SupportLevel supportLevel = TypeId::SUPPORTED,
                      const std::string &supportMsg = "");
  std::size_t GetTraceSourceN(uint16_t uid) const;
  TypeId::TraceSourceInformation GetTraceSource(uint16_t uid,
                                                std::size_t i) const;
  bool MustHideFromDocumentation(uint16_t uid) const;

private:
  bool HasTraceSource(uint16_t uid, std::string name);
  bool HasAttribute(uint16_t uid, std::string name);
  static TypeId::hash_t Hasher(const std::string name);

  struct IidInformation {
    std::string name;
    TypeId::hash_t hash;
    uint16_t parent;
    std::string groupName;
    std::size_t size;
    bool hasConstructor;
    Callback<ObjectBase *> constructor;
    bool mustHideFromDocumentation;
    std::vector<TypeId::AttributeInformation> attributes;
    std::vector<TypeId::TraceSourceInformation> traceSources;
    TypeId::SupportLevel supportLevel;
    std::string supportMsg;
  };

  typedef std::vector<IidInformation>::const_iterator Iterator;

  IidManager::IidInformation *LookupInformation(uint16_t uid) const;

  std::vector<IidInformation> m_information;

  typedef std::map<std::string, uint16_t> namemap_t;
  namemap_t m_namemap;

  typedef std::map<TypeId::hash_t, uint16_t> hashmap_t;
  hashmap_t m_hashmap;

  enum { HashChainFlag = 0x80000000 };
};

TypeId::hash_t IidManager::Hasher(const std::string name) {
  static ns3::Hasher hasher(Create<Hash::Function::Murmur3>());
  return hasher.clear().GetHash32(name);
}

#define IID "IidManager"
#define IIDL IID << ": "

uint16_t IidManager::AllocateUid(std::string name) {
  NS_LOG_FUNCTION(IID << name);
  NS_ASSERT_MSG(m_namemap.count(name) == 0,
                "Trying to allocate twice the same uid: " << name);

  TypeId::hash_t hash = Hasher(name) & (~HashChainFlag);
  if (m_hashmap.count(hash) == 1) {
    NS_LOG_ERROR("Hash chaining TypeId for '"
                 << name << "'.  "
                 << "This is not a bug, but is extremely unlikely.  "
                 << "Please contact the ns3 developers.");

    NS_ASSERT_MSG(
        m_hashmap.count(hash | HashChainFlag) == 0,
        "Triplicate hash detected while chaining TypeId for '"
            << name << "'. Please contact the ns3 developers for assistance.");

    IidInformation *hinfo = LookupInformation(GetUid(hash));
    if (name > hinfo->name) {
      NS_LOG_LOGIC(IIDL << "New TypeId '" << name << "' getting chained.");
      hash = hash | HashChainFlag;
    } else {
      NS_LOG_LOGIC(IIDL << "Old TypeId '" << hinfo->name
                        << "' getting chained.");
      uint16_t oldUid = GetUid(hinfo->hash);
      m_hashmap.erase(m_hashmap.find(hinfo->hash));
      hinfo->hash = hash | HashChainFlag;
      m_hashmap.insert(std::make_pair(hinfo->hash, oldUid));
    }
  }

  IidInformation information;
  information.name = name;
  information.hash = hash;
  information.parent = 0;
  information.groupName = "";
  information.size = (std::size_t)(-1);
  information.hasConstructor = false;
  information.mustHideFromDocumentation = false;
  information.supportLevel = TypeId::SUPPORTED;
  m_information.push_back(information);
  std::size_t tuid = m_information.size();
  NS_ASSERT(tuid <= 0xffff);
  auto uid = static_cast<uint16_t>(tuid);

  m_namemap.insert(std::make_pair(name, uid));
  m_hashmap.insert(std::make_pair(hash, uid));
  NS_LOG_LOGIC(IIDL << uid);
  return uid;
}

IidManager::IidInformation *IidManager::LookupInformation(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  NS_ASSERT(uid <= m_information.size() && uid != 0);
  NS_LOG_LOGIC(IIDL << m_information[uid - 1].name);
  return const_cast<IidInformation *>(&m_information[uid - 1]);
}

void IidManager::SetParent(uint16_t uid, uint16_t parent) {
  NS_LOG_FUNCTION(IID << uid << parent);
  NS_ASSERT(parent <= m_information.size());
  IidInformation *information = LookupInformation(uid);
  information->parent = parent;
}

void IidManager::SetGroupName(uint16_t uid, std::string groupName) {
  NS_LOG_FUNCTION(IID << uid << groupName);
  IidInformation *information = LookupInformation(uid);
  information->groupName = groupName;
}

void IidManager::SetSize(uint16_t uid, std::size_t size) {
  NS_LOG_FUNCTION(IID << uid << size);
  IidInformation *information = LookupInformation(uid);
  information->size = size;
}

void IidManager::HideFromDocumentation(uint16_t uid) {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  information->mustHideFromDocumentation = true;
}

void IidManager::AddConstructor(uint16_t uid, Callback<ObjectBase *> callback) {
  NS_LOG_FUNCTION(IID << uid << &callback);
  IidInformation *information = LookupInformation(uid);
  if (information->hasConstructor) {
    NS_FATAL_ERROR(information->name << " already has a constructor.");
  }
  information->hasConstructor = true;
  information->constructor = callback;
}

uint16_t IidManager::GetUid(std::string name) const {
  NS_LOG_FUNCTION(IID << name);
  uint16_t uid = 0;
  auto it = m_namemap.find(name);
  if (it != m_namemap.end()) {
    uid = it->second;
  }
  NS_LOG_LOGIC(IIDL << uid);
  return uid;
}

uint16_t IidManager::GetUid(TypeId::hash_t hash) const {
  NS_LOG_FUNCTION(IID << hash);
  auto it = m_hashmap.find(hash);
  uint16_t uid = 0;
  if (it != m_hashmap.end()) {
    uid = it->second;
  }
  NS_LOG_LOGIC(IIDL << uid);
  return uid;
}

std::string IidManager::GetName(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  NS_LOG_LOGIC(IIDL << information->name);
  return information->name;
}

TypeId::hash_t IidManager::GetHash(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  TypeId::hash_t hash = information->hash;
  NS_LOG_LOGIC(IIDL << hash);
  return hash;
}

uint16_t IidManager::GetParent(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  uint16_t pid = information->parent;
  NS_LOG_LOGIC(IIDL << pid);
  return pid;
}

std::string IidManager::GetGroupName(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  NS_LOG_LOGIC(IIDL << information->groupName);
  return information->groupName;
}

std::size_t IidManager::GetSize(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  std::size_t size = information->size;
  NS_LOG_LOGIC(IIDL << size);
  return size;
}

Callback<ObjectBase *> IidManager::GetConstructor(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  if (!information->hasConstructor) {
    NS_FATAL_ERROR("Requested constructor for "
                   << information->name << " but it does not have one.");
  }
  return information->constructor;
}

bool IidManager::HasConstructor(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  bool hasC = information->hasConstructor;
  NS_LOG_LOGIC(IIDL << hasC);
  return hasC;
}

uint16_t IidManager::GetRegisteredN() const {
  NS_LOG_FUNCTION(IID << m_information.size());
  return static_cast<uint16_t>(m_information.size());
}

uint16_t IidManager::GetRegistered(uint16_t i) const {
  NS_LOG_FUNCTION(IID << i);
  return i + 1;
}

bool IidManager::HasAttribute(uint16_t uid, std::string name) {
  NS_LOG_FUNCTION(IID << uid << name);
  IidInformation *information = LookupInformation(uid);
  while (true) {
    for (auto i = information->attributes.begin();
         i != information->attributes.end(); ++i) {
      if (i->name == name) {
        NS_LOG_LOGIC(IIDL << true);
        return true;
      }
    }
    IidInformation *parent = LookupInformation(information->parent);
    if (parent == information) {
      NS_LOG_LOGIC(IIDL << false);
      return false;
    }
    information = parent;
  }
  NS_LOG_LOGIC(IIDL << false);
  return false;
}

void IidManager::AddAttribute(uint16_t uid, std::string name, std::string help,
                              uint32_t flags,
                              Ptr<const AttributeValue> initialValue,
                              Ptr<const AttributeAccessor> accessor,
                              Ptr<const AttributeChecker> checker,
                              TypeId::SupportLevel supportLevel,
                              const std::string &supportMsg) {
  NS_LOG_FUNCTION(IID << uid << name << help << flags << initialValue
                      << accessor << checker << supportLevel << supportMsg);
  IidInformation *information = LookupInformation(uid);
  if (name.find(' ') != std::string::npos) {
    NS_FATAL_ERROR("Attribute name \""
                   << name << "\" may not contain spaces ' ', "
                   << "encountered when registering TypeId \""
                   << information->name << "\"");
  }
  if (HasAttribute(uid, name)) {
    NS_FATAL_ERROR("Attribute \"" << name << "\" already registered on tid=\""
                                  << information->name << "\"");
  }
  TypeId::AttributeInformation info;
  info.name = name;
  info.help = help;
  info.flags = flags;
  info.initialValue = initialValue;
  info.originalInitialValue = initialValue;
  info.accessor = accessor;
  info.checker = checker;
  info.supportLevel = supportLevel;
  info.supportMsg = supportMsg;
  information->attributes.push_back(info);
  NS_LOG_LOGIC(IIDL << information->attributes.size() - 1);
}

void IidManager::SetAttributeInitialValue(
    uint16_t uid, std::size_t i, Ptr<const AttributeValue> initialValue) {
  NS_LOG_FUNCTION(IID << uid << i << initialValue);
  IidInformation *information = LookupInformation(uid);
  NS_ASSERT(i < information->attributes.size());
  information->attributes[i].initialValue = initialValue;
}

std::size_t IidManager::GetAttributeN(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  std::size_t size = information->attributes.size();
  NS_LOG_LOGIC(IIDL << size);
  return size;
}

TypeId::AttributeInformation IidManager::GetAttribute(uint16_t uid,
                                                      std::size_t i) const {
  NS_LOG_FUNCTION(IID << uid << i);
  IidInformation *information = LookupInformation(uid);
  NS_ASSERT(i < information->attributes.size());
  NS_LOG_LOGIC(IIDL << information->name);
  return information->attributes[i];
}

bool IidManager::HasTraceSource(uint16_t uid, std::string name) {
  NS_LOG_FUNCTION(IID << uid << name);
  IidInformation *information = LookupInformation(uid);
  while (true) {
    for (auto i = information->traceSources.begin();
         i != information->traceSources.end(); ++i) {
      if (i->name == name) {
        NS_LOG_LOGIC(IIDL << true);
        return true;
      }
    }
    IidInformation *parent = LookupInformation(information->parent);
    if (parent == information) {
      NS_LOG_LOGIC(IIDL << false);
      return false;
    }
    information = parent;
  }
  NS_LOG_LOGIC(IIDL << false);
  return false;
}

void IidManager::AddTraceSource(uint16_t uid, std::string name,
                                std::string help,
                                Ptr<const TraceSourceAccessor> accessor,
                                std::string callback,
                                TypeId::SupportLevel supportLevel,
                                const std::string &supportMsg) {
  NS_LOG_FUNCTION(IID << uid << name << help << accessor << callback
                      << supportLevel << supportMsg);
  IidInformation *information = LookupInformation(uid);
  if (HasTraceSource(uid, name)) {
    NS_FATAL_ERROR("Trace source \"" << name
                                     << "\" already registered on tid=\""
                                     << information->name << "\"");
  }
  TypeId::TraceSourceInformation source;
  source.name = name;
  source.help = help;
  source.accessor = accessor;
  source.callback = callback;
  source.supportLevel = supportLevel;
  source.supportMsg = supportMsg;
  information->traceSources.push_back(source);
  NS_LOG_LOGIC(IIDL << information->traceSources.size() - 1);
}

std::size_t IidManager::GetTraceSourceN(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  std::size_t size = information->traceSources.size();
  NS_LOG_LOGIC(IIDL << size);
  return size;
}

TypeId::TraceSourceInformation IidManager::GetTraceSource(uint16_t uid,
                                                          std::size_t i) const {
  NS_LOG_FUNCTION(IID << uid << i);
  IidInformation *information = LookupInformation(uid);
  NS_ASSERT(i < information->traceSources.size());
  NS_LOG_LOGIC(IIDL << information->name);
  return information->traceSources[i];
}

bool IidManager::MustHideFromDocumentation(uint16_t uid) const {
  NS_LOG_FUNCTION(IID << uid);
  IidInformation *information = LookupInformation(uid);
  bool hide = information->mustHideFromDocumentation;
  NS_LOG_LOGIC(IIDL << hide);
  return hide;
}

} // namespace ns3

namespace ns3 {

TypeId::TypeId(const std::string &name) {
  NS_LOG_FUNCTION(this << name);
  uint16_t uid = IidManager::Get()->AllocateUid(name);
  NS_LOG_LOGIC(uid);
  NS_ASSERT(uid != 0);
  m_tid = uid;
}

TypeId::TypeId(uint16_t tid) : m_tid(tid) { NS_LOG_FUNCTION(this << tid); }

TypeId TypeId::LookupByName(std::string name) {
  NS_LOG_FUNCTION(name);
  uint16_t uid = IidManager::Get()->GetUid(name);
  NS_ASSERT_MSG(uid != 0,
                "Assert in TypeId::LookupByName: " << name << " not found");
  return TypeId(uid);
}

bool TypeId::LookupByNameFailSafe(std::string name, TypeId *tid) {
  NS_LOG_FUNCTION(name << tid->GetUid());
  uint16_t uid = IidManager::Get()->GetUid(name);
  if (uid == 0) {
    return false;
  }
  *tid = TypeId(uid);
  return true;
}

TypeId TypeId::LookupByHash(hash_t hash) {
  uint16_t uid = IidManager::Get()->GetUid(hash);
  NS_ASSERT_MSG(uid != 0, "Assert in TypeId::LookupByHash: 0x"
                              << std::hex << hash << std::dec << " not found");
  return TypeId(uid);
}

bool TypeId::LookupByHashFailSafe(hash_t hash, TypeId *tid) {
  uint16_t uid = IidManager::Get()->GetUid(hash);
  if (uid == 0) {
    return false;
  }
  *tid = TypeId(uid);
  return true;
}

uint16_t TypeId::GetRegisteredN() {
  NS_LOG_FUNCTION_NOARGS();
  return IidManager::Get()->GetRegisteredN();
}

TypeId TypeId::GetRegistered(uint16_t i) {
  NS_LOG_FUNCTION(i);
  return TypeId(IidManager::Get()->GetRegistered(i));
}

bool TypeId::LookupAttributeByName(std::string name,
                                   TypeId::AttributeInformation *info) const {
  NS_LOG_FUNCTION(this << name << info);
  TypeId tid;
  TypeId nextTid = *this;
  do {
    tid = nextTid;
    for (std::size_t i = 0; i < tid.GetAttributeN(); i++) {
      TypeId::AttributeInformation tmp = tid.GetAttribute(i);
      if (tmp.name == name) {
        if (tmp.supportLevel == TypeId::SUPPORTED) {
          *info = tmp;
          return true;
        } else if (tmp.supportLevel == TypeId::DEPRECATED) {
          std::cerr << "Attribute '" << name
                    << "' is deprecated: " << tmp.supportMsg << std::endl;
          *info = tmp;
          return true;
        } else if (tmp.supportLevel == TypeId::OBSOLETE) {
          NS_FATAL_ERROR("Attribute '" << name
                                       << "' is obsolete, with no fallback: "
                                       << tmp.supportMsg);
        }
      }
    }
    nextTid = tid.GetParent();
  } while (nextTid != tid);
  return false;
}

TypeId TypeId::SetParent(TypeId tid) {
  NS_LOG_FUNCTION(this << tid.GetUid());
  IidManager::Get()->SetParent(m_tid, tid.m_tid);
  return *this;
}

TypeId TypeId::SetGroupName(std::string groupName) {
  NS_LOG_FUNCTION(this << groupName);
  IidManager::Get()->SetGroupName(m_tid, groupName);
  return *this;
}

TypeId TypeId::SetSize(std::size_t size) {
  NS_LOG_FUNCTION(this << size);
  IidManager::Get()->SetSize(m_tid, size);
  return *this;
}

TypeId TypeId::GetParent() const {
  NS_LOG_FUNCTION(this);
  uint16_t parent = IidManager::Get()->GetParent(m_tid);
  return TypeId(parent);
}

bool TypeId::HasParent() const {
  NS_LOG_FUNCTION(this);
  uint16_t parent = IidManager::Get()->GetParent(m_tid);
  return parent != m_tid;
}

bool TypeId::IsChildOf(TypeId other) const {
  NS_LOG_FUNCTION(this << other.GetUid());
  TypeId tmp = *this;
  while (tmp != other && tmp != tmp.GetParent()) {
    tmp = tmp.GetParent();
  }
  return tmp == other && *this != other;
}

std::string TypeId::GetGroupName() const {
  NS_LOG_FUNCTION(this);
  std::string groupName = IidManager::Get()->GetGroupName(m_tid);
  return groupName;
}

std::string TypeId::GetName() const {
  NS_LOG_FUNCTION(this);
  std::string name = IidManager::Get()->GetName(m_tid);
  return name;
}

TypeId::hash_t TypeId::GetHash() const {
  hash_t hash = IidManager::Get()->GetHash(m_tid);
  return hash;
}

std::size_t TypeId::GetSize() const {
  NS_LOG_FUNCTION(this);
  std::size_t size = IidManager::Get()->GetSize(m_tid);
  return size;
}

bool TypeId::HasConstructor() const {
  NS_LOG_FUNCTION(this);
  bool hasConstructor = IidManager::Get()->HasConstructor(m_tid);
  return hasConstructor;
}

void TypeId::DoAddConstructor(Callback<ObjectBase *> cb) {
  NS_LOG_FUNCTION(this << &cb);
  IidManager::Get()->AddConstructor(m_tid, cb);
}

TypeId TypeId::AddAttribute(std::string name, std::string help,
                            const AttributeValue &initialValue,
                            Ptr<const AttributeAccessor> accessor,
                            Ptr<const AttributeChecker> checker,
                            SupportLevel supportLevel,
                            const std::string &supportMsg) {
  NS_LOG_FUNCTION(this << name << help << &initialValue << accessor << checker
                       << supportLevel << supportMsg);
  IidManager::Get()->AddAttribute(m_tid, name, help, ATTR_SGC,
                                  initialValue.Copy(), accessor, checker,
                                  supportLevel, supportMsg);
  return *this;
}

TypeId TypeId::AddAttribute(std::string name, std::string help, uint32_t flags,
                            const AttributeValue &initialValue,
                            Ptr<const AttributeAccessor> accessor,
                            Ptr<const AttributeChecker> checker,
                            SupportLevel supportLevel,
                            const std::string &supportMsg) {
  NS_LOG_FUNCTION(this << name << help << flags << &initialValue << accessor
                       << checker << supportLevel << supportMsg);
  IidManager::Get()->AddAttribute(m_tid, name, help, flags, initialValue.Copy(),
                                  accessor, checker, supportLevel, supportMsg);
  return *this;
}

bool TypeId::SetAttributeInitialValue(std::size_t i,
                                      Ptr<const AttributeValue> initialValue) {
  NS_LOG_FUNCTION(this << i << initialValue);
  IidManager::Get()->SetAttributeInitialValue(m_tid, i, initialValue);
  return true;
}

Callback<ObjectBase *> TypeId::GetConstructor() const {
  NS_LOG_FUNCTION(this);
  Callback<ObjectBase *> cb = IidManager::Get()->GetConstructor(m_tid);
  return cb;
}

bool TypeId::MustHideFromDocumentation() const {
  NS_LOG_FUNCTION(this);
  bool mustHide = IidManager::Get()->MustHideFromDocumentation(m_tid);
  return mustHide;
}

std::size_t TypeId::GetAttributeN() const {
  NS_LOG_FUNCTION(this);
  std::size_t n = IidManager::Get()->GetAttributeN(m_tid);
  return n;
}

TypeId::AttributeInformation TypeId::GetAttribute(std::size_t i) const {
  NS_LOG_FUNCTION(this << i);
  return IidManager::Get()->GetAttribute(m_tid, i);
}

std::string TypeId::GetAttributeFullName(std::size_t i) const {
  NS_LOG_FUNCTION(this << i);
  TypeId::AttributeInformation info = GetAttribute(i);
  return GetName() + "::" + info.name;
}

std::size_t TypeId::GetTraceSourceN() const {
  NS_LOG_FUNCTION(this);
  return IidManager::Get()->GetTraceSourceN(m_tid);
}

TypeId::TraceSourceInformation TypeId::GetTraceSource(std::size_t i) const {
  NS_LOG_FUNCTION(this << i);
  return IidManager::Get()->GetTraceSource(m_tid, i);
}

TypeId TypeId::AddTraceSource(std::string name, std::string help,
                              Ptr<const TraceSourceAccessor> accessor,
                              std::string callback, SupportLevel supportLevel,
                              const std::string &supportMsg) {
  NS_LOG_FUNCTION(this << name << help << accessor << callback << supportLevel
                       << supportMsg);
  IidManager::Get()->AddTraceSource(m_tid, name, help, accessor, callback,
                                    supportLevel, supportMsg);
  return *this;
}

TypeId TypeId::HideFromDocumentation() {
  NS_LOG_FUNCTION(this);
  IidManager::Get()->HideFromDocumentation(m_tid);
  return *this;
}

Ptr<const TraceSourceAccessor>
TypeId::LookupTraceSourceByName(std::string name,
                                TraceSourceInformation *info) const {
  NS_LOG_FUNCTION(this << name);
  TypeId tid;
  TypeId nextTid = *this;
  TypeId::TraceSourceInformation tmp;
  do {
    tid = nextTid;
    for (std::size_t i = 0; i < tid.GetTraceSourceN(); i++) {
      tmp = tid.GetTraceSource(i);
      if (tmp.name == name) {
        if (tmp.supportLevel == TypeId::SUPPORTED) {
          *info = tmp;
          return tmp.accessor;
        } else if (tmp.supportLevel == TypeId::DEPRECATED) {
          std::cerr << "TraceSource '" << name
                    << "' is deprecated: " << tmp.supportMsg << std::endl;
          *info = tmp;
          return tmp.accessor;
        } else if (tmp.supportLevel == TypeId::OBSOLETE) {
          NS_FATAL_ERROR("TraceSource '" << name
                                         << "' is obsolete, with no fallback: "
                                         << tmp.supportMsg);
        }
      }
    }
    nextTid = tid.GetParent();
  } while (nextTid != tid);
  return nullptr;
}

Ptr<const TraceSourceAccessor>
TypeId::LookupTraceSourceByName(std::string name) const {
  TraceSourceInformation info;
  return LookupTraceSourceByName(name, &info);
}

uint16_t TypeId::GetUid() const {
  NS_LOG_FUNCTION(this);
  return m_tid;
}

void TypeId::SetUid(uint16_t uid) {
  NS_LOG_FUNCTION(this << uid);
  m_tid = uid;
}

std::ostream &operator<<(std::ostream &os, TypeId tid) {
  os << tid.GetName();
  return os;
}

std::istream &operator>>(std::istream &is, TypeId &tid) {
  std::string tidString;
  is >> tidString;
  bool ok = TypeId::LookupByNameFailSafe(tidString, &tid);
  if (!ok) {
    is.setstate(std::ios_base::badbit);
  }
  return is;
}

ATTRIBUTE_HELPER_CPP(TypeId);

bool operator<(TypeId a, TypeId b) { return a.m_tid < b.m_tid; }

} // namespace ns3
