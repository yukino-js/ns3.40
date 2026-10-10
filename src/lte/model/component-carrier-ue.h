
#ifndef COMPONENT_CARRIER_UE_H
#define COMPONENT_CARRIER_UE_H

#include "component-carrier.h"
#include "lte-phy.h"
#include "lte-ue-phy.h"

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>

namespace ns3 {

class LteUeMac;

class ComponentCarrierUe : public ComponentCarrier {
public:
  static TypeId GetTypeId();

  ComponentCarrierUe();

  ~ComponentCarrierUe() override;
  void DoDispose() override;

  Ptr<LteUePhy> GetPhy() const;

  Ptr<LteUeMac> GetMac() const;

  void SetPhy(Ptr<LteUePhy> s);

  void SetMac(Ptr<LteUeMac> s);

protected:
  void DoInitialize() override;

private:
  Ptr<LteUePhy> m_phy;
  Ptr<LteUeMac> m_mac;
};

} // namespace ns3

#endif
