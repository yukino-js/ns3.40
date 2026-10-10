
#ifndef TCP_HEADER_H
#define TCP_HEADER_H

#include "tcp-option.h"
#include "tcp-socket-factory.h"

#include "ns3/buffer.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/sequence-number.h"

#include <stdint.h>

namespace ns3 {

class TcpHeader : public Header {
public:
  typedef std::list<Ptr<const TcpOption>> TcpOptionList;

  friend std::ostream &operator<<(std::ostream &os, const TcpHeader &tc);

  static std::string FlagsToString(uint8_t flags,
                                   const std::string &delimiter = "|");

  void EnableChecksums();

  void SetSourcePort(uint16_t port);

  void SetDestinationPort(uint16_t port);

  void SetSequenceNumber(SequenceNumber32 sequenceNumber);

  void SetAckNumber(SequenceNumber32 ackNumber);

  void SetFlags(uint8_t flags);

  void SetWindowSize(uint16_t windowSize);

  void SetUrgentPointer(uint16_t urgentPointer);

  uint16_t GetSourcePort() const;

  uint16_t GetDestinationPort() const;

  SequenceNumber32 GetSequenceNumber() const;

  SequenceNumber32 GetAckNumber() const;

  uint8_t GetLength() const;

  uint8_t GetFlags() const;

  uint16_t GetWindowSize() const;

  uint16_t GetUrgentPointer() const;

  Ptr<const TcpOption> GetOption(uint8_t kind) const;

  const TcpOptionList &GetOptionList() const;

  uint8_t GetOptionLength() const;

  uint8_t GetMaxOptionLength() const;

  bool HasOption(uint8_t kind) const;

  bool AppendOption(Ptr<const TcpOption> option);

  void InitializeChecksum(const Ipv4Address &source,
                          const Ipv4Address &destination, uint8_t protocol);

  void InitializeChecksum(const Ipv6Address &source,
                          const Ipv6Address &destination, uint8_t protocol);

  void InitializeChecksum(const Address &source, const Address &destination,
                          uint8_t protocol);

  enum Flags_t {
    NONE = 0,
    FIN = 1,
    SYN = 2,
    RST = 4,
    PSH = 8,
    ACK = 16,
    URG = 32,
    ECE = 64,
    CWR = 128
  };

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  bool IsChecksumOk() const;

  friend bool operator==(const TcpHeader &lhs, const TcpHeader &rhs);

private:
  uint16_t CalculateHeaderChecksum(uint16_t size) const;

  uint8_t CalculateHeaderLength() const;

  uint16_t m_sourcePort{0};
  uint16_t m_destinationPort{0};
  SequenceNumber32 m_sequenceNumber{0};
  SequenceNumber32 m_ackNumber{0};
  uint8_t m_length{5};
  uint8_t m_flags{0};
  uint16_t m_windowSize{0xffff};
  uint16_t m_urgentPointer{0};

  Address m_source;
  Address m_destination;
  uint8_t m_protocol{6};

  bool m_calcChecksum{false};
  bool m_goodChecksum{true};

  static const uint8_t m_maxOptionsLen = 40;
  TcpOptionList m_options;
  uint8_t m_optionsLen{0};
};

} // namespace ns3

#endif
