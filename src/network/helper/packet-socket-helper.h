
#ifndef PACKET_SOCKET_HELPER_H
#define PACKET_SOCKET_HELPER_H

#include "node-container.h"

namespace ns3 {

class PacketSocketHelper {
public:
  void Install(Ptr<Node> node) const;

  void Install(std::string nodeName) const;

  void Install(NodeContainer c) const;
};

} // namespace ns3

#endif
