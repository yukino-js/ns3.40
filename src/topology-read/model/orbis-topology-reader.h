
#ifndef ORBIS_TOPOLOGY_READER_H
#define ORBIS_TOPOLOGY_READER_H

#include "topology-reader.h"

namespace ns3 {

class OrbisTopologyReader : public TopologyReader {
public:
  static TypeId GetTypeId();

  OrbisTopologyReader();
  ~OrbisTopologyReader() override;

  OrbisTopologyReader(const OrbisTopologyReader &) = delete;
  OrbisTopologyReader &operator=(const OrbisTopologyReader &) = delete;

  NodeContainer Read() override;
};

}; // namespace ns3

#endif
