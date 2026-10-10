
#ifndef SERVICE_FLOW_MANAGER_H
#define SERVICE_FLOW_MANAGER_H

#include "mac-messages.h"

#include "ns3/buffer.h"
#include "ns3/event-id.h"

#include <stdint.h>

namespace ns3 {

class Packet;
class ServiceFlow;
class WimaxNetDevice;
class SSRecord;
class WimaxConnection;

class ServiceFlowManager : public Object {
public:
  enum ConfirmationCode { CONFIRMATION_CODE_SUCCESS, CONFIRMATION_CODE_REJECT };

  static TypeId GetTypeId();

  ServiceFlowManager();
  ~ServiceFlowManager() override;
  void DoDispose() override;

  void AddServiceFlow(ServiceFlow *serviceFlow);
  ServiceFlow *GetServiceFlow(uint32_t sfid) const;
  ServiceFlow *GetServiceFlow(Cid cid) const;
  std::vector<ServiceFlow *>
  GetServiceFlows(ServiceFlow::SchedulingType schedulingType) const;

  bool AreServiceFlowsAllocated();
  bool AreServiceFlowsAllocated(std::vector<ServiceFlow *> *serviceFlows);
  bool AreServiceFlowsAllocated(std::vector<ServiceFlow *> serviceFlows);
  ServiceFlow *GetNextServiceFlowToAllocate();

  uint32_t GetNrServiceFlows() const;

  ServiceFlow *DoClassify(Ipv4Address SrcAddress, Ipv4Address DstAddress,
                          uint16_t SrcPort, uint16_t DstPort, uint8_t Proto,
                          ServiceFlow::Direction dir) const;

private:
  std::vector<ServiceFlow *> *m_serviceFlows;
};

} // namespace ns3

#endif
