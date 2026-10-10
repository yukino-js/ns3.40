
#ifndef WIMAX_SS_NET_DEVICE_H
#define WIMAX_SS_NET_DEVICE_H

#include "ipcs-classifier.h"
#include "ss-service-flow-manager.h"
#include "wimax-mac-header.h"
#include "wimax-net-device.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"

namespace ns3 {

class Node;
class OfdmDlBurstProfile;
class OfdmUlBurstProfile;
class SSScheduler;
class SSLinkManager;
class SsServiceFlowManager;
class IpcsClassifier;

class SubscriberStationNetDevice : public WimaxNetDevice {
public:
  enum State {
    SS_STATE_IDLE,
    SS_STATE_SCANNING,
    SS_STATE_SYNCHRONIZING,
    SS_STATE_ACQUIRING_PARAMETERS,
    SS_STATE_WAITING_REG_RANG_INTRVL,
    SS_STATE_WAITING_INV_RANG_INTRVL,
    SS_STATE_WAITING_RNG_RSP,
    SS_STATE_ADJUSTING_PARAMETERS,
    SS_STATE_REGISTERED,
    SS_STATE_TRANSMITTING,
    SS_STATE_STOPPED
  };

  enum EventType {
    EVENT_NONE,
    EVENT_WAIT_FOR_RNG_RSP,
    EVENT_DL_MAP_SYNC_TIMEOUT,
    EVENT_LOST_DL_MAP,
    EVENT_LOST_UL_MAP,
    EVENT_DCD_WAIT_TIMEOUT,
    EVENT_UCD_WAIT_TIMEOUT,
    EVENT_RANG_OPP_WAIT_TIMEOUT
  };

  static TypeId GetTypeId();
  SubscriberStationNetDevice();
  SubscriberStationNetDevice(Ptr<Node> node, Ptr<WimaxPhy> phy);
  ~SubscriberStationNetDevice() override;

  void InitSubscriberStationNetDevice();
  void SetLostDlMapInterval(Time lostDlMapInterval);
  Time GetLostDlMapInterval() const;
  void SetLostUlMapInterval(Time lostUlMapInterval);
  Time GetLostUlMapInterval() const;
  void SetMaxDcdInterval(Time maxDcdInterval);
  Time GetMaxDcdInterval() const;
  void SetMaxUcdInterval(Time maxUcdInterval);
  Time GetMaxUcdInterval() const;
  void SetIntervalT1(Time interval1);
  Time GetIntervalT1() const;
  void SetIntervalT2(Time interval2);
  Time GetIntervalT2() const;
  void SetIntervalT3(Time interval3);
  Time GetIntervalT3() const;
  void SetIntervalT7(Time interval7);
  Time GetIntervalT7() const;
  void SetIntervalT12(Time interval12);
  Time GetIntervalT12() const;
  void SetIntervalT20(Time interval20);
  Time GetIntervalT20() const;
  void SetIntervalT21(Time interval21);
  Time GetIntervalT21() const;
  void SetMaxContentionRangingRetries(uint8_t maxContentionRangingRetries);
  uint8_t GetMaxContentionRangingRetries() const;
  void SetBasicConnection(Ptr<WimaxConnection> basicConnection);
  Ptr<WimaxConnection> GetBasicConnection() const;
  void SetPrimaryConnection(Ptr<WimaxConnection> primaryConnection);
  Ptr<WimaxConnection> GetPrimaryConnection() const;
  Cid GetBasicCid() const;
  Cid GetPrimaryCid() const;

  void SetModulationType(WimaxPhy::ModulationType modulationType);
  WimaxPhy::ModulationType GetModulationType() const;
  void
  SetAreManagementConnectionsAllocated(bool areManagementConnectionsAllocated);
  bool GetAreManagementConnectionsAllocated() const;
  void SetAreServiceFlowsAllocated(bool areServiceFlowsAllocated);
  bool GetAreServiceFlowsAllocated() const;
  Ptr<SSScheduler> GetScheduler() const;
  void SetScheduler(Ptr<SSScheduler> ssScheduler);
  bool HasServiceFlows() const;
  bool Enqueue(Ptr<Packet> packet, const MacHeaderType &hdrType,
               Ptr<WimaxConnection> connection) override;
  void SendBurst(uint8_t uiuc, uint16_t nrSymbols,
                 Ptr<WimaxConnection> connection,
                 MacHeaderType::HeaderType packetType =
                     MacHeaderType::HEADER_TYPE_GENERIC);

