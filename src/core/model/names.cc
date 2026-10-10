
#include "names.h"

#include "abort.h"
#include "assert.h"
#include "log.h"
#include "object.h"
#include "singleton.h"

#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Names");

class NameNode {
public:
  NameNode();
  NameNode(const NameNode &nameNode);
  NameNode(NameNode *parent, std::string name, Ptr<Object> object);
  NameNode &operator=(const NameNode &rhs);

  ~NameNode();

  NameNode *m_parent;
  std::string m_name;
  Ptr<Object> m_object;

  std::map<std::string, NameNode *> m_nameMap;
};

NameNode::NameNode() : m_parent(nullptr), m_name(""), m_object(nullptr) {}

NameNode::NameNode(const NameNode &nameNode) {
  m_parent = nameNode.m_parent;
  m_name = nameNode.m_name;
  m_object = nameNode.m_object;
  m_nameMap = nameNode.m_nameMap;
}

NameNode &NameNode::operator=(const NameNode &rhs) {
  m_parent = rhs.m_parent;
  m_name = rhs.m_name;
  m_object = rhs.m_object;
  m_nameMap = rhs.m_nameMap;
  return *this;
}

NameNode::NameNode(NameNode *parent, std::string name, Ptr<Object> object)
    : m_parent(parent), m_name(name), m_object(object) {
  NS_LOG_FUNCTION(this << parent << name << object);
}

NameNode::~NameNode() { NS_LOG_FUNCTION(this); }

class NamesPriv : public Singleton<NamesPriv> {
public:
  NamesPriv();
  ~NamesPriv() override;

  bool Add(std::string name, Ptr<Object> object);
  bool Add(std::string path, std::string name, Ptr<Object> object);
  bool Add(Ptr<Object> context, std::string name, Ptr<Object> object);

  bool Rename(std::string oldpath, std::string newname);
  bool Rename(std::string path, std::string oldname, std::string newname);
  bool Rename(Ptr<Object> context, std::string oldname, std::string newname);

  std::string FindName(Ptr<Object> object);
  std::string FindPath(Ptr<Object> object);

  void Clear();

  Ptr<Object> Find(std::string path);
  Ptr<Object> Find(std::string path, std::string name);
  Ptr<Object> Find(Ptr<Object> context, std::string name);

private:
  NameNode *IsNamed(Ptr<Object> object);
  bool IsDuplicateName(NameNode *node, std::string name);

  NameNode m_root;

  std::map<Ptr<Object>, NameNode *> m_objectMap;
};

NamesPriv::NamesPriv() {
  NS_LOG_FUNCTION(this);

  m_root.m_parent = nullptr;
  m_root.m_name = "Names";
  m_root.m_object = nullptr;
}

NamesPriv::~NamesPriv() {
  NS_LOG_FUNCTION(this);
  Clear();
  m_root.m_name = "";
}

void NamesPriv::Clear() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_objectMap.begin(); i != m_objectMap.end(); ++i) {
    delete i->second;
    i->second = nullptr;
  }

  m_objectMap.clear();

  m_root.m_parent = nullptr;
  m_root.m_name = "Names";
  m_root.m_object = nullptr;
  m_root.m_nameMap.clear();
}

bool NamesPriv::Add(std::string name, Ptr<Object> object) {
  NS_LOG_FUNCTION(this << name << object);
  std::string namespaceName = "/Names";
  std::string::size_type offset = name.find(namespaceName);
  if (offset != 0) {
    offset = name.find('/');
    if (offset == 0) {
      NS_ASSERT_MSG(
          false, "NamesPriv::Add(): Name begins with '/' but not \"/Names\"");
      return false;
    }

    name = "/Names/" + name;
  }

  std::string::size_type i = name.rfind('/');
  NS_ASSERT_MSG(i != std::string::npos,
                "NamesPriv::Add(): Internal error.  Can't find '/' in name");

  NS_ASSERT_MSG(i != 0,
                "NamesPriv::Add(): Can't find a name in the path string");

  return Add(name.substr(0, i), name.substr(i + 1), object);
}

bool NamesPriv::Add(std::string path, std::string name, Ptr<Object> object) {
  NS_LOG_FUNCTION(this << path << name << object);
  if (path == "/Names") {
    return Add(Ptr<Object>(nullptr, false), name, object);
  }
  return Add(Find(path), name, object);
}

bool NamesPriv::Add(Ptr<Object> context, std::string name, Ptr<Object> object) {
  NS_LOG_FUNCTION(this << context << name << object);

  if (IsNamed(object)) {
    NS_LOG_LOGIC("Object is already named");
    return false;
  }

  NameNode *node = nullptr;
  if (context) {
    node = IsNamed(context);
    NS_ASSERT_MSG(
        node,
        "NamesPriv::Name(): context must point to a previously named node");
  } else {
    node = &m_root;
  }

  if (IsDuplicateName(node, name)) {
    NS_LOG_LOGIC("Name is already taken");
    return false;
  }

  auto newNode = new NameNode(node, name, object);
  node->m_nameMap[name] = newNode;
  m_objectMap[object] = newNode;

  return true;
}

