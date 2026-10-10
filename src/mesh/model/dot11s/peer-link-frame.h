
#ifndef PEER_LINK_FRAME_START_H
#define PEER_LINK_FRAME_START_H
#include "dot11s-mac-header.h"
#include "ie-dot11s-configuration.h"
#include "ie-dot11s-id.h"
#include "ie-dot11s-peering-protocol.h"

#include "ns3/header.h"
#include "ns3/supported-rates.h"

#include <optional>

namespace ns3 {
namespace dot11s {
class PeerLinkOpenStart : public Header {
public:
  PeerLinkOpenStart();

  PeerLinkOpenStart(const PeerLinkOpenStart &) = delete;
  PeerLinkOpenStart &operator=(const PeerLinkOpenStart &) = delete;

  struct PlinkOpenStartFields {
    IePeeringProtocol protocol;
    uint16_t capability;
    SupportedRates rates;
    std::optional<ExtendedSupportedRatesIE> extendedRates;
    IeMeshId meshId;
    IeConfiguration config;
  };

  void SetPlinkOpenStart(PlinkOpenStartFields fields);
  PlinkOpenStartFields GetFields() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_capability;
  SupportedRates m_rates;
  std::optional<ExtendedSupportedRatesIE> m_extendedRates;
  IeMeshId m_meshId;
  IeConfiguration m_config;

  friend bool operator==(const PeerLinkOpenStart &a,
                         const PeerLinkOpenStart &b);
};

bool operator==(const PeerLinkOpenStart &a, const PeerLinkOpenStart &b);

class PeerLinkCloseStart : public Header {
public:
  PeerLinkCloseStart();

  PeerLinkCloseStart(const PeerLinkCloseStart &) = delete;
  PeerLinkCloseStart &operator=(const PeerLinkCloseStart &) = delete;

  struct PlinkCloseStartFields {
    IePeeringProtocol protocol;
    IeMeshId meshId;
  };

  void SetPlinkCloseStart(PlinkCloseStartFields fields);
  PlinkCloseStartFields GetFields() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  IeMeshId m_meshId;

  friend bool operator==(const PeerLinkCloseStart &a,
                         const PeerLinkCloseStart &b);
};

bool operator==(const PeerLinkCloseStart &a, const PeerLinkCloseStart &b);

class PeerLinkConfirmStart : public Header {
public:
  PeerLinkConfirmStart();

  PeerLinkConfirmStart(const PeerLinkConfirmStart &) = delete;
  PeerLinkConfirmStart &operator=(const PeerLinkConfirmStart &) = delete;

  struct PlinkConfirmStartFields {
    IePeeringProtocol protocol;
    uint16_t capability;
    uint16_t aid;
    SupportedRates rates;
    std::optional<ExtendedSupportedRatesIE> extendedRates;
    IeConfiguration config;
  };

  void SetPlinkConfirmStart(PlinkConfirmStartFields fields);
  PlinkConfirmStartFields GetFields() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_capability;
  uint16_t m_aid;
  SupportedRates m_rates;
  std::optional<ExtendedSupportedRatesIE> m_extendedRates;
  IeConfiguration m_config;

  friend bool operator==(const PeerLinkConfirmStart &a,
                         const PeerLinkConfirmStart &b);
};

bool operator==(const PeerLinkConfirmStart &a, const PeerLinkConfirmStart &b);
} // namespace dot11s
} // namespace ns3
#endif
