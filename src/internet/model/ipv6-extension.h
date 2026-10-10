
#ifndef IPV6_EXTENSION_H
#define IPV6_EXTENSION_H

#include "ipv6-extension-header.h"
#include "ipv6-header.h"
#include "ipv6-interface.h"
#include "ipv6-l3-protocol.h"

#include "ns3/buffer.h"
#include "ns3/ipv6-address.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-callback.h"

#include <list>
#include <map>
#include <tuple>

namespace ns3 {

class Ipv6Extension : public Object {
public:
  static TypeId GetTypeId();

  Ipv6Extension();

  ~Ipv6Extension() override;

  void SetNode(Ptr<Node> node);

  Ptr<Node> GetNode() const;

  virtual uint8_t GetExtensionNumber() const = 0;

  virtual uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                          const Ipv6Header &ipv6Header, Ipv6Address dst,
                          uint8_t *nextHeader, bool &stopProcessing,
                          bool &isDropped,
                          Ipv6L3Protocol::DropReason &dropReason) = 0;

  virtual uint8_t ProcessOptions(Ptr<Packet> &packet, uint8_t offset,
                                 uint8_t length, const Ipv6Header &ipv6Header,
                                 Ipv6Address dst, uint8_t *nextHeader,
                                 bool &stopProcessing, bool &isDropped,
                                 Ipv6L3Protocol::DropReason &dropReason);

  int64_t AssignStreams(int64_t stream);

protected:
  Ptr<UniformRandomVariable> m_uvar;

private:
  Ptr<Node> m_node;
};

class Ipv6ExtensionHopByHop : public Ipv6Extension {
public:
  static const uint8_t EXT_NUMBER = 0;

  static TypeId GetTypeId();

  Ipv6ExtensionHopByHop();

  ~Ipv6ExtensionHopByHop() override;

  uint8_t GetExtensionNumber() const override;

  uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, Ipv6Address dst,
                  uint8_t *nextHeader, bool &stopProcessing, bool &isDropped,
                  Ipv6L3Protocol::DropReason &dropReason) override;
};

class Ipv6ExtensionDestination : public Ipv6Extension {
public:
  static const uint8_t EXT_NUMBER = 60;

  static TypeId GetTypeId();

  Ipv6ExtensionDestination();

  ~Ipv6ExtensionDestination() override;

  uint8_t GetExtensionNumber() const override;

  uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, Ipv6Address dst,
                  uint8_t *nextHeader, bool &stopProcessing, bool &isDropped,
                  Ipv6L3Protocol::DropReason &dropReason) override;
};

class Ipv6ExtensionFragment : public Ipv6Extension {
public:
  static const uint8_t EXT_NUMBER = 44;

  static TypeId GetTypeId();

  Ipv6ExtensionFragment();

  ~Ipv6ExtensionFragment() override;

  uint8_t GetExtensionNumber() const override;

  uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, Ipv6Address dst,
                  uint8_t *nextHeader, bool &stopProcessing, bool &isDropped,
                  Ipv6L3Protocol::DropReason &dropReason) override;

  typedef std::pair<Ptr<Packet>, Ipv6Header> Ipv6PayloadHeaderPair;

  void GetFragments(Ptr<Packet> packet, Ipv6Header ipv6Header,
                    uint32_t fragmentSize,
                    std::list<Ipv6PayloadHeaderPair> &listFragments);

protected:
  void DoDispose() override;