bool NamesPriv::Rename(std::string oldpath, std::string newname) {
  NS_LOG_FUNCTION(this << oldpath << newname);
  std::string namespaceName = "/Names";
  std::string::size_type offset = oldpath.find(namespaceName);
  if (offset != 0) {
    offset = oldpath.find('/');
    if (offset == 0) {
      NS_ASSERT_MSG(
          false, "NamesPriv::Add(): Name begins with '/' but not \"/Names\"");
      return false;
    }

    oldpath = "/Names/" + oldpath;
  }

  std::string::size_type i = oldpath.rfind('/');
  NS_ASSERT_MSG(i != std::string::npos,
                "NamesPriv::Add(): Internal error.  Can't find '/' in name");

  NS_ASSERT_MSG(i != 0,
                "NamesPriv::Add(): Can't find a name in the path string");

  return Rename(oldpath.substr(0, i), oldpath.substr(i + 1), newname);
}

bool NamesPriv::Rename(std::string path, std::string oldname,
                       std::string newname) {
  NS_LOG_FUNCTION(this << path << oldname << newname);
  if (path == "/Names") {
    return Rename(Ptr<Object>(nullptr, false), oldname, newname);
  }
  return Rename(Find(path), oldname, newname);
}

bool NamesPriv::Rename(Ptr<Object> context, std::string oldname,
                       std::string newname) {
  NS_LOG_FUNCTION(this << context << oldname << newname);

  NameNode *node = nullptr;
  if (context) {
    node = IsNamed(context);
    NS_ASSERT_MSG(
        node,
        "NamesPriv::Name(): context must point to a previously named node");
  } else {
    node = &m_root;
  }

  if (IsDuplicateName(node, newname)) {
    NS_LOG_LOGIC("New name is already taken");
    return false;
  }

  auto i = node->m_nameMap.find(oldname);
  if (i == node->m_nameMap.end()) {
    NS_LOG_LOGIC("Old name does not exist in name map");
    return false;
  } else {
    NS_LOG_LOGIC("Old name exists in name map");

    NameNode *changeNode = i->second;
    node->m_nameMap.erase(i);
    changeNode->m_name = newname;
    node->m_nameMap[newname] = changeNode;
    return true;
  }
}

std::string NamesPriv::FindName(Ptr<Object> object) {
  NS_LOG_FUNCTION(this << object);

  auto i = m_objectMap.find(object);
  if (i == m_objectMap.end()) {
    NS_LOG_LOGIC("Object does not exist in object map");
    return "";
  } else {
    NS_LOG_LOGIC("Object exists in object map");
    return i->second->m_name;
  }
}

std::string NamesPriv::FindPath(Ptr<Object> object) {
  NS_LOG_FUNCTION(this << object);

  auto i = m_objectMap.find(object);
  if (i == m_objectMap.end()) {
    NS_LOG_LOGIC("Object does not exist in object map");
    return "";
  }

  NameNode *p = i->second;
  NS_ASSERT_MSG(p, "NamesPriv::FindFullName(): Internal error: Invalid "
                   "NameNode pointer from map");

  std::string path;

  do {
    path = "/" + p->m_name + path;
    NS_LOG_LOGIC("path is " << path);
  } while ((p = p->m_parent) != nullptr);

  return path;
}

Ptr<Object> NamesPriv::Find(std::string path) {

  NS_LOG_FUNCTION(this << path);
  std::string namespaceName = "/Names/";
  std::string remaining;

  std::string::size_type offset = path.find(namespaceName);
  if (offset == 0) {
    NS_LOG_LOGIC(path << " is a fully qualified name");
    remaining = path.substr(namespaceName.size());
  } else {
    NS_LOG_LOGIC(path << " begins with a relative name");
    remaining = path;
  }

  NameNode *node = &m_root;

  for (;;) {
    NS_LOG_LOGIC("Looking for the object of name " << remaining);
    offset = remaining.find('/');
    if (offset == std::string::npos) {
      auto i = node->m_nameMap.find(remaining);
      if (i == node->m_nameMap.end()) {
        NS_LOG_LOGIC("Name does not exist in name map");
        return nullptr;
      } else {
        NS_LOG_LOGIC("Name parsed, found object");
        return i->second->m_object;
      }
    } else {
      offset = remaining.find('/');
      std::string segment = remaining.substr(0, offset);

      auto i = node->m_nameMap.find(segment);
      if (i == node->m_nameMap.end()) {
        NS_LOG_LOGIC("Name does not exist in name map");
        return nullptr;
      } else {
        node = i->second;
        remaining = remaining.substr(offset + 1);
        NS_LOG_LOGIC("Intermediate segment parsed");
        continue;
      }
    }
  }

  NS_ASSERT_MSG(node, "NamesPriv::Find(): Internal error:  this can't happen");
  return nullptr;
}

