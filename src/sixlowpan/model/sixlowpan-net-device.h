
#ifndef SIXLOWPAN_NET_DEVICE_H
#define SIXLOWPAN_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/traced-callback.h"

#include <map>
#include <stdint.h>
#include <string>
#include <tuple>

namespace ns3 {

class Node;
class UniformRandomVariable;
class EventId;

class SixLowPanNetDevice : public NetDevice {
public:
  enum DropReason {
    DROP_FRAGMENT_TIMEOUT = 1,
    DROP_FRAGMENT_BUFFER_FULL,
    DROP_UNKNOWN_EXTENSION,
    DROP_DISALLOWED_COMPRESSION,
    DROP_SATETFUL_DECOMPRESSION_PROBLEM,
  };

  static TypeId GetTypeId();

  SixLowPanNetDevice();

  SixLowPanNetDevice(const SixLowPanNetDevice &) = delete;
  SixLowPanNetDevice &operator=(const SixLowPanNetDevice &) = delete;

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  void SetAddress(Address address) override;
  Address GetAddress() const override;
  bool SetMtu(const uint16_t mtu) override;

  uint16_t GetMtu() const override;
  bool IsLinkUp() const override;
  void AddLinkChangeCallback(Callback<void> callback) override;
  bool IsBroadcast() const override;
  Address GetBroadcast() const override;
  bool IsMulticast() const override;
  Address GetMulticast(Ipv4Address multicastGroup) const override;
  bool IsPointToPoint() const override;
  bool IsBridge() const override;
  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  Ptr<Node> GetNode() const override;
  void SetNode(Ptr<Node> node) override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
  void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;
  Address GetMulticast(Ipv6Address addr) const override;

  Ptr<NetDevice> GetNetDevice() const;

  void SetNetDevice(Ptr<NetDevice> device);

  int64_t AssignStreams(int64_t stream);

  typedef void (*RxTxTracedCallback)(Ptr<const Packet> packet,
                                     Ptr<SixLowPanNetDevice> sixNetDevice,
                                     uint32_t ifindex);

  typedef void (*DropTracedCallback)(DropReason reason,
                                     Ptr<const Packet> packet,
                                     Ptr<SixLowPanNetDevice> sixNetDevice,
                                     uint32_t ifindex);

  void AddContext(uint8_t contextId, Ipv6Prefix contextPrefix,
                  bool compressionAllowed, Time validLifetime);

  bool GetContext(uint8_t contextId, Ipv6Prefix &contextPrefix,
                  bool &compressionAllowed, Time &validLifetime);

  void RenewContext(uint8_t contextId, Time validLifetime);

  void InvalidateContext(uint8_t contextId);

  void RemoveContext(uint8_t contextId);

protected:
  void DoDispose() override;

private:
  void ReceiveFromDevice(Ptr<NetDevice> device, Ptr<const Packet> packet,
                         uint16_t protocol, const Address &source,
                         const Address &destination, PacketType packetType);

  bool DoSend(Ptr<Packet> packet, const Address &source, const Address &dest,
              uint16_t protocolNumber, bool doSendFrom);

  NetDevice::ReceiveCallback m_rxCallback;

  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  TracedCallback<Ptr<const Packet>, Ptr<SixLowPanNetDevice>, uint32_t>
      m_txTrace;

  TracedCallback<Ptr<const Packet>, Ptr<SixLowPanNetDevice>, uint32_t>
      m_rxTrace;

  TracedCallback<DropReason, Ptr<const Packet>, Ptr<SixLowPanNetDevice>,
                 uint32_t>
      m_dropTrace;

  uint32_t CompressLowPanHc1(Ptr<Packet> packet, const Address &src,
                             const Address &dst);

  void DecompressLowPanHc1(Ptr<Packet> packet, const Address &src,
                           const Address &dst);

  uint32_t CompressLowPanIphc(Ptr<Packet> packet, const Address &src,
                              const Address &dst);

  bool CanCompressLowPanNhc(uint8_t headerType);

  bool DecompressLowPanIphc(Ptr<Packet> packet, const Address &src,
                            const Address &dst);

