
#ifndef MGT_HEADERS_H
#define MGT_HEADERS_H

#include "capability-information.h"
#include "edca-parameter-set.h"
#include "extended-capabilities.h"
#include "reduced-neighbor-report.h"
#include "ssid.h"
#include "status-code.h"
#include "supported-rates.h"
#include "wifi-mgt-header.h"

#include "ns3/dsss-parameter-set.h"
#include "ns3/eht-capabilities.h"
#include "ns3/eht-operation.h"
#include "ns3/erp-information.h"
#include "ns3/he-capabilities.h"
#include "ns3/he-operation.h"
#include "ns3/ht-capabilities.h"
#include "ns3/ht-operation.h"
#include "ns3/mac48-address.h"
#include "ns3/mu-edca-parameter-set.h"
#include "ns3/multi-link-element.h"
#include "ns3/tid-to-link-mapping-element.h"
#include "ns3/vht-capabilities.h"
#include "ns3/vht-operation.h"

#include <list>

namespace ns3 {

class Packet;

template <>
struct CanBeInPerStaProfile<ReducedNeighborReport> : std::false_type {};

template <> struct CanBeInPerStaProfile<TidToLinkMapping> : std::false_type {};

template <> struct CanBeInPerStaProfile<MultiLinkElement> : std::false_type {};

template <> struct CanBeInPerStaProfile<Ssid> : std::false_type {};

using ProbeRequestElems =
    std::tuple<Ssid, SupportedRates, std::optional<ExtendedSupportedRatesIE>,
               std::optional<HtCapabilities>,
               std::optional<ExtendedCapabilities>,
               std::optional<VhtCapabilities>, std::optional<HeCapabilities>,
               std::optional<EhtCapabilities>>;

using ProbeResponseElems = std::tuple<
    Ssid, SupportedRates, std::optional<DsssParameterSet>,
    std::optional<ErpInformation>, std::optional<ExtendedSupportedRatesIE>,
    std::optional<EdcaParameterSet>, std::optional<HtCapabilities>,
    std::optional<HtOperation>, std::optional<ExtendedCapabilities>,
    std::optional<VhtCapabilities>, std::optional<VhtOperation>,
    std::optional<ReducedNeighborReport>, std::optional<HeCapabilities>,
    std::optional<HeOperation>, std::optional<MuEdcaParameterSet>,
    std::optional<MultiLinkElement>, std::optional<EhtCapabilities>,
    std::optional<EhtOperation>, std::vector<TidToLinkMapping>>;

using AssocRequestElems =
    std::tuple<Ssid, SupportedRates, std::optional<ExtendedSupportedRatesIE>,
               std::optional<HtCapabilities>,
               std::optional<ExtendedCapabilities>,
               std::optional<VhtCapabilities>, std::optional<HeCapabilities>,
               std::optional<MultiLinkElement>, std::optional<EhtCapabilities>,
               std::vector<TidToLinkMapping>>;

using AssocResponseElems =
    std::tuple<SupportedRates, std::optional<ExtendedSupportedRatesIE>,
               std::optional<EdcaParameterSet>, std::optional<HtCapabilities>,
               std::optional<HtOperation>, std::optional<ExtendedCapabilities>,
               std::optional<VhtCapabilities>, std::optional<VhtOperation>,
               std::optional<HeCapabilities>, std::optional<HeOperation>,
               std::optional<MuEdcaParameterSet>,
               std::optional<MultiLinkElement>, std::optional<EhtCapabilities>,
               std::optional<EhtOperation>, std::vector<TidToLinkMapping>>;

class MgtAssocRequestHeader
    : public MgtHeaderInPerStaProfile<MgtAssocRequestHeader,
                                      AssocRequestElems> {
  friend class WifiMgtHeader<MgtAssocRequestHeader, AssocRequestElems>;
  friend class MgtHeaderInPerStaProfile<MgtAssocRequestHeader,
                                        AssocRequestElems>;

public:
  ~MgtAssocRequestHeader() override = default;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void SetListenInterval(uint16_t interval);
  uint16_t GetListenInterval() const;
  CapabilityInformation &Capabilities();
  const CapabilityInformation &Capabilities() const;

protected:
  uint32_t GetSerializedSizeImpl() const;
  void SerializeImpl(Buffer::Iterator start) const;
  uint32_t DeserializeImpl(Buffer::Iterator start);

  uint32_t GetSerializedSizeInPerStaProfileImpl(
      const MgtAssocRequestHeader &frame) const;

  void SerializeInPerStaProfileImpl(Buffer::Iterator start,
                                    const MgtAssocRequestHeader &frame) const;

  uint32_t DeserializeFromPerStaProfileImpl(Buffer::Iterator start,
                                            uint16_t length,
                                            const MgtAssocRequestHeader &frame);

private:
  CapabilityInformation m_capability;
  uint16_t m_listenInterval{0};
};

class MgtReassocRequestHeader
    : public MgtHeaderInPerStaProfile<MgtReassocRequestHeader,
                                      AssocRequestElems> {
  friend class WifiMgtHeader<MgtReassocRequestHeader, AssocRequestElems>;
  friend class MgtHeaderInPerStaProfile<MgtReassocRequestHeader,
                                        AssocRequestElems>;

public:
  ~MgtReassocRequestHeader() override = default;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void SetListenInterval(uint16_t interval);
  uint16_t GetListenInterval() const;
  CapabilityInformation &Capabilities();
  const CapabilityInformation &Capabilities() const;
  void SetCurrentApAddress(Mac48Address currentApAddr);

protected:
  uint32_t GetSerializedSizeImpl() const;
  void SerializeImpl(Buffer::Iterator start) const;
  uint32_t DeserializeImpl(Buffer::Iterator start);
  void PrintImpl(std::ostream &os) const;

  uint32_t GetSerializedSizeInPerStaProfileImpl(
      const MgtReassocRequestHeader &frame) const;

  void SerializeInPerStaProfileImpl(Buffer::Iterator start,
                                    const MgtReassocRequestHeader &frame) const;

  uint32_t
  DeserializeFromPerStaProfileImpl(Buffer::Iterator start, uint16_t length,
                                   const MgtReassocRequestHeader &frame);

private:
  Mac48Address m_currentApAddr;
  CapabilityInformation m_capability;
  uint16_t m_listenInterval{0};
};

class MgtAssocResponseHeader
    : public MgtHeaderInPerStaProfile<MgtAssocResponseHeader,
                                      AssocResponseElems> {
  friend class WifiMgtHeader<MgtAssocResponseHeader, AssocResponseElems>;
  friend class MgtHeaderInPerStaProfile<MgtAssocResponseHeader,
                                        AssocResponseElems>;

public:
  ~MgtAssocResponseHeader() override = default;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  StatusCode GetStatusCode();
  void SetStatusCode(StatusCode code);
  CapabilityInformation &Capabilities();
  const CapabilityInformation &Capabilities() const;
  uint16_t GetAssociationId() const;
  void SetAssociationId(uint16_t aid);

protected:
  uint32_t GetSerializedSizeImpl() const;
  void SerializeImpl(Buffer::Iterator start) const;
  uint32_t DeserializeImpl(Buffer::Iterator start);
  void PrintImpl(std::ostream &os) const;

  uint32_t GetSerializedSizeInPerStaProfileImpl(
      const MgtAssocResponseHeader &frame) const;

  void SerializeInPerStaProfileImpl(Buffer::Iterator start,
                                    const MgtAssocResponseHeader &frame) const;

  uint32_t
  DeserializeFromPerStaProfileImpl(Buffer::Iterator start, uint16_t length,
                                   const MgtAssocResponseHeader &frame);

private:
  CapabilityInformation m_capability;
  StatusCode m_code;
  uint16_t m_aid{0};
};

class MgtProbeRequestHeader
    : public WifiMgtHeader<MgtProbeRequestHeader, ProbeRequestElems> {
public:
  ~MgtProbeRequestHeader() override = default;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
};

class MgtProbeResponseHeader
    : public WifiMgtHeader<MgtProbeResponseHeader, ProbeResponseElems> {
  friend class WifiMgtHeader<MgtProbeResponseHeader, ProbeResponseElems>;

public:
  ~MgtProbeResponseHeader() override = default;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint64_t GetBeaconIntervalUs() const;
  void SetBeaconIntervalUs(uint64_t us);
  CapabilityInformation &Capabilities();
  const CapabilityInformation &Capabilities() const;
  uint64_t GetTimestamp() const;

protected:
  uint32_t GetSerializedSizeImpl() const;
  void SerializeImpl(Buffer::Iterator start) const;
  uint32_t DeserializeImpl(Buffer::Iterator start);

private:
  uint64_t m_timestamp;
  uint64_t m_beaconInterval;
  CapabilityInformation m_capability;
};

class MgtBeaconHeader : public MgtProbeResponseHeader {
public:
  ~MgtBeaconHeader() override = default;

