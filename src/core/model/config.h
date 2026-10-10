#ifndef CONFIG_H
#define CONFIG_H

#include "ptr.h"

#include <string>
#include <vector>

namespace ns3 {

class AttributeValue;
class Object;
class CallbackBase;

namespace Config {

void Reset();

void Set(std::string path, const AttributeValue &value);
bool SetFailSafe(std::string path, const AttributeValue &value);
void SetDefault(std::string name, const AttributeValue &value);
bool SetDefaultFailSafe(std::string name, const AttributeValue &value);
void SetGlobal(std::string name, const AttributeValue &value);
bool SetGlobalFailSafe(std::string name, const AttributeValue &value);
void ConnectWithoutContext(std::string path, const CallbackBase &cb);
bool ConnectWithoutContextFailSafe(std::string path, const CallbackBase &cb);
void DisconnectWithoutContext(std::string path, const CallbackBase &cb);
void Connect(std::string path, const CallbackBase &cb);
bool ConnectFailSafe(std::string path, const CallbackBase &cb);
void Disconnect(std::string path, const CallbackBase &cb);

class MatchContainer {
public:
  typedef std::vector<Ptr<Object>>::const_iterator Iterator;
  MatchContainer();
  MatchContainer(const std::vector<Ptr<Object>> &objects,
                 const std::vector<std::string> &contexts, std::string path);

  MatchContainer::Iterator Begin() const;

  MatchContainer::Iterator begin() const { return Begin(); }

  MatchContainer::Iterator End() const;

  MatchContainer::Iterator end() const { return End(); }

  std::size_t GetN() const;

  std::size_t size() const { return GetN(); }

  Ptr<Object> Get(std::size_t i) const;
  std::string GetMatchedPath(uint32_t i) const;
  std::string GetPath() const;

  void Set(std::string name, const AttributeValue &value);
  bool SetFailSafe(std::string name, const AttributeValue &value);
  void Connect(std::string name, const CallbackBase &cb);
  bool ConnectFailSafe(std::string name, const CallbackBase &cb);
  void ConnectWithoutContext(std::string name, const CallbackBase &cb);
  bool ConnectWithoutContextFailSafe(std::string name, const CallbackBase &cb);
  void Disconnect(std::string name, const CallbackBase &cb);
  void DisconnectWithoutContext(std::string name, const CallbackBase &cb);

private:
  std::vector<Ptr<Object>> m_objects;
  std::vector<std::string> m_contexts;
  std::string m_path;
};

MatchContainer LookupMatches(std::string path);

void RegisterRootNamespaceObject(Ptr<Object> obj);
void UnregisterRootNamespaceObject(Ptr<Object> obj);

std::size_t GetRootNamespaceObjectN();

Ptr<Object> GetRootNamespaceObject(uint32_t i);

} // namespace Config

} // namespace ns3

#endif
