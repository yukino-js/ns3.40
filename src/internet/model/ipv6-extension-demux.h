
#ifndef IPV6_EXTENSION_DEMUX_H
#define IPV6_EXTENSION_DEMUX_H

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>

namespace ns3 {

class Ipv6Extension;
class Node;

class Ipv6ExtensionDemux : public Object {
public:
  static TypeId GetTypeId();

  Ipv6ExtensionDemux();

  ~Ipv6ExtensionDemux() override;

  void SetNode(Ptr<Node> node);

  void Insert(Ptr<Ipv6Extension> extension);

  Ptr<Ipv6Extension> GetExtension(uint8_t extensionNumber);

  void Remove(Ptr<Ipv6Extension> extension);

protected:
  void DoDispose() override;

private:
  typedef std::list<Ptr<Ipv6Extension>> Ipv6ExtensionList_t;

  Ipv6ExtensionList_t m_extensions;

  Ptr<Node> m_node;
};

} // namespace ns3

#endif