  static TypeId GetTypeId();
};

class WifiActionHeader : public Header {
public:
  WifiActionHeader();
  ~WifiActionHeader() override;

  enum CategoryValue {
    QOS = 1,
    BLOCK_ACK = 3,
    PUBLIC = 4,
    RADIO_MEASUREMENT = 5,
    MESH = 13,
    MULTIHOP = 14,
    SELF_PROTECTED = 15,
    DMG = 16,
    FST = 18,
    UNPROTECTED_DMG = 20,
    PROTECTED_EHT = 37,
    VENDOR_SPECIFIC_ACTION = 127,
  };

  enum QosActionValue {
    ADDTS_REQUEST = 0,
    ADDTS_RESPONSE = 1,
    DELTS = 2,
    SCHEDULE = 3,
    QOS_MAP_CONFIGURE = 4,
  };

  enum BlockAckActionValue {
    BLOCK_ACK_ADDBA_REQUEST = 0,
    BLOCK_ACK_ADDBA_RESPONSE = 1,
    BLOCK_ACK_DELBA = 2
  };

  enum PublicActionValue {
    QAB_REQUEST = 16,
    QAB_RESPONSE = 17,
  };

  enum RadioMeasurementActionValue {
    RADIO_MEASUREMENT_REQUEST = 0,
    RADIO_MEASUREMENT_REPORT = 1,
    LINK_MEASUREMENT_REQUEST = 2,
    LINK_MEASUREMENT_REPORT = 3,
    NEIGHBOR_REPORT_REQUEST = 4,
    NEIGHBOR_REPORT_RESPONSE = 5
  };

