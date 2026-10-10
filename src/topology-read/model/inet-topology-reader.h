
#ifndef INET_TOPOLOGY_READER_H
#define INET_TOPOLOGY_READER_H

#include "topology-reader.h"

namespace ns3 {

class InetTopologyReader : public TopologyReader {
public:
  static TypeId GetTypeId();

  InetTopologyReader();
  ~InetTopologyReader() override;

  InetTopologyReader(const InetTopologyReader &) = delete;
  InetTopologyReader &operator=(const InetTopologyReader &) = delete;

  NodeContainer Read() override;
};

}; // namespace ns3

#endif
