
#ifndef LTE_RADIO_BEARER_INFO_H
#define LTE_RADIO_BEARER_INFO_H

#include "eps-bearer.h"
#include "lte-rrc-sap.h"

#include <ns3/ipv4-address.h>
#include <ns3/object.h>
#include <ns3/pointer.h>

namespace ns3 {

class LteRlc;
class LtePdcp;

class LteRadioBearerInfo : public Object {
public:
  LteRadioBearerInfo();
  ~LteRadioBearerInfo() override;
  static TypeId GetTypeId();

  Ptr<LteRlc> m_rlc;
  Ptr<LtePdcp> m_pdcp;
};

class LteSignalingRadioBearerInfo : public LteRadioBearerInfo {
public:
  static TypeId GetTypeId();

  uint8_t m_srbIdentity;
  LteRrcSap::LogicalChannelConfig m_logicalChannelConfig;
};

class LteDataRadioBearerInfo : public LteRadioBearerInfo {
public:
  static TypeId GetTypeId();

  EpsBearer m_epsBearer;
  uint8_t m_epsBearerIdentity;
  uint8_t m_drbIdentity;
  LteRrcSap::RlcConfig m_rlcConfig;
  uint8_t m_logicalChannelIdentity;
  LteRrcSap::LogicalChannelConfig m_logicalChannelConfig;
  uint32_t m_gtpTeid;
  Ipv4Address m_transportLayerAddress;
};

} // namespace ns3

#endif