  enum MeshActionValue {
    LINK_METRIC_REPORT = 0,
    PATH_SELECTION = 1,
    PORTAL_ANNOUNCEMENT = 2,
    CONGESTION_CONTROL_NOTIFICATION = 3,
    MDA_SETUP_REQUEST = 4,
    MDA_SETUP_REPLY = 5,
    MDAOP_ADVERTISEMENT_REQUEST = 6,
    MDAOP_ADVERTISEMENTS = 7,
    MDAOP_SET_TEARDOWN = 8,
    TBTT_ADJUSTMENT_REQUEST = 9,
    TBTT_ADJUSTMENT_RESPONSE = 10,
  };

  enum MultihopActionValue {
    PROXY_UPDATE = 0,
    PROXY_UPDATE_CONFIRMATION = 1,
  };

  enum SelfProtectedActionValue {
    PEER_LINK_OPEN = 1,
    PEER_LINK_CONFIRM = 2,
    PEER_LINK_CLOSE = 3,
    GROUP_KEY_INFORM = 4,
    GROUP_KEY_ACK = 5,
  };

  enum DmgActionValue {
    DMG_POWER_SAVE_CONFIGURATION_REQUEST = 0,
    DMG_POWER_SAVE_CONFIGURATION_RESPONSE = 1,
    DMG_INFORMATION_REQUEST = 2,
    DMG_INFORMATION_RESPONSE = 3,
    DMG_HANDOVER_REQUEST = 4,
    DMG_HANDOVER_RESPONSE = 5,
    DMG_DTP_REQUEST = 6,
    DMG_DTP_RESPONSE = 7,
    DMG_RELAY_SEARCH_REQUEST = 8,
    DMG_RELAY_SEARCH_RESPONSE = 9,
    DMG_MULTI_RELAY_CHANNEL_MEASUREMENT_REQUEST = 10,
    DMG_MULTI_RELAY_CHANNEL_MEASUREMENT_REPORT = 11,
    DMG_RLS_REQUEST = 12,
    DMG_RLS_RESPONSE = 13,
    DMG_RLS_ANNOUNCEMENT = 14,
    DMG_RLS_TEARDOWN = 15,
    DMG_RELAY_ACK_REQUEST = 16,
    DMG_RELAY_ACK_RESPONSE = 17,
    DMG_TPA_REQUEST = 18,
    DMG_TPA_RESPONSE = 19,
    DMG_TPA_REPORT = 20,
    DMG_ROC_REQUEST = 21,
    DMG_ROC_RESPONSE = 22
  };

