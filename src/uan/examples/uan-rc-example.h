
#ifndef UAN_RC_EXAMPLE_H
#define UAN_RC_EXAMPLE_H

#include "ns3/network-module.h"
#include "ns3/stats-module.h"
#include "ns3/uan-module.h"

using namespace ns3;

class Experiment {
public:
  uint32_t m_simMin;
  uint32_t m_simMax;
  uint32_t m_simStep;
  uint32_t m_numRates;
  uint32_t m_totalRate;
  uint32_t m_maxRange;
  uint32_t m_numNodes;
  uint32_t m_pktSize;
  bool m_doNode;
  Time m_sifs;
  Time m_simTime;

  std::string m_gnuplotfile;

  uint32_t m_bytesTotal;

  UanModesList m_dataModes;
  UanModesList m_controlModes;

  void ReceivePacket(Ptr<Socket> socket);
  UanTxMode CreateMode(uint32_t kass, uint32_t fc, bool upperblock,
                       std::string name) const;
  void CreateDualModes(uint32_t fc);
  uint32_t Run(uint32_t param);

  Experiment();
};

#endif
