
#ifndef EPC_GTPC_HEADER_H
#define EPC_GTPC_HEADER_H

#include "epc-tft.h"
#include "eps-bearer.h"

#include "ns3/header.h"

namespace ns3 {

class GtpcHeader : public Header {
public:
  GtpcHeader();
  ~GtpcHeader() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  virtual uint32_t GetMessageSize() const;

  uint8_t GetMessageType() const;
  uint16_t GetMessageLength() const;
  uint32_t GetTeid() const;
  uint32_t GetSequenceNumber() const;

  void SetMessageType(uint8_t messageType);
  void SetMessageLength(uint16_t messageLength);
  void SetTeid(uint32_t teid);
  void SetSequenceNumber(uint32_t sequenceNumber);
  void SetIesLength(uint16_t iesLength);

  void ComputeMessageLength();

  enum InterfaceType_t {
    S1U_ENB_GTPU = 0,
    S5_SGW_GTPU = 4,
    S5_PGW_GTPU = 5,
    S5_SGW_GTPC = 6,
    S5_PGW_GTPC = 7,
    S11_MME_GTPC = 10,
  };

  struct Fteid_t {
    InterfaceType_t interfaceType;
    Ipv4Address addr;
    uint32_t teid;
  };

  enum MessageType_t {
    Reserved = 0,
    CreateSessionRequest = 32,
    CreateSessionResponse = 33,
    ModifyBearerRequest = 34,
    ModifyBearerResponse = 35,
    DeleteSessionRequest = 36,
    DeleteSessionResponse = 37,
    DeleteBearerCommand = 66,
    DeleteBearerRequest = 99,
    DeleteBearerResponse = 100,
  };

private:
  bool m_teidFlag;
  uint8_t m_messageType;
  uint16_t m_messageLength;
  uint32_t m_teid;
  uint32_t m_sequenceNumber;

protected:
  void PreSerialize(Buffer::Iterator &i) const;
  uint32_t PreDeserialize(Buffer::Iterator &i);
};

class GtpcIes {
public:
  enum Cause_t {
    RESERVED = 0,
    REQUEST_ACCEPTED = 16,
  };

  const uint32_t serializedSizeImsi = 12;
  const uint32_t serializedSizeCause = 6;
  const uint32_t serializedSizeEbi = 5;
  const uint32_t serializedSizeBearerQos = 26;
  const uint32_t serializedSizePacketFilter = 3 + 9 + 9 + 5 + 5 + 3;
  uint32_t GetSerializedSizeBearerTft(
      std::list<EpcTft::PacketFilter> packetFilters) const;
  const uint32_t serializedSizeUliEcgi = 12;
  const uint32_t serializedSizeFteid = 13;
  const uint32_t serializedSizeBearerContextHeader = 4;

  void SerializeImsi(Buffer::Iterator &i, uint64_t imsi) const;
  uint32_t DeserializeImsi(Buffer::Iterator &i, uint64_t &imsi) const;

  void SerializeCause(Buffer::Iterator &i, Cause_t cause) const;
  uint32_t DeserializeCause(Buffer::Iterator &i, Cause_t &cause) const;

  void SerializeEbi(Buffer::Iterator &i, uint8_t epsBearerId) const;
  uint32_t DeserializeEbi(Buffer::Iterator &i, uint8_t &epsBearerId) const;

  void WriteHtonU40(Buffer::Iterator &i, uint64_t data) const;
  uint64_t ReadNtohU40(Buffer::Iterator &i);

  void SerializeBearerQos(Buffer::Iterator &i, EpsBearer bearerQos) const;
  uint32_t DeserializeBearerQos(Buffer::Iterator &i, EpsBearer &bearerQos);

  void SerializeBearerTft(Buffer::Iterator &i,
                          std::list<EpcTft::PacketFilter> packetFilters) const;
  uint32_t DeserializeBearerTft(Buffer::Iterator &i, Ptr<EpcTft> epcTft) const;

  void SerializeUliEcgi(Buffer::Iterator &i, uint32_t uliEcgi) const;
  uint32_t DeserializeUliEcgi(Buffer::Iterator &i, uint32_t &uliEcgi) const;

  void SerializeFteid(Buffer::Iterator &i, GtpcHeader::Fteid_t fteid) const;
  uint32_t DeserializeFteid(Buffer::Iterator &i,
                            GtpcHeader::Fteid_t &fteid) const;

  void SerializeBearerContextHeader(Buffer::Iterator &i, uint16_t length) const;
  uint32_t DeserializeBearerContextHeader(Buffer::Iterator &i,
                                          uint16_t &length) const;
};

class GtpcCreateSessionRequestMessage : public GtpcHeader, public GtpcIes {
public:
  GtpcCreateSessionRequestMessage();
  ~GtpcCreateSessionRequestMessage() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  uint32_t GetMessageSize() const override;

