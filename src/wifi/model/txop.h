
#ifndef TXOP_H
#define TXOP_H

#include "wifi-mac-header.h"

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-value.h"

#include <map>
#include <memory>
#include <vector>

namespace ns3 {

class Packet;
class ChannelAccessManager;
class MacTxMiddle;
class WifiMode;
class WifiMacQueue;
class WifiMpdu;
class UniformRandomVariable;
class CtrlBAckResponseHeader;
class WifiMac;
enum WifiMacDropReason : uint8_t;

class Txop : public Object {
public:
  Txop();

  Txop(Ptr<WifiMacQueue> queue);

  ~Txop() override;

  static TypeId GetTypeId();

  typedef Callback<void, WifiMacDropReason, Ptr<const WifiMpdu>> DroppedMpdu;

  enum ChannelAccessStatus { NOT_REQUESTED = 0, REQUESTED, GRANTED };

  virtual bool IsQosTxop() const;

  virtual void SetWifiMac(const Ptr<WifiMac> mac);
  void SetTxMiddle(const Ptr<MacTxMiddle> txMiddle);

  virtual void SetDroppedMpduCallback(DroppedMpdu callback);

  Ptr<WifiMacQueue> GetWifiMacQueue() const;

  void SetMinCw(uint32_t minCw);
  void SetMinCws(std::vector<uint32_t> minCws);
  void SetMinCw(uint32_t minCw, uint8_t linkId);
  void SetMaxCw(uint32_t maxCw);
  void SetMaxCws(std::vector<uint32_t> maxCws);
  void SetMaxCw(uint32_t maxCw, uint8_t linkId);
  void SetAifsn(uint8_t aifsn);
  void SetAifsns(std::vector<uint8_t> aifsns);
  void SetAifsn(uint8_t aifsn, uint8_t linkId);
  void SetTxopLimit(Time txopLimit);
  void SetTxopLimits(const std::vector<Time> &txopLimits);
  void SetTxopLimit(Time txopLimit, uint8_t linkId);
  uint32_t GetMinCw() const;
  std::vector<uint32_t> GetMinCws() const;
  virtual uint32_t GetMinCw(uint8_t linkId) const;
  uint32_t GetMaxCw() const;
  std::vector<uint32_t> GetMaxCws() const;
  virtual uint32_t GetMaxCw(uint8_t linkId) const;
  uint8_t GetAifsn() const;
  std::vector<uint8_t> GetAifsns() const;
  virtual uint8_t GetAifsn(uint8_t linkId) const;
  Time GetTxopLimit() const;
  std::vector<Time> GetTxopLimits() const;
  Time GetTxopLimit(uint8_t linkId) const;
  void ResetCw(uint8_t linkId);
  void UpdateFailedCw(uint8_t linkId);

  virtual void NotifySleep(uint8_t linkId);
  virtual void NotifyOff();
  virtual void NotifyWakeUp(uint8_t linkId);
  virtual void NotifyOn();

  virtual void Queue(Ptr<Packet> packet, const WifiMacHeader &hdr);
  virtual void Queue(Ptr<WifiMpdu> mpdu);

  virtual void NotifyChannelAccessed(uint8_t linkId,
                                     Time txopDuration = Seconds(0));
  virtual void NotifyChannelReleased(uint8_t linkId);

  int64_t AssignStreams(int64_t stream);

  virtual ChannelAccessStatus GetAccessStatus(uint8_t linkId) const;

  void StartBackoffNow(uint32_t nSlots, uint8_t linkId);

  virtual bool HasFramesToTransmit(uint8_t linkId);

  void SwapLinks(std::map<uint8_t, uint8_t> links);

protected:
  friend class ChannelAccessManager;

  void DoDispose() override;
  void DoInitialize() override;

  virtual void NotifyAccessRequested(uint8_t linkId);

  virtual void GenerateBackoff(uint8_t linkId);
  virtual void StartAccessIfNeeded(uint8_t linkId);
  void RequestAccess(uint8_t linkId);

  uint32_t GetCw(uint8_t linkId) const;
  uint32_t GetBackoffSlots(uint8_t linkId) const;
  Time GetBackoffStart(uint8_t linkId) const;
  void UpdateBackoffSlotsNow(uint32_t nSlots, Time backoffUpdateBound,
                             uint8_t linkId);

  struct LinkEntity {
    virtual ~LinkEntity() = default;

    uint32_t backoffSlots{0};
    Time backoffStart{0};
    uint32_t cw{0};
    uint32_t cwMin{0};
    uint32_t cwMax{0};
    uint8_t aifsn{0};
    Time txopLimit{0};
    ChannelAccessStatus access{NOT_REQUESTED};
  };

  LinkEntity &GetLink(uint8_t linkId) const;

  const std::map<uint8_t, std::unique_ptr<LinkEntity>> &GetLinks() const;

  DroppedMpdu m_droppedMpduCallback;
  Ptr<WifiMacQueue> m_queue;
  Ptr<MacTxMiddle> m_txMiddle;
  Ptr<WifiMac> m_mac;
  Ptr<UniformRandomVariable> m_rng;

  typedef TracedCallback<uint32_t, uint8_t> BackoffValueTracedCallback;
  typedef TracedCallback<uint32_t, uint8_t> CwValueTracedCallback;

  BackoffValueTracedCallback m_backoffTrace;
  CwValueTracedCallback m_cwTrace;

private:
  virtual std::unique_ptr<LinkEntity> CreateLinkEntity() const;

  std::map<uint8_t, std::unique_ptr<LinkEntity>> m_links;
};

} // namespace ns3

#endif
