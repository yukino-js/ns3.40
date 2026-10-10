#ifndef TCPOPTIONRFC793_H
#define TCPOPTIONRFC793_H

#include "tcp-option.h"

namespace ns3 {

class TcpOptionEnd : public TcpOption {
public:
  TcpOptionEnd();
  ~TcpOptionEnd() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;
};

class TcpOptionNOP : public TcpOption {
public:
  TcpOptionNOP();
  ~TcpOptionNOP() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;
};

class TcpOptionMSS : public TcpOption {
public:
  TcpOptionMSS();
  ~TcpOptionMSS() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;

  uint16_t GetMSS() const;
  void SetMSS(uint16_t mss);

protected:
  uint16_t m_mss;
};

} // namespace ns3

#endif
