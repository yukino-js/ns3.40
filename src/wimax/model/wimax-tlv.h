
#ifndef WIMAX_TLV_H
#define WIMAX_TLV_H

#define WIMAX_TLV_EXTENDED_LENGTH_MASK 0x80

#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"

#include <cstdlib>
#include <vector>

namespace ns3 {

class TlvValue {
public:
  virtual ~TlvValue() {}

  virtual uint32_t GetSerializedSize() const = 0;
  virtual void Serialize(Buffer::Iterator start) const = 0;
  virtual uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLen) = 0;
  virtual TlvValue *Copy() const = 0;

private:
};

class Tlv : public Header {
public:
  enum CommonTypes {
    HMAC_TUPLE = 149,
    MAC_VERSION_ENCODING = 148,
    CURRENT_TRANSMIT_POWER = 147,
    DOWNLINK_SERVICE_FLOW = 146,
    UPLINK_SERVICE_FLOW = 145,
    VENDOR_ID_EMCODING = 144,
    VENDOR_SPECIFIC_INFORMATION = 143
  };

  Tlv(uint8_t type, uint64_t length, const TlvValue &value);
  Tlv();
  ~Tlv() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  uint8_t GetSizeOfLen() const;
  uint8_t GetType() const;
  uint64_t GetLength() const;
  TlvValue *PeekValue();
  Tlv *Copy() const;
  TlvValue *CopyValue() const;
  Tlv &operator=(const Tlv &o);
  Tlv(const Tlv &tlv);

private:
  uint8_t m_type;
  uint64_t m_length;
  TlvValue *m_value;
};

class U8TlvValue : public TlvValue {
public:
  U8TlvValue(uint8_t value);
  U8TlvValue();
  ~U8TlvValue() override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLen) override;
  uint32_t Deserialize(Buffer::Iterator start);
  uint8_t GetValue() const;
  U8TlvValue *Copy() const override;

private:
  uint8_t m_value;
};

class U16TlvValue : public TlvValue {
public:
  U16TlvValue(uint16_t value);
  U16TlvValue();
  ~U16TlvValue() override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLen) override;
  uint32_t Deserialize(Buffer::Iterator start);
  uint16_t GetValue() const;
  U16TlvValue *Copy() const override;

private:
  uint16_t m_value;
};

class U32TlvValue : public TlvValue {
public:
  U32TlvValue(uint32_t value);
  U32TlvValue();
  ~U32TlvValue() override;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLen) override;
  uint32_t Deserialize(Buffer::Iterator start);
  uint32_t GetValue() const;
  U32TlvValue *Copy() const override;

private:
  uint32_t m_value;
};

class VectorTlvValue : public TlvValue {
public:
  typedef std::vector<Tlv *>::const_iterator Iterator;
  VectorTlvValue();
  ~VectorTlvValue() override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start,
                       uint64_t valueLength) override = 0;
  Iterator Begin() const;
  Iterator End() const;
  void Add(const Tlv &val);
  VectorTlvValue *Copy() const override = 0;

private:
  std::vector<Tlv *> *m_tlvList;
};

class SfVectorTlvValue : public VectorTlvValue {
public:
  enum Type {
    SFID = 1,
    CID = 2,
    Service_Class_Name = 3,
    reserved1 = 4,
    QoS_Parameter_Set_Type = 5,
    Traffic_Priority = 6,
    Maximum_Sustained_Traffic_Rate = 7,
    Maximum_Traffic_Burst = 8,
    Minimum_Reserved_Traffic_Rate = 9,
    Minimum_Tolerable_Traffic_Rate = 10,
    Service_Flow_Scheduling_Type = 11,
    Request_Transmission_Policy = 12,
    Tolerated_Jitter = 13,
    Maximum_Latency = 14,
    Fixed_length_versus_Variable_length_SDU_Indicator = 15,
    SDU_Size = 16,
    Target_SAID = 17,
    ARQ_Enable = 18,
    ARQ_WINDOW_SIZE = 19,
    ARQ_RETRY_TIMEOUT_Transmitter_Delay = 20,
    ARQ_RETRY_TIMEOUT_Receiver_Delay = 21,
    ARQ_BLOCK_LIFETIME = 22,
    ARQ_SYNC_LOSS = 23,
    ARQ_DELIVER_IN_ORDER = 24,
    ARQ_PURGE_TIMEOUT = 25,
    ARQ_BLOCK_SIZE = 26,
    reserved2 = 27,
    CS_Specification = 28,
    IPV4_CS_Parameters = 100
  };

  SfVectorTlvValue();
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLength) override;
  SfVectorTlvValue *Copy() const override;
};

class CsParamVectorTlvValue : public VectorTlvValue {
public:
  enum Type {
    Classifier_DSC_Action = 1,
    Packet_Classification_Rule = 3,
  };

  CsParamVectorTlvValue();
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLength) override;
  CsParamVectorTlvValue *Copy() const override;

private:
};

class ClassificationRuleVectorTlvValue : public VectorTlvValue {
public:
  enum ClassificationRuleTlvType {
    Priority = 1,
    ToS = 2,
    Protocol = 3,
    IP_src = 4,
    IP_dst = 5,
    Port_src = 6,
    Port_dst = 7,
    Index = 14,
  };

  ClassificationRuleVectorTlvValue();
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLength) override;
  ClassificationRuleVectorTlvValue *Copy() const override;

private:
};

class TosTlvValue : public TlvValue {
public:
  TosTlvValue();
  TosTlvValue(uint8_t low, uint8_t high, uint8_t mask);
  ~TosTlvValue() override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLength) override;
  uint8_t GetLow() const;
  uint8_t GetHigh() const;
  uint8_t GetMask() const;
  TosTlvValue *Copy() const override;

private:
  uint8_t m_low;
  uint8_t m_high;
  uint8_t m_mask;
};

class PortRangeTlvValue : public TlvValue {
public:
  struct PortRange {
    uint16_t PortLow;
    uint16_t PortHigh;
  };

  typedef std::vector<PortRange>::const_iterator Iterator;
  PortRangeTlvValue();
  ~PortRangeTlvValue() override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLength) override;
  void Add(uint16_t portLow, uint16_t portHigh);
  Iterator Begin() const;
  Iterator End() const;
  PortRangeTlvValue *Copy() const override;

private:
  std::vector<PortRange> *m_portRange;
};

class ProtocolTlvValue : public TlvValue {
public:
  ProtocolTlvValue();
  ~ProtocolTlvValue() override;
  typedef std::vector<uint8_t>::const_iterator Iterator;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLength) override;
  void Add(uint8_t protocol);
  Iterator Begin() const;
  Iterator End() const;
  ProtocolTlvValue *Copy() const override;

private:
  std::vector<uint8_t> *m_protocol;
};

class Ipv4AddressTlvValue : public TlvValue {
public:
  struct Ipv4Addr {
    Ipv4Address Address;
    Ipv4Mask Mask;
  };

  typedef std::vector<Ipv4Addr>::const_iterator Iterator;
  Ipv4AddressTlvValue();
  ~Ipv4AddressTlvValue() override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start, uint64_t valueLength) override;
  void Add(Ipv4Address address, Ipv4Mask mask);
  Iterator Begin() const;
  Iterator End() const;
  Ipv4AddressTlvValue *Copy() const override;

private:
  std::vector<Ipv4Addr> *m_ipv4Addr;
};

} // namespace ns3

#endif
