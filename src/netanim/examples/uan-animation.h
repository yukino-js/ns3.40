
#ifndef UAN_CW_EXAMPLE_H
#define UAN_CW_EXAMPLE_H

#include "ns3/network-module.h"
#include "ns3/uan-module.h"

using namespace ns3;

class NetAnimExperiment {
public:
  void Run(UanHelper &uan);
  void ReceivePacket(Ptr<Socket> socket);
  void UpdatePositions(NodeContainer &nodes) const;
  void ResetData();
  void IncrementCw(uint32_t cw);
  uint32_t m_numNodes;
  uint32_t m_dataRate;
  double m_depth;
  double m_boundary;
  uint32_t m_packetSize;
  uint32_t m_bytesTotal;
  uint32_t m_cwMin;
  uint32_t m_cwMax;
  uint32_t m_cwStep;
  uint32_t m_avgs;

  Time m_slotTime;
  Time m_simTime;

  std::vector<double> m_throughputs;

  NetAnimExperiment();
};

#endif