  enum FstActionValue {
    FST_SETUP_REQUEST = 0,
    FST_SETUP_RESPONSE = 1,
    FST_TEAR_DOWN = 2,
    FST_ACK_REQUEST = 3,
    FST_ACK_RESPONSE = 4,
    ON_CHANNEL_TUNNEL_REQUEST = 5
  };

  enum UnprotectedDmgActionValue {
    UNPROTECTED_DMG_ANNOUNCE = 0,
    UNPROTECTED_DMG_BRP = 1,
    UNPROTECTED_MIMO_BF_SETUP = 2,
    UNPROTECTED_MIMO_BF_POLL = 3,
    UNPROTECTED_MIMO_BF_FEEDBACK = 4,
    UNPROTECTED_MIMO_BF_SELECTION = 5,
  };

  enum ProtectedEhtActionValue {
    PROTECTED_EHT_TID_TO_LINK_MAPPING_REQUEST = 0,
    PROTECTED_EHT_TID_TO_LINK_MAPPING_RESPONSE = 1,
    PROTECTED_EHT_TID_TO_LINK_MAPPING_TEARDOWN = 2,
    PROTECTED_EHT_EPCS_PRIORITY_ACCESS_ENABLE_REQUEST = 3,
    PROTECTED_EHT_EPCS_PRIORITY_ACCESS_ENABLE_RESPONSE = 4,
    PROTECTED_EHT_EPCS_PRIORITY_ACCESS_TEARDOWN = 5,
    PROTECTED_EHT_EML_OPERATING_MODE_NOTIFICATION = 6,
    PROTECTED_EHT_LINK_RECOMMENDATION = 7,
    PROTECTED_EHT_MULTI_LINK_OPERATION_UPDATE_REQUEST = 8,
    PROTECTED_EHT_MULTI_LINK_OPERATION_UPDATE_RESPONSE = 9,
  };

  typedef union {
    QosActionValue qos;
    BlockAckActionValue blockAck;
    RadioMeasurementActionValue radioMeasurementAction;
    PublicActionValue publicAction;
    SelfProtectedActionValue selfProtectedAction;
    MultihopActionValue multihopAction;
    MeshActionValue meshAction;
    DmgActionValue dmgAction;
    FstActionValue fstAction;
    UnprotectedDmgActionValue unprotectedDmgAction;
    ProtectedEhtActionValue protectedEhtAction;
  } ActionValue;

