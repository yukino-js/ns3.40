#ifndef NODE_CONTAINER_H
#define NODE_CONTAINER_H

#include "ns3/node.h"

#include <type_traits>
#include <vector>

namespace ns3 {

class NodeContainer {
public:
  typedef std::vector<Ptr<Node>>::const_iterator Iterator;

  static NodeContainer GetGlobal();

  NodeContainer();

  NodeContainer(Ptr<Node> node);

  NodeContainer(std::string nodeName);

  explicit NodeContainer(uint32_t n, uint32_t systemId = 0);

  template <typename... Ts>
  NodeContainer(const NodeContainer &nc, Ts &&...args);

  Iterator Begin() const;

  Iterator End() const;

  uint32_t GetN() const;

  Ptr<Node> Get(uint32_t i) const;

  void Create(uint32_t n);

  void Create(uint32_t n, uint32_t systemId);

  void Add(const NodeContainer &nc);

  template <typename... Ts> void Add(const NodeContainer &nc, Ts &&...args);

  void Add(Ptr<Node> node);

  void Add(std::string nodeName);

  bool Contains(uint32_t id) const;

private:
  std::vector<Ptr<Node>> m_nodes;
};

} // namespace ns3

namespace ns3 {

template <typename... Ts>
NodeContainer::NodeContainer(const NodeContainer &nc, Ts &&...args) {
  static_assert(std::conjunction_v<std::is_convertible<Ts, NodeContainer>...>,
                "Variable types are not convertible to NodeContainer");

  Add(nc, std::forward<Ts>(args)...);
}

template <typename... Ts>
void NodeContainer::Add(const NodeContainer &nc, Ts &&...args) {
  static_assert(std::conjunction_v<std::is_convertible<Ts, NodeContainer>...>,
                "Variable types are not convertible to NodeContainer");

  Add(nc);
  Add(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
