
#ifndef BS_SERVICE_FLOW_MANAGER_H
#define BS_SERVICE_FLOW_MANAGER_H

#include "bs-net-device.h"
#include "mac-messages.h"
#include "service-flow-manager.h"

#include "ns3/buffer.h"
#include "ns3/event-id.h"

#include <stdint.h>

namespace ns3 {

class Packet;
class ServiceFlow;
class WimaxNetDevice;
class SSRecord;
class WimaxConnection;
class BaseStationNetDevice;

class BsServiceFlowManager : public ServiceFlowManager {
public:
  enum ConfirmationCode { CONFIRMATION_CODE_SUCCESS, CONFIRMATION_CODE_REJECT };

  BsServiceFlowManager(Ptr<BaseStationNetDevice> device);
  ~BsServiceFlowManager() override;
  void DoDispose() override;
  static TypeId GetTypeId();

  void AddServiceFlow(ServiceFlow *serviceFlow);
  ServiceFlow *GetServiceFlow(uint32_t sfid) const;
  ServiceFlow *GetServiceFlow(Cid cid) const;
  std::vector<ServiceFlow *>
  GetServiceFlows(ServiceFlow::SchedulingType schedulingType) const;
  void SetMaxDsaRspRetries(uint8_t maxDsaRspRetries);

  EventId GetDsaAckTimeoutEvent() const;
  void AllocateServiceFlows(const DsaReq &dsaReq, Cid cid);
  void AddMulticastServiceFlow(ServiceFlow sf,
                               WimaxPhy::ModulationType modulation);
  void ProcessDsaAck(const DsaAck &dsaAck, Cid cid);

  ServiceFlow *ProcessDsaReq(const DsaReq &dsaReq, Cid cid);

private:
  DsaRsp CreateDsaRsp(const ServiceFlow *serviceFlow, uint16_t transactionId);
  uint8_t GetMaxDsaRspRetries() const;
  void ScheduleDsaRsp(ServiceFlow *serviceFlow, Cid cid);
  Ptr<WimaxNetDevice> m_device;
  uint32_t m_sfidIndex;
  uint8_t m_maxDsaRspRetries;
  EventId m_dsaAckTimeoutEvent;
  Cid m_inuseScheduleDsaRspCid;
};

} // namespace ns3

#endif