private:
  typedef std::pair<Ipv6Address, uint32_t> FragmentKey_t;

  typedef std::list<std::tuple<Time, FragmentKey_t, Ipv6Header>>
      FragmentsTimeoutsList_t;
  typedef std::list<std::tuple<Time, FragmentKey_t, Ipv6Header>>::iterator
      FragmentsTimeoutsListI_t;

  class Fragments : public SimpleRefCount<Fragments> {
  public:
    Fragments();

    ~Fragments();

    void AddFragment(Ptr<Packet> fragment, uint16_t fragmentOffset,
                     bool moreFragment);

    void SetUnfragmentablePart(Ptr<Packet> unfragmentablePart);

    bool IsEntire() const;

    Ptr<Packet> GetPacket() const;

    Ptr<Packet> GetPartialPacket() const;

    void SetTimeoutIter(FragmentsTimeoutsListI_t iter);

    FragmentsTimeoutsListI_t GetTimeoutIter();

  private:
    bool m_moreFragment;

    std::list<std::pair<Ptr<Packet>, uint16_t>> m_packetFragments;

    Ptr<Packet> m_unfragmentable;

    FragmentsTimeoutsListI_t m_timeoutIter;
  };

  void HandleFragmentsTimeout(FragmentKey_t key, Ipv6Header ipHeader);

  Ptr<Packet> GetPartialPacket() const;

  void SetTimeoutEventId(EventId event);

  void CancelTimeout();

  typedef std::map<FragmentKey_t, Ptr<Fragments>> MapFragments_t;

  MapFragments_t m_fragments;

  FragmentsTimeoutsListI_t SetTimeout(FragmentKey_t key, Ipv6Header ipHeader);

  void HandleTimeout();

  FragmentsTimeoutsList_t m_timeoutEventList;
  EventId m_timeoutEvent;
  Time m_fragmentExpirationTimeout;
};

class Ipv6ExtensionRouting : public Ipv6Extension {
public:
  static const uint8_t EXT_NUMBER = 43;

  static TypeId GetTypeId();

  Ipv6ExtensionRouting();

  ~Ipv6ExtensionRouting() override;

  uint8_t GetExtensionNumber() const override;

  virtual uint8_t GetTypeRouting() const;

  virtual Ipv6ExtensionRoutingHeader *GetExtensionRoutingHeaderPtr();

  uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, Ipv6Address dst,
                  uint8_t *nextHeader, bool &stopProcessing, bool &isDropped,
                  Ipv6L3Protocol::DropReason &dropReason) override;
};

class Ipv6ExtensionRoutingDemux : public Object {
public:
  static TypeId GetTypeId();

  Ipv6ExtensionRoutingDemux();

  ~Ipv6ExtensionRoutingDemux() override;

  void SetNode(Ptr<Node> node);

  void Insert(Ptr<Ipv6ExtensionRouting> extensionRouting);

  Ptr<Ipv6ExtensionRouting> GetExtensionRouting(uint8_t typeRouting);

  Ipv6ExtensionRoutingHeader *GetExtensionRoutingHeaderPtr(uint8_t typeRouting);

  void Remove(Ptr<Ipv6ExtensionRouting> extensionRouting);

protected:
  void DoDispose() override;

private:
  typedef std::list<Ptr<Ipv6ExtensionRouting>> Ipv6ExtensionRoutingList_t;

  Ipv6ExtensionRoutingList_t m_extensionsRouting;

  Ptr<Node> m_node;
};

class Ipv6ExtensionLooseRouting : public Ipv6ExtensionRouting {
public:
  static const uint8_t TYPE_ROUTING = 0;

  static TypeId GetTypeId();

  Ipv6ExtensionLooseRouting();

  ~Ipv6ExtensionLooseRouting() override;

  uint8_t GetTypeRouting() const override;

  Ipv6ExtensionRoutingHeader *GetExtensionRoutingHeaderPtr() override;

  uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, Ipv6Address dst,
                  uint8_t *nextHeader, bool &stopProcessing, bool &isDropped,
                  Ipv6L3Protocol::DropReason &dropReason) override;
};

class Ipv6ExtensionESP : public Ipv6Extension {
public:
  static const uint8_t EXT_NUMBER = 50;

  static TypeId GetTypeId();

  Ipv6ExtensionESP();

  ~Ipv6ExtensionESP() override;

  uint8_t GetExtensionNumber() const override;

  uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, Ipv6Address dst,
                  uint8_t *nextHeader, bool &stopProcessing, bool &isDropped,
                  Ipv6L3Protocol::DropReason &dropReason) override;
};

class Ipv6ExtensionAH : public Ipv6Extension {
public:
  static const uint8_t EXT_NUMBER = 51;

  static TypeId GetTypeId();

  Ipv6ExtensionAH();

  ~Ipv6ExtensionAH() override;

  uint8_t GetExtensionNumber() const override;

  uint8_t Process(Ptr<Packet> &packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, Ipv6Address dst,
                  uint8_t *nextHeader, bool &stopProcessing, bool &isDropped,
                  Ipv6L3Protocol::DropReason &dropReason) override;
};

} // namespace ns3

#endif
