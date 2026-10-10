
#ifndef ROCKETFUEL_TOPOLOGY_READER_H
#define ROCKETFUEL_TOPOLOGY_READER_H

#include "topology-reader.h"

namespace ns3 {

class RocketfuelTopologyReader : public TopologyReader {
public:
  static TypeId GetTypeId();

  RocketfuelTopologyReader();
  ~RocketfuelTopologyReader() override;

  RocketfuelTopologyReader(const RocketfuelTopologyReader &) = delete;
  RocketfuelTopologyReader &
  operator=(const RocketfuelTopologyReader &) = delete;

  NodeContainer Read() override;

private:
  NodeContainer GenerateFromMapsFile(const std::vector<std::string> &argv);

  NodeContainer GenerateFromWeightsFile(const std::vector<std::string> &argv);

  enum RF_FileType { RF_MAPS, RF_WEIGHTS, RF_UNKNOWN };

  RF_FileType GetFileType(const std::string &buf);

  int m_linksNumber;
  int m_nodesNumber;
  std::map<std::string, Ptr<Node>> m_nodeMap;
};

}; // namespace ns3

#endif
