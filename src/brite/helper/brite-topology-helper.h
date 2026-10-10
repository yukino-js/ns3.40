

#ifndef BRITE_TOPOLOGY_HELPER_H
#define BRITE_TOPOLOGY_HELPER_H

#include "ns3/channel.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/random-variable-stream.h"

#include <string>
#include <vector>

namespace brite {
struct Topology;
};

namespace ns3 {

class PointToPointHelper;
class Ipv4AddressHelper;

class BriteTopologyHelper {
public:
  BriteTopologyHelper(std::string confFile, std::string seedFile,
                      std::string newseedFile);

  BriteTopologyHelper(std::string confFile);

  ~BriteTopologyHelper();

  void AssignStreams(int64_t streamNumber);

  void BuildBriteTopology(InternetStackHelper &stack);

  void BuildBriteTopology(InternetStackHelper &stack,
                          const uint32_t systemCount);

  uint32_t GetNLeafNodesForAs(uint32_t asNum);

  Ptr<Node> GetLeafNodeForAs(uint32_t asNum, uint32_t leafNum);

  uint32_t GetNNodesForAs(uint32_t asNum);

  Ptr<Node> GetNodeForAs(uint32_t asNum, uint32_t nodeNum);

  uint32_t GetNAs() const;

  uint32_t GetSystemNumberForAs(uint32_t asNum) const;

  void AssignIpv4Addresses(Ipv4AddressHelper &address);

  void AssignIpv6Addresses(Ipv6AddressHelper &address);

  uint32_t GetNNodesTopology() const;

  uint32_t GetNEdgesTopology() const;

private:
  static const int mbpsToBps = 1000000;

  struct BriteNodeInfo {
    int nodeId;
    double xCoordinate;
    double yCoordinate;
    int inDegree;
    int outDegree;
    int asId;
    std::string type;
  };

  struct BriteEdgeInfo {
    int edgeId;
    int srcId;
    int destId;
    double length;
    double delay;
    double bandwidth;
    int asFrom;
    int asTo;
    std::string type;
  };

  NodeContainer m_nodes;

  void BuildBriteNodeInfoList();
  void BuildBriteEdgeInfoList();
  void ConstructTopology();
  void GenerateBriteTopology();

  std::string m_confFile;

  std::string m_seedFile;

  std::string m_newSeedFile;

  uint32_t m_numAs;

  std::vector<NetDeviceContainer *> m_netDevices;

  std::vector<NodeContainer *> m_asLeafNodes;

  std::vector<NodeContainer *> m_nodesByAs;

  std::vector<int> m_systemForAs;

  brite::Topology *m_topology;

  uint32_t m_numNodes;

  uint32_t m_numEdges;

  typedef std::vector<BriteNodeInfo> BriteNodeInfoList;
  typedef std::vector<BriteEdgeInfo> BriteEdgeInfoList;

  BriteNodeInfoList m_briteNodeInfoList;
  BriteEdgeInfoList m_briteEdgeInfoList;

  PointToPointHelper m_britePointToPointHelper;

  Ptr<UniformRandomVariable> m_uv;
};

} // namespace ns3

#endif
