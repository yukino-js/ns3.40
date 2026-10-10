
#ifndef COMPONENT_CARRIER_ENB_H
#define COMPONENT_CARRIER_ENB_H

#include "component-carrier.h"
#include "lte-enb-phy.h"

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>

namespace ns3 {

class LteEnbMac;
class FfMacScheduler;
class LteFfrAlgorithm;

class ComponentCarrierEnb : public ComponentCarrierBaseStation {
public:
  static TypeId GetTypeId();

  ComponentCarrierEnb();

  ~ComponentCarrierEnb() override;
  void DoDispose() override;

  Ptr<LteEnbPhy> GetPhy();

  Ptr<LteEnbMac> GetMac();

  Ptr<LteFfrAlgorithm> GetFfrAlgorithm();

  Ptr<FfMacScheduler> GetFfMacScheduler();

  void SetPhy(Ptr<LteEnbPhy> s);
  void SetMac(Ptr<LteEnbMac> s);

  void SetFfMacScheduler(Ptr<FfMacScheduler> s);

  void SetFfrAlgorithm(Ptr<LteFfrAlgorithm> s);

protected:
  void DoInitialize() override;

private:
  Ptr<LteEnbPhy> m_phy;
  Ptr<LteEnbMac> m_mac;
  Ptr<FfMacScheduler> m_scheduler;
  Ptr<LteFfrAlgorithm> m_ffrAlgorithm;
};

} // namespace ns3

#endif