  void Start() override;
  void Stop() override;

  void AddServiceFlow(ServiceFlow *sf) const;
  void AddServiceFlow(ServiceFlow sf) const;
  void SetTimer(EventId eventId, EventId &event);
  bool IsRegistered() const;
  Time GetTimeToAllocation(Time deferTime);

  Ptr<SSLinkManager> m_linkManager;
  Ptr<IpcsClassifier> GetIpcsClassifier() const;
  void SetIpcsPacketClassifier(Ptr<IpcsClassifier> classifier);
  Ptr<SSLinkManager> GetLinkManager() const;
  void SetLinkManager(Ptr<SSLinkManager> linkManager);
  Ptr<SsServiceFlowManager> GetServiceFlowManager() const;
  void SetServiceFlowManager(Ptr<SsServiceFlowManager> sfm);

  typedef Callback<void, std::string, Ptr<const Packet>> AsciiTraceCallback;

  void SetAsciiTxQueueEnqueueCallback(AsciiTraceCallback cb);

  void SetAsciiTxQueueDequeueCallback(AsciiTraceCallback cb);

  void SetAsciiTxQueueDropCallback(AsciiTraceCallback cb);

private:
  static Time GetDefaultLostDlMapInterval();

  void DoDispose() override;
  bool DoSend(Ptr<Packet> packet, const Mac48Address &source,
              const Mac48Address &dest, uint16_t protocolNumber) override;
  void DoReceive(Ptr<Packet> packet) override;

  void ProcessDlMap(const DlMap &dlmap);
  void ProcessUlMap(const UlMap &ulmap);
  void ProcessDcd(const Dcd &dcd);
  void ProcessUcd(const Ucd &ucd);

  Time m_lostDlMapInterval;
  Time m_lostUlMapInterval;
  Time m_maxDcdInterval;
  Time m_maxUcdInterval;
  Time m_intervalT1;
  Time m_intervalT2;
  Time m_intervalT3;
  Time m_intervalT7;
  Time m_intervalT12;
  Time m_intervalT20;
  Time m_intervalT21;
  uint8_t m_maxContentionRangingRetries;

  uint8_t m_dcdCount;
  Mac48Address m_baseStationId;

  uint8_t m_ucdCount;
  double m_allocationStartTime;

  uint16_t m_nrDlMapElements;
  uint16_t m_nrUlMapElements;

  Ptr<WimaxConnection> m_basicConnection;
  Ptr<WimaxConnection> m_primaryConnection;

  EventId m_lostDlMapEvent;
  EventId m_lostUlMapEvent;
  EventId m_dcdWaitTimeoutEvent;
  EventId m_ucdWaitTimeoutEvent;
  EventId m_rangOppWaitTimeoutEvent;

  uint32_t m_nrDlMapRecvd;
  uint32_t m_nrUlMapRecvd;
  uint32_t m_nrDcdRecvd;
  uint32_t m_nrUcdRecvd;

  OfdmDlBurstProfile *m_dlBurstProfile;
  OfdmUlBurstProfile *m_ulBurstProfile;

  WimaxPhy::ModulationType m_modulationType;

  bool m_areManagementConnectionsAllocated;
  bool m_areServiceFlowsAllocated;

  Ptr<SSScheduler> m_scheduler;
  Ptr<SsServiceFlowManager> m_serviceFlowManager;
  Ptr<IpcsClassifier> m_classifier;

  TracedCallback<Ptr<const Packet>, Mac48Address, const Cid &> m_traceSSRx;

  TracedCallback<Ptr<const Packet>> m_ssTxTrace;

  TracedCallback<Ptr<const Packet>> m_ssTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_ssPromiscRxTrace;

  TracedCallback<Ptr<const Packet>> m_ssRxTrace;

  TracedCallback<Ptr<const Packet>> m_ssRxDropTrace;

  AsciiTraceCallback m_asciiTxQueueEnqueueCb;
  AsciiTraceCallback m_asciiTxQueueDequeueCb;
  AsciiTraceCallback m_asciiTxQueueDropCb;
};

} // namespace ns3

#endif
