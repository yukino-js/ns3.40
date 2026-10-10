#ifndef NODE_LIST_H
#define NODE_LIST_H

#include "ns3/ptr.h"

#include <vector>

namespace ns3 {

class Node;
class CallbackBase;

class NodeList {
public:
  typedef std::vector<Ptr<Node>>::const_iterator Iterator;

  static uint32_t Add(Ptr<Node> node);
  static Iterator Begin();
  static Iterator End();
  static Ptr<Node> GetNode(uint32_t n);
  static uint32_t GetNNodes();
};

} // namespace ns3

#endif