  void SetAction(CategoryValue type, ActionValue action);

  CategoryValue GetCategory() const;
  ActionValue GetAction() const;

  static std::pair<CategoryValue, ActionValue> Peek(Ptr<const Packet> pkt);

  static std::pair<CategoryValue, ActionValue> Remove(Ptr<Packet> pkt);

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_category;
  uint8_t m_actionValue;
};

class MgtAddBaRequestHeader : public Header {
public:
  MgtAddBaRequestHeader();

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetDelayedBlockAck();
  void SetImmediateBlockAck();
  void SetTid(uint8_t tid);
  void SetTimeout(uint16_t timeout);
  void SetBufferSize(uint16_t size);
  void SetStartingSequence(uint16_t seq);
  void SetAmsduSupport(bool supported);

  uint16_t GetStartingSequence() const;
  uint8_t GetTid() const;
  bool IsImmediateBlockAck() const;
  uint16_t GetTimeout() const;
  uint16_t GetBufferSize() const;
  bool IsAmsduSupported() const;

private:
  uint16_t GetParameterSet() const;
  void SetParameterSet(uint16_t params);
  uint16_t GetStartingSequenceControl() const;
  void SetStartingSequenceControl(uint16_t seqControl);

  uint8_t m_dialogToken;
  uint8_t m_amsduSupport;
  uint8_t m_policy;
  uint8_t m_tid;
  uint16_t m_bufferSize;
  uint16_t m_timeoutValue;
  uint16_t m_startingSeq;
};

class MgtAddBaResponseHeader : public Header {
public:
  MgtAddBaResponseHeader();

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetDelayedBlockAck();
  void SetImmediateBlockAck();
  void SetTid(uint8_t tid);
  void SetTimeout(uint16_t timeout);
  void SetBufferSize(uint16_t size);
  void SetStatusCode(StatusCode code);
  void SetAmsduSupport(bool supported);

  StatusCode GetStatusCode() const;
  uint8_t GetTid() const;
  bool IsImmediateBlockAck() const;
  uint16_t GetTimeout() const;
  uint16_t GetBufferSize() const;
  bool IsAmsduSupported() const;

private:
  uint16_t GetParameterSet() const;
  void SetParameterSet(uint16_t params);

  uint8_t m_dialogToken;
  StatusCode m_code;
  uint8_t m_amsduSupport;
  uint8_t m_policy;
  uint8_t m_tid;
  uint16_t m_bufferSize;
  uint16_t m_timeoutValue;
};

class MgtDelBaHeader : public Header {
public:
  MgtDelBaHeader();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  bool IsByOriginator() const;
  uint8_t GetTid() const;
  void SetTid(uint8_t tid);
  void SetByOriginator();
  void SetByRecipient();

private:
  uint16_t GetParameterSet() const;
  void SetParameterSet(uint16_t params);

  uint16_t m_initiator;
  uint16_t m_tid;
  uint16_t m_reasonCode;
};

class MgtEmlOmn : public Header {
public:
  MgtEmlOmn() = default;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  struct EmlControl {
    uint8_t emlsrMode : 1;
    uint8_t emlmrMode : 1;
    uint8_t emlsrParamUpdateCtrl : 1;
    uint8_t : 5;
    std::optional<uint16_t> linkBitmap;
    std::optional<uint8_t> mcsMapCountCtrl;
  };

  struct EmlsrParamUpdate {
    uint8_t paddingDelay : 3;
    uint8_t transitionDelay : 3;
  };

  void SetLinkIdInBitmap(uint8_t linkId);
  std::list<uint8_t> GetLinkBitmap() const;

  uint8_t m_dialogToken{0};
  EmlControl m_emlControl{};
  std::optional<EmlsrParamUpdate> m_emlsrParamUpdate{};
};

} // namespace ns3

#endif