  uint64_t GetImsi() const;
  void SetImsi(uint64_t imsi);

  uint32_t GetUliEcgi() const;
  void SetUliEcgi(uint32_t uliEcgi);

  GtpcHeader::Fteid_t GetSenderCpFteid() const;
  void SetSenderCpFteid(GtpcHeader::Fteid_t fteid);

  struct BearerContextToBeCreated {
    GtpcHeader::Fteid_t sgwS5uFteid;
    uint8_t epsBearerId;
    Ptr<EpcTft> tft;
    EpsBearer bearerLevelQos;
  };

  std::list<BearerContextToBeCreated> GetBearerContextsToBeCreated() const;
  void SetBearerContextsToBeCreated(
      std::list<BearerContextToBeCreated> bearerContexts);

private:
  uint64_t m_imsi;
  uint32_t m_uliEcgi;
  GtpcHeader::Fteid_t m_senderCpFteid;

  std::list<BearerContextToBeCreated> m_bearerContextsToBeCreated;
};

class GtpcCreateSessionResponseMessage : public GtpcHeader, public GtpcIes {
public:
  GtpcCreateSessionResponseMessage();
  ~GtpcCreateSessionResponseMessage() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  uint32_t GetMessageSize() const override;

  Cause_t GetCause() const;
  void SetCause(Cause_t cause);

  GtpcHeader::Fteid_t GetSenderCpFteid() const;
  void SetSenderCpFteid(GtpcHeader::Fteid_t fteid);

  struct BearerContextCreated {
    uint8_t epsBearerId;
    uint8_t cause;
    Ptr<EpcTft> tft;
    GtpcHeader::Fteid_t fteid;
    EpsBearer bearerLevelQos;
  };

  std::list<BearerContextCreated> GetBearerContextsCreated() const;
  void SetBearerContextsCreated(std::list<BearerContextCreated> bearerContexts);

private:
  Cause_t m_cause;
  GtpcHeader::Fteid_t m_senderCpFteid;
  std::list<BearerContextCreated> m_bearerContextsCreated;
};

class GtpcModifyBearerRequestMessage : public GtpcHeader, public GtpcIes {
public:
  GtpcModifyBearerRequestMessage();
  ~GtpcModifyBearerRequestMessage() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  uint32_t GetMessageSize() const override;

  uint64_t GetImsi() const;
  void SetImsi(uint64_t imsi);

  uint32_t GetUliEcgi() const;
  void SetUliEcgi(uint32_t uliEcgi);

  struct BearerContextToBeModified {
    uint8_t epsBearerId;
    GtpcHeader::Fteid_t fteid;
  };

  std::list<BearerContextToBeModified> GetBearerContextsToBeModified() const;
  void SetBearerContextsToBeModified(
      std::list<BearerContextToBeModified> bearerContexts);

private:
  uint64_t m_imsi;
  uint32_t m_uliEcgi;

  std::list<BearerContextToBeModified> m_bearerContextsToBeModified;
};

class GtpcModifyBearerResponseMessage : public GtpcHeader, public GtpcIes {
public:
  GtpcModifyBearerResponseMessage();
  ~GtpcModifyBearerResponseMessage() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  uint32_t GetMessageSize() const override;

  Cause_t GetCause() const;
  void SetCause(Cause_t cause);

private:
  Cause_t m_cause;
};

class GtpcDeleteBearerCommandMessage : public GtpcHeader, public GtpcIes {
public:
  GtpcDeleteBearerCommandMessage();
  ~GtpcDeleteBearerCommandMessage() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  uint32_t GetMessageSize() const override;

  struct BearerContext {
    uint8_t m_epsBearerId;
  };

  std::list<BearerContext> GetBearerContexts() const;
  void SetBearerContexts(std::list<BearerContext> bearerContexts);

private:
  std::list<BearerContext> m_bearerContexts;
};

class GtpcDeleteBearerRequestMessage : public GtpcHeader, public GtpcIes {
public:
  GtpcDeleteBearerRequestMessage();
  ~GtpcDeleteBearerRequestMessage() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  uint32_t GetMessageSize() const override;

  std::list<uint8_t> GetEpsBearerIds() const;
  void SetEpsBearerIds(std::list<uint8_t> epsBearerIds);

private:
  std::list<uint8_t> m_epsBearerIds;
};

class GtpcDeleteBearerResponseMessage : public GtpcHeader, public GtpcIes {
public:
  GtpcDeleteBearerResponseMessage();
  ~GtpcDeleteBearerResponseMessage() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  uint32_t GetMessageSize() const override;

  Cause_t GetCause() const;
  void SetCause(Cause_t cause);

  std::list<uint8_t> GetEpsBearerIds() const;
  void SetEpsBearerIds(std::list<uint8_t> epsBearerIds);

private:
  Cause_t m_cause;
  std::list<uint8_t> m_epsBearerIds;
};

} // namespace ns3

#endif
