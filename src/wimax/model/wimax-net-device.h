
#ifndef WIMAX_NET_DEVICE_H
#define WIMAX_NET_DEVICE_H

#include "cid-factory.h"
#include "cid.h"
#include "dl-mac-messages.h"
#include "mac-messages.h"
#include "ul-mac-messages.h"
#include "wimax-connection.h"
#include "wimax-mac-header.h"
#include "wimax-phy.h"

#include "ns3/event-id.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Node;
class Packet;
class TraceContext;
class TraceResolver;
class Channel;
class WimaxChannel;
class PacketBurst;
class BurstProfileManager;
class ConnectionManager;
class ServiceFlowManager;
class BandwidthManager;
class UplinkScheduler;

class WimaxNetDevice : public NetDevice {
public:
  enum Direction { DIRECTION_DOWNLINK, DIRECTION_UPLINK };

  enum RangingStatus {
    RANGING_STATUS_EXPIRED,
    RANGING_STATUS_CONTINUE,
    RANGING_STATUS_ABORT,
    RANGING_STATUS_SUCCESS
  };

  static TypeId GetTypeId();
  WimaxNetDevice();
  ~WimaxNetDevice() override;
  void SetTtg(uint16_t ttg);
  uint16_t GetTtg() const;
  void SetRtg(uint16_t rtg);
  uint16_t GetRtg() const;
  void Attach(Ptr<WimaxChannel> channel);
  void SetPhy(Ptr<WimaxPhy> phy);
  Ptr<WimaxPhy> GetPhy() const;

  void SetChannel(Ptr<WimaxChannel> wimaxChannel);

  uint64_t GetChannel(uint8_t index) const;

  void SetNrFrames(uint32_t nrFrames);
  uint32_t GetNrFrames() const;
  void SetMacAddress(Mac48Address address);
  Mac48Address GetMacAddress() const;
  void SetState(uint8_t state);
  uint8_t GetState() const;
  Ptr<WimaxConnection> GetInitialRangingConnection() const;
  Ptr<WimaxConnection> GetBroadcastConnection() const;

  void SetCurrentDcd(Dcd dcd);
  Dcd GetCurrentDcd() const;
  void SetCurrentUcd(Ucd ucd);
  Ucd GetCurrentUcd() const;
  Ptr<ConnectionManager> GetConnectionManager() const;

  virtual void SetConnectionManager(Ptr<ConnectionManager> connectionManager);

  Ptr<BurstProfileManager> GetBurstProfileManager() const;

  void SetBurstProfileManager(Ptr<BurstProfileManager> burstProfileManager);

  Ptr<BandwidthManager> GetBandwidthManager() const;

  void SetBandwidthManager(Ptr<BandwidthManager> bandwidthManager);

  void CreateDefaultConnections();

  virtual void Start() = 0;
  virtual void Stop() = 0;

  void SetReceiveCallback();

  void ForwardUp(Ptr<Packet> packet, const Mac48Address &source,
                 const Mac48Address &dest);

  virtual bool Enqueue(Ptr<Packet> packet, const MacHeaderType &hdrType,
                       Ptr<WimaxConnection> connection) = 0;
  void ForwardDown(Ptr<PacketBurst> burst,
                   WimaxPhy::ModulationType modulationType);

  static uint8_t m_direction;

  static Time m_frameStartTime;

  virtual void SetName(const std::string name);
  virtual std::string GetName() const;
  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  virtual Ptr<Channel> GetPhyChannel() const;
  Ptr<Channel> GetChannel() const override;
  void SetAddress(Address address) override;
  Address GetAddress() const override;
  bool SetMtu(const uint16_t mtu) override;
  uint16_t GetMtu() const override;
  bool IsLinkUp() const override;
  virtual void SetLinkChangeCallback(Callback<void> callback);
  bool IsBroadcast() const override;
  Address GetBroadcast() const override;
  bool IsMulticast() const override;
  virtual Address GetMulticast() const;
  virtual Address MakeMulticastAddress(Ipv4Address multicastGroup) const;
  bool IsPointToPoint() const override;
  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;
  void SetNode(Ptr<Node> node) override;
  Ptr<Node> GetNode() const override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
  void AddLinkChangeCallback(Callback<void> callback) override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
  NetDevice::PromiscReceiveCallback GetPromiscReceiveCallback();
  bool SupportsSendFrom() const override;

  typedef void (*TxRxTracedCallback)(Ptr<const Packet> packet,
                                     const Mac48Address &mac);
  TracedCallback<Ptr<const Packet>, const Mac48Address &> m_traceRx;
  TracedCallback<Ptr<const Packet>, const Mac48Address &> m_traceTx;

  void DoDispose() override;
  Address GetMulticast(Ipv6Address addr) const override;
  Address GetMulticast(Ipv4Address multicastGroup) const override;
  bool IsBridge() const override;

  bool IsPromisc();
  void NotifyPromiscTrace(Ptr<Packet> p);

private:
  WimaxNetDevice(const WimaxNetDevice &);
  WimaxNetDevice &operator=(const WimaxNetDevice &);

  static const uint16_t MAX_MSDU_SIZE = 1500;
  static const uint16_t DEFAULT_MSDU_SIZE = 1400;

  virtual bool DoSend(Ptr<Packet> packet, const Mac48Address &source,
                      const Mac48Address &dest, uint16_t protocolNumber) = 0;
  virtual void DoReceive(Ptr<Packet> packet) = 0;
  virtual Ptr<WimaxChannel> DoGetChannel() const;
  void Receive(Ptr<const PacketBurst> burst);
  void InitializeChannels();

  Ptr<Node> m_node;
  Ptr<WimaxPhy> m_phy;
  NetDevice::ReceiveCallback m_forwardUp;
  NetDevice::PromiscReceiveCallback m_promiscRx;

  uint32_t m_ifIndex;
  std::string m_name;
  bool m_linkUp;
  Callback<void> m_linkChange;
  mutable uint16_t m_mtu;

  static uint32_t m_nrFrames;

  std::vector<uint64_t> m_dlChannels;

  Mac48Address m_address;
  uint8_t m_state;
  uint32_t m_symbolIndex;

  uint16_t m_ttg;
  uint16_t m_rtg;

  Dcd m_currentDcd;
  Ucd m_currentUcd;

  Ptr<WimaxConnection> m_initialRangingConnection;
  Ptr<WimaxConnection> m_broadcastConnection;

  Ptr<ConnectionManager> m_connectionManager;
  Ptr<BurstProfileManager> m_burstProfileManager;
  Ptr<BandwidthManager> m_bandwidthManager;
};

} // namespace ns3

#endif