  uint32_t CompressLowPanNhc(Ptr<Packet> packet, uint8_t headerType,
                             const Address &src, const Address &dst);

  std::pair<uint8_t, bool> DecompressLowPanNhc(Ptr<Packet> packet,
                                               const Address &src,
                                               const Address &dst,
                                               Ipv6Address srcAddress,
                                               Ipv6Address dstAddress);

  uint32_t CompressLowPanUdpNhc(Ptr<Packet> packet, bool omitChecksum);

  void DecompressLowPanUdpNhc(Ptr<Packet> packet, Ipv6Address saddr,
                              Ipv6Address daddr);

  typedef std::pair<std::pair<Address, Address>, std::pair<uint16_t, uint16_t>>
      FragmentKey_t;

  typedef std::list<std::tuple<Time, FragmentKey_t, uint32_t>>
      FragmentsTimeoutsList_t;
  typedef std::list<std::tuple<Time, FragmentKey_t, uint32_t>>::iterator
      FragmentsTimeoutsListI_t;

  FragmentsTimeoutsListI_t SetTimeout(FragmentKey_t key, uint32_t iif);

  void HandleTimeout();

  FragmentsTimeoutsList_t m_timeoutEventList;

  EventId m_timeoutEvent;

  class Fragments : public SimpleRefCount<Fragments> {
  public:
    Fragments();

    ~Fragments();

    void AddFragment(Ptr<Packet> fragment, uint16_t fragmentOffset);

    void AddFirstFragment(Ptr<Packet> fragment);

    bool IsEntire() const;

    Ptr<Packet> GetPacket() const;

    void SetPacketSize(uint32_t packetSize);

    std::list<Ptr<Packet>> GetFragments() const;

    void SetTimeoutIter(FragmentsTimeoutsListI_t iter);

    FragmentsTimeoutsListI_t GetTimeoutIter();

  private:
    uint32_t m_packetSize;

    std::list<std::pair<Ptr<Packet>, uint16_t>> m_fragments;

    Ptr<Packet> m_firstFragment;

    FragmentsTimeoutsListI_t m_timeoutIter;
  };

  void DoFragmentation(Ptr<Packet> packet, uint32_t origPacketSize,
                       uint32_t origHdrSize, uint32_t extraHdrSize,
                       std::list<Ptr<Packet>> &listFragments);

  bool ProcessFragment(Ptr<Packet> &packet, const Address &src,
                       const Address &dst, bool isFirst);

  void HandleFragmentsTimeout(FragmentKey_t key, uint32_t iif);

  void DropOldestFragmentSet();

  Address Get16MacFrom48Mac(Address addr);

  typedef std::map<FragmentKey_t, Ptr<Fragments>> MapFragments_t;
  typedef std::map<FragmentKey_t, Ptr<Fragments>>::iterator MapFragmentsI_t;

  MapFragments_t m_fragments;
  Time m_fragmentExpirationTimeout;

  uint16_t m_fragmentReassemblyListSize;

  bool m_useIphc;

  bool m_meshUnder;
  uint8_t m_bc0Serial;
  uint8_t m_meshUnderHopsLeft;
  uint16_t m_meshCacheLength;
  Ptr<RandomVariableStream> m_meshUnderJitter;
  std::map<Address, std::list<uint8_t>> m_seenPkts;

  Ptr<Node> m_node;
  Ptr<NetDevice> m_netDevice;
  uint32_t m_ifIndex;

  bool m_forceEtherType;

  uint16_t m_etherType;
  bool m_omitUdpChecksum;

  uint32_t m_compressionThreshold;

  Ptr<UniformRandomVariable> m_rng;

  struct ContextEntry {
    Ipv6Prefix contextPrefix;
    bool compressionAllowed;
    Time validLifetime;
  };

  std::map<uint8_t, ContextEntry> m_contextTable;

  bool FindUnicastCompressionContext(Ipv6Address address, uint8_t &contextId);

  bool FindMulticastCompressionContext(Ipv6Address address, uint8_t &contextId);

  Ipv6Address CleanPrefix(Ipv6Address address, Ipv6Prefix prefix);
};

} // namespace ns3

#endif
