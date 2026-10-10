
#ifndef LTE_HEX_GRID_ENB_TOPOLOGY_HELPER_H
#define LTE_HEX_GRID_ENB_TOPOLOGY_HELPER_H

#include "lte-helper.h"

namespace ns3 {

class LteHexGridEnbTopologyHelper : public Object {
public:
  LteHexGridEnbTopologyHelper();
  ~LteHexGridEnbTopologyHelper() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  void SetLteHelper(Ptr<LteHelper> h);

  NetDeviceContainer SetPositionAndInstallEnbDevice(NodeContainer c);

private:
  Ptr<LteHelper> m_lteHelper;

  double m_offset;

  double m_d;

  double m_xMin;

  double m_yMin;

  uint32_t m_gridWidth;

  uint32_t m_siteHeight;
};

} // namespace ns3

#endif
