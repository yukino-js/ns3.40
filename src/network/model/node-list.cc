
#include "node-list.h"

#include "node.h"

#include "ns3/assert.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/object-vector.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NodeList");

class NodeListPriv : public Object {
public:
  static TypeId GetTypeId();
  NodeListPriv();
  ~NodeListPriv() override;

  uint32_t Add(Ptr<Node> node);

  NodeList::Iterator Begin() const;

  NodeList::Iterator End() const;

  Ptr<Node> GetNode(uint32_t n);

  uint32_t GetNNodes();

  static Ptr<NodeListPriv> Get();

private:
  static Ptr<NodeListPriv> *DoGet();

  static void Delete();

  void DoDispose() override;

  std::vector<Ptr<Node>> m_nodes;
};

NS_OBJECT_ENSURE_REGISTERED(NodeListPriv);

TypeId NodeListPriv::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::NodeListPriv")
          .SetParent<Object>()
          .SetGroupName("Network")
          .AddAttribute("NodeList",
                        "The list of all nodes created during the simulation.",
                        ObjectVectorValue(),
                        MakeObjectVectorAccessor(&NodeListPriv::m_nodes),
                        MakeObjectVectorChecker<Node>());
  return tid;
}

Ptr<NodeListPriv> NodeListPriv::Get() {
  NS_LOG_FUNCTION_NOARGS();
  return *DoGet();
}

Ptr<NodeListPriv> *NodeListPriv::DoGet() {
  NS_LOG_FUNCTION_NOARGS();
  static Ptr<NodeListPriv> ptr = nullptr;
  if (!ptr) {
    ptr = CreateObject<NodeListPriv>();
    Config::RegisterRootNamespaceObject(ptr);
    Simulator::ScheduleDestroy(&NodeListPriv::Delete);
  }
  return &ptr;
}

void NodeListPriv::Delete() {
  NS_LOG_FUNCTION_NOARGS();
  Config::UnregisterRootNamespaceObject(Get());
  (*DoGet()) = nullptr;
}

NodeListPriv::NodeListPriv() { NS_LOG_FUNCTION(this); }

NodeListPriv::~NodeListPriv() { NS_LOG_FUNCTION(this); }

void NodeListPriv::DoDispose() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_nodes.begin(); i != m_nodes.end(); i++) {
    Ptr<Node> node = *i;
    node->Dispose();
    *i = nullptr;
  }
  m_nodes.erase(m_nodes.begin(), m_nodes.end());
  Object::DoDispose();
}

uint32_t NodeListPriv::Add(Ptr<Node> node) {
  NS_LOG_FUNCTION(this << node);
  uint32_t index = m_nodes.size();
  m_nodes.push_back(node);
  Simulator::ScheduleWithContext(index, TimeStep(0), &Node::Initialize, node);
  return index;
}

NodeList::Iterator NodeListPriv::Begin() const {
  NS_LOG_FUNCTION(this);
  return m_nodes.begin();
}

NodeList::Iterator NodeListPriv::End() const {
  NS_LOG_FUNCTION(this);
  return m_nodes.end();
}

uint32_t NodeListPriv::GetNNodes() {
  NS_LOG_FUNCTION(this);
  return m_nodes.size();
}

Ptr<Node> NodeListPriv::GetNode(uint32_t n) {
  NS_LOG_FUNCTION(this << n);
  NS_ASSERT_MSG(n < m_nodes.size(), "Node index "
                                        << n << " is out of range (only have "
                                        << m_nodes.size() << " nodes).");
  return m_nodes[n];
}

} // namespace ns3

namespace ns3 {

uint32_t NodeList::Add(Ptr<Node> node) {
  NS_LOG_FUNCTION(node);
  return NodeListPriv::Get()->Add(node);
}

NodeList::Iterator NodeList::Begin() {
  NS_LOG_FUNCTION_NOARGS();
  return NodeListPriv::Get()->Begin();
}

NodeList::Iterator NodeList::End() {
  NS_LOG_FUNCTION_NOARGS();
  return NodeListPriv::Get()->End();
}

Ptr<Node> NodeList::GetNode(uint32_t n) {
  NS_LOG_FUNCTION(n);
  return NodeListPriv::Get()->GetNode(n);
}

uint32_t NodeList::GetNNodes() {
  NS_LOG_FUNCTION_NOARGS();
  return NodeListPriv::Get()->GetNNodes();
}

} // namespace ns3
