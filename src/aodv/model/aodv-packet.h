#ifndef AODVPACKET_H
#define AODVPACKET_H

#include "ns3/enum.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <iostream>
#include <map>

namespace ns3 {
namespace aodv {

enum MessageType {
  AODVTYPE_RREQ = 1,
  AODVTYPE_RREP = 2,
  AODVTYPE_RERR = 3,
  AODVTYPE_RREP_ACK = 4
};

class TypeHeader : public Header {
public:
  TypeHeader(MessageType t = AODVTYPE_RREQ);

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  MessageType Get() const { return m_type; }

  bool IsValid() const { return m_valid; }

  bool operator==(const TypeHeader &o) const;

private:
  MessageType m_type;
  bool m_valid;
};

std::ostream &operator<<(std::ostream &os, const TypeHeader &h);

class RreqHeader : public Header {
public:
  RreqHeader(uint8_t flags = 0, uint8_t reserved = 0, uint8_t hopCount = 0,
             uint32_t requestID = 0, Ipv4Address dst = Ipv4Address(),
             uint32_t dstSeqNo = 0, Ipv4Address origin = Ipv4Address(),
             uint32_t originSeqNo = 0);

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  void SetHopCount(uint8_t count) { m_hopCount = count; }

  uint8_t GetHopCount() const { return m_hopCount; }

  void SetId(uint32_t id) { m_requestID = id; }

  uint32_t GetId() const { return m_requestID; }

  void SetDst(Ipv4Address a) { m_dst = a; }

  Ipv4Address GetDst() const { return m_dst; }

  void SetDstSeqno(uint32_t s) { m_dstSeqNo = s; }

  uint32_t GetDstSeqno() const { return m_dstSeqNo; }

  void SetOrigin(Ipv4Address a) { m_origin = a; }

  Ipv4Address GetOrigin() const { return m_origin; }

  void SetOriginSeqno(uint32_t s) { m_originSeqNo = s; }

  uint32_t GetOriginSeqno() const { return m_originSeqNo; }

  void SetGratuitousRrep(bool f);
  bool GetGratuitousRrep() const;
  void SetDestinationOnly(bool f);
  bool GetDestinationOnly() const;
  void SetUnknownSeqno(bool f);
  bool GetUnknownSeqno() const;

  bool operator==(const RreqHeader &o) const;

private:
  uint8_t m_flags;
  uint8_t m_reserved;
  uint8_t m_hopCount;
  uint32_t m_requestID;
  Ipv4Address m_dst;
  uint32_t m_dstSeqNo;
  Ipv4Address m_origin;
  uint32_t m_originSeqNo;
};

std::ostream &operator<<(std::ostream &os, const RreqHeader &);

class RrepHeader : public Header {
public:
  RrepHeader(uint8_t prefixSize = 0, uint8_t hopCount = 0,
             Ipv4Address dst = Ipv4Address(), uint32_t dstSeqNo = 0,
             Ipv4Address origin = Ipv4Address(),
             Time lifetime = MilliSeconds(0));
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  void SetHopCount(uint8_t count) { m_hopCount = count; }

  uint8_t GetHopCount() const { return m_hopCount; }

  void SetDst(Ipv4Address a) { m_dst = a; }

  Ipv4Address GetDst() const { return m_dst; }

  void SetDstSeqno(uint32_t s) { m_dstSeqNo = s; }

  uint32_t GetDstSeqno() const { return m_dstSeqNo; }

  void SetOrigin(Ipv4Address a) { m_origin = a; }

  Ipv4Address GetOrigin() const { return m_origin; }

  void SetLifeTime(Time t);
  Time GetLifeTime() const;

  void SetAckRequired(bool f);
  bool GetAckRequired() const;
  void SetPrefixSize(uint8_t sz);
  uint8_t GetPrefixSize() const;

  void SetHello(Ipv4Address src, uint32_t srcSeqNo, Time lifetime);

  bool operator==(const RrepHeader &o) const;

private:
  uint8_t m_flags;
  uint8_t m_prefixSize;
  uint8_t m_hopCount;
  Ipv4Address m_dst;
  uint32_t m_dstSeqNo;
  Ipv4Address m_origin;
  uint32_t m_lifeTime;
};

std::ostream &operator<<(std::ostream &os, const RrepHeader &);

class RrepAckHeader : public Header {
public:
  RrepAckHeader();

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  bool operator==(const RrepAckHeader &o) const;

private:
  uint8_t m_reserved;
};

std::ostream &operator<<(std::ostream &os, const RrepAckHeader &);

class RerrHeader : public Header {
public:
  RerrHeader();

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator i) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  void SetNoDelete(bool f);
  bool GetNoDelete() const;

  bool AddUnDestination(Ipv4Address dst, uint32_t seqNo);
  bool RemoveUnDestination(std::pair<Ipv4Address, uint32_t> &un);
  void Clear();

  uint8_t GetDestCount() const { return (uint8_t)m_unreachableDstSeqNo.size(); }

  bool operator==(const RerrHeader &o) const;

private:
  uint8_t m_flag;
  uint8_t m_reserved;

  std::map<Ipv4Address, uint32_t> m_unreachableDstSeqNo;
};

std::ostream &operator<<(std::ostream &os, const RerrHeader &);

} // namespace aodv
} // namespace ns3

#endif