Ptr<Object> NamesPriv::Find(std::string path, std::string name) {
  NS_LOG_FUNCTION(this << path << name);

  if (path == "/Names") {
    return Find(Ptr<Object>(nullptr, false), name);
  }
  return Find(Find(path), name);
}

Ptr<Object> NamesPriv::Find(Ptr<Object> context, std::string name) {
  NS_LOG_FUNCTION(this << context << name);

  NameNode *node = nullptr;

  if (!context) {
    NS_LOG_LOGIC("Zero context implies root NameNode");
    node = &m_root;
  } else {
    node = IsNamed(context);
    if (node == nullptr) {
      NS_LOG_LOGIC("Context does not point to a previously named node");
      return nullptr;
    }
  }

  auto i = node->m_nameMap.find(name);
  if (i == node->m_nameMap.end()) {
    NS_LOG_LOGIC("Name does not exist in name map");
    return nullptr;
  } else {
    NS_LOG_LOGIC("Name exists in name map");
    return i->second->m_object;
  }
}

NameNode *NamesPriv::IsNamed(Ptr<Object> object) {
  NS_LOG_FUNCTION(this << object);

  auto i = m_objectMap.find(object);
  if (i == m_objectMap.end()) {
    NS_LOG_LOGIC("Object does not exist in object map, returning NameNode 0");
    return nullptr;
  } else {
    NS_LOG_LOGIC("Object exists in object map, returning NameNode "
                 << &i->second);
    return i->second;
  }
}

bool NamesPriv::IsDuplicateName(NameNode *node, std::string name) {
  NS_LOG_FUNCTION(this << node << name);

  auto i = node->m_nameMap.find(name);
  if (i == node->m_nameMap.end()) {
    NS_LOG_LOGIC("Name does not exist in name map");
    return false;
  } else {
    NS_LOG_LOGIC("Name exists in name map");
    return true;
  }
}

void Names::Add(std::string name, Ptr<Object> object) {
  NS_LOG_FUNCTION(name << object);
  bool result = NamesPriv::Get()->Add(name, object);
  NS_ABORT_MSG_UNLESS(result, "Names::Add(): Error adding name " << name);
}

void Names::Rename(std::string oldpath, std::string newname) {
  NS_LOG_FUNCTION(oldpath << newname);
  bool result = NamesPriv::Get()->Rename(oldpath, newname);
  NS_ABORT_MSG_UNLESS(result, "Names::Rename(): Error renaming "
                                  << oldpath << " to " << newname);
}

void Names::Add(std::string path, std::string name, Ptr<Object> object) {
  NS_LOG_FUNCTION(path << name << object);
  bool result = NamesPriv::Get()->Add(path, name, object);
  NS_ABORT_MSG_UNLESS(result,
                      "Names::Add(): Error adding " << path << " " << name);
}

void Names::Rename(std::string path, std::string oldname, std::string newname) {
  NS_LOG_FUNCTION(path << oldname << newname);
  bool result = NamesPriv::Get()->Rename(path, oldname, newname);
  NS_ABORT_MSG_UNLESS(result, "Names::Rename (): Error renaming "
                                  << path << " " << oldname << " to "
                                  << newname);
}

void Names::Add(Ptr<Object> context, std::string name, Ptr<Object> object) {
  NS_LOG_FUNCTION(context << name << object);
  bool result = NamesPriv::Get()->Add(context, name, object);
  NS_ABORT_MSG_UNLESS(result, "Names::Add(): Error adding name "
                                  << name << " under context " << &context);
}

void Names::Rename(Ptr<Object> context, std::string oldname,
                   std::string newname) {
  NS_LOG_FUNCTION(context << oldname << newname);
  bool result = NamesPriv::Get()->Rename(context, oldname, newname);
  NS_ABORT_MSG_UNLESS(result, "Names::Rename (): Error renaming "
                                  << oldname << " to " << newname
                                  << " under context " << &context);
}

std::string Names::FindName(Ptr<Object> object) {
  NS_LOG_FUNCTION(object);
  return NamesPriv::Get()->FindName(object);
}

std::string Names::FindPath(Ptr<Object> object) {
  NS_LOG_FUNCTION(object);
  return NamesPriv::Get()->FindPath(object);
}

void Names::Clear() {
  NS_LOG_FUNCTION_NOARGS();
  return NamesPriv::Get()->Clear();
}

Ptr<Object> Names::FindInternal(std::string name) {
  NS_LOG_FUNCTION(name);
  return NamesPriv::Get()->Find(name);
}

Ptr<Object> Names::FindInternal(std::string path, std::string name) {
  NS_LOG_FUNCTION(path << name);
  return NamesPriv::Get()->Find(path, name);
}

Ptr<Object> Names::FindInternal(Ptr<Object> context, std::string name) {
  NS_LOG_FUNCTION(context << name);
  return NamesPriv::Get()->Find(context, name);
}

} // namespace ns3
