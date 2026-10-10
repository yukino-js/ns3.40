
#ifndef SS_SERVICE_FLOW_MANAGER_H
#define SS_SERVICE_FLOW_MANAGER_H

#include "mac-messages.h"
#include "service-flow-manager.h"
#include "ss-net-device.h"

#include "ns3/buffer.h"
#include "ns3/event-id.h"

#include <stdint.h>

namespace ns3 {

class Packet;
class ServiceFlow;
class WimaxNetDevice;
class WimaxConnection;
class SubscriberStationNetDevice;

class SsServiceFlowManager : public ServiceFlowManager {
public:
  enum ConfirmationCode { CONFIRMATION_CODE_SUCCESS, CONFIRMATION_CODE_REJECT };

  SsServiceFlowManager(Ptr<SubscriberStationNetDevice> device);
  ~SsServiceFlowManager() override;
  void DoDispose() override;

  static TypeId GetTypeId();

  void AddServiceFlow(ServiceFlow *serviceFlow);
  void AddServiceFlow(ServiceFlow serviceFlow);
  void SetMaxDsaReqRetries(uint8_t maxDsaReqRetries);
  uint8_t GetMaxDsaReqRetries() const;

  EventId GetDsaRspTimeoutEvent() const;
  EventId GetDsaAckTimeoutEvent() const;

  void InitiateServiceFlows();

  DsaReq CreateDsaReq(const ServiceFlow *serviceFlow);

  Ptr<Packet> CreateDsaAck();

  void ScheduleDsaReq(const ServiceFlow *serviceFlow);

  void ProcessDsaRsp(const DsaRsp &dsaRsp);

private:
  Ptr<SubscriberStationNetDevice> m_device;

  uint8_t m_maxDsaReqRetries;

  EventId m_dsaRspTimeoutEvent;
  EventId m_dsaAckTimeoutEvent;

  DsaReq m_dsaReq;
  DsaAck m_dsaAck;

  uint16_t m_currentTransactionId;
  uint16_t m_transactionIdIndex;
  uint8_t m_dsaReqRetries;

  ServiceFlow *m_pendingServiceFlow;
};

} // namespace ns3

#endif
