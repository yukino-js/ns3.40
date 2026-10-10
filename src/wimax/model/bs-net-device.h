
#ifndef WIMAX_BS_NET_DEVICE_H
#define WIMAX_BS_NET_DEVICE_H

#include "bs-service-flow-manager.h"
#include "dl-mac-messages.h"
#include "ipcs-classifier.h"
#include "wimax-connection.h"
#include "wimax-net-device.h"

#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"

namespace ns3 {

class Node;
class Packet;
class SSRecord;
class SSManager;
class BSScheduler;
class BurstProfileManager;
class BSLinkManager;
class UplinkScheduler;
class BsServiceFlowManager;

class BaseStationNetDevice : public WimaxNetDevice {
public:
  enum State {
    BS_STATE_DL_SUB_FRAME,
    BS_STATE_UL_SUB_FRAME,
    BS_STATE_TTG,
    BS_STATE_RTG
  };

  enum MacPreamble { SHORT_PREAMBLE = 1, LONG_PREAMBLE };

  static TypeId GetTypeId();
  BaseStationNetDevice();
  BaseStationNetDevice(Ptr<Node> node, Ptr<WimaxPhy> phy);
  BaseStationNetDevice(Ptr<Node> node, Ptr<WimaxPhy> phy,
                       Ptr<UplinkScheduler> uplinkScheduler,
                       Ptr<BSScheduler> bsScheduler);
  ~BaseStationNetDevice() override;
  void SetInitialRangingInterval(Time initialRangInterval);
  void InitBaseStationNetDevice();
  Time GetInitialRangingInterval() const;
  void SetDcdInterval(Time dcdInterval);
  Time GetDcdInterval() const;
  void SetUcdInterval(Time ucdInterval);
  Time GetUcdInterval() const;
  void SetIntervalT8(Time interval);
  Time GetIntervalT8() const;
  void SetMaxRangingCorrectionRetries(uint8_t maxRangCorrectionRetries);
  uint8_t GetMaxRangingCorrectionRetries() const;
  void SetMaxInvitedRangRetries(uint8_t maxInvitedRangRetries);
  uint8_t GetMaxInvitedRangRetries() const;
  void SetRangReqOppSize(uint8_t rangReqOppSize);
  uint8_t GetRangReqOppSize() const;
  void SetBwReqOppSize(uint8_t bwReqOppSize);
  uint8_t GetBwReqOppSize() const;
  void SetNrDlSymbols(uint32_t dlSymbols);
  uint32_t GetNrDlSymbols() const;
  void SetNrUlSymbols(uint32_t ulSymbols);
  uint32_t GetNrUlSymbols() const;
  uint32_t GetNrDcdSent() const;
  uint32_t GetNrUcdSent() const;
  Time GetDlSubframeStartTime() const;
  Time GetUlSubframeStartTime() const;
  uint8_t GetRangingOppNumber() const;
  Ptr<SSManager> GetSSManager() const;
  void SetSSManager(Ptr<SSManager> ssManager);
  Ptr<UplinkScheduler> GetUplinkScheduler() const;
  void SetUplinkScheduler(Ptr<UplinkScheduler> ulScheduler);
  Ptr<BSLinkManager> GetLinkManager() const;
  void SetBSScheduler(Ptr<BSScheduler> bsSchedule);
  Ptr<BSScheduler> GetBSScheduler() const;
  void SetLinkManager(Ptr<BSLinkManager> linkManager);
  Ptr<IpcsClassifier> GetBsClassifier() const;
  void SetBsClassifier(Ptr<IpcsClassifier> classifier);

  Time GetPsDuration() const;
  Time GetSymbolDuration() const;
  void Start() override;
  void Stop() override;
  bool Enqueue(Ptr<Packet> packet, const MacHeaderType &hdrType,
               Ptr<WimaxConnection> connection) override;
  Ptr<WimaxConnection> GetConnection(Cid cid);

  void MarkUplinkAllocations();
  void MarkRangingOppStart(Time rangingOppStartTime);
  Ptr<BsServiceFlowManager> GetServiceFlowManager() const;
  void SetServiceFlowManager(Ptr<BsServiceFlowManager> sfm);

private:
  void DoDispose() override;
  void StartFrame();
  void StartDlSubFrame();
  void EndDlSubFrame();
  void StartUlSubFrame();
  void EndUlSubFrame();
  void EndFrame();
  bool DoSend(Ptr<Packet> packet, const Mac48Address &source,
              const Mac48Address &dest, uint16_t protocolNumber) override;
  void DoReceive(Ptr<Packet> packet) override;
  void CreateMapMessages();
  void CreateDescriptorMessages(bool sendDcd, bool sendUcd);
  void SendBursts();

  Ptr<Packet> CreateDlMap();
  Ptr<Packet> CreateDcd();
  Ptr<Packet> CreateUlMap();
  Ptr<Packet> CreateUcd();
  void SetDlBurstProfiles(Dcd *dcd);
  void SetUlBurstProfiles(Ucd *ucd);

  void MarkUplinkAllocationStart(Time allocationStartTime);
  void MarkUplinkAllocationEnd(Time allocationEndTime, Cid cid, uint8_t uiuc);
  void UplinkAllocationStart();
  void UplinkAllocationEnd(Cid cid, uint8_t uiuc);
  void RangingOppStart();

  Time m_initialRangInterval;
  Time m_dcdInterval;
  Time m_ucdInterval;
  Time m_intervalT8;

  uint8_t m_maxRangCorrectionRetries;
  uint8_t m_maxInvitedRangRetries;
  uint8_t m_rangReqOppSize;
  uint8_t m_bwReqOppSize;

  uint32_t m_nrDlSymbols;
  uint32_t m_nrUlSymbols;

  uint32_t m_nrDlMapSent;
  uint32_t m_nrUlMapSent;
  uint32_t m_nrDcdSent;
  uint32_t m_nrUcdSent;

  uint32_t m_dcdConfigChangeCount;
  uint32_t m_ucdConfigChangeCount;

  uint32_t m_framesSinceLastDcd;
  uint32_t m_framesSinceLastUcd;

  uint32_t m_nrDlFrames;
  uint32_t m_nrUlFrames;

  uint16_t m_nrSsRegistered;

  uint16_t m_nrDlAllocations;
  uint16_t m_nrUlAllocations;

  Time m_dlSubframeStartTime;
  Time m_ulSubframeStartTime;

  uint8_t m_ulAllocationNumber;
  uint8_t m_rangingOppNumber;

  CidFactory *m_cidFactory;

  uint32_t m_allocationStartTime;

  Ptr<SSManager> m_ssManager;
  Ptr<UplinkScheduler> m_uplinkScheduler;
  Ptr<BSScheduler> m_scheduler;
  Ptr<BSLinkManager> m_linkManager;
  Ptr<IpcsClassifier> m_bsClassifier;
  Ptr<BsServiceFlowManager> m_serviceFlowManager;
  Time m_psDuration;
  Time m_symbolDuration;

  TracedCallback<Ptr<const Packet>, Mac48Address, Cid> m_traceBSRx;

  TracedCallback<Ptr<const Packet>> m_bsTxTrace;

  TracedCallback<Ptr<const Packet>> m_bsTxDropTrace;

  TracedCallback<Ptr<const Packet>> m_bsPromiscRxTrace;

  TracedCallback<Ptr<const Packet>> m_bsRxTrace;

  TracedCallback<Ptr<const Packet>> m_bsRxDropTrace;
};

} // namespace ns3

#endif
