
#ifndef RANN_INFORMATION_ELEMENT_H
#define RANN_INFORMATION_ELEMENT_H

#include "ns3/mac48-address.h"
#include "ns3/mesh-information-element-vector.h"

namespace ns3 {
namespace dot11s {
class IeRann : public WifiInformationElement {
public:
  IeRann();
  ~IeRann() override;
  void SetFlags(uint8_t flags);
  void SetHopcount(uint8_t hopcount);
  void SetTTL(uint8_t ttl);
  void SetOriginatorAddress(Mac48Address originator_address);
  void SetDestSeqNumber(uint32_t dest_seq_number);
  void SetMetric(uint32_t metric);
  uint8_t GetFlags() const;
  uint8_t GetHopcount() const;
  uint8_t GetTtl() const;
  Mac48Address GetOriginatorAddress();
  uint32_t GetDestSeqNumber() const;
  uint32_t GetMetric() const;
  void DecrementTtl();
  void IncrementMetric(uint32_t metric);

  WifiInformationElementId ElementId() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
  uint16_t GetInformationFieldSize() const override;
  void Print(std::ostream &os) const override;

private:
  uint8_t m_flags;
  uint8_t m_hopcount;
  uint8_t m_ttl;
  Mac48Address m_originatorAddress;
  uint32_t m_destSeqNumber;
  uint32_t m_metric;

  friend bool operator==(const IeRann &a, const IeRann &b);
};

bool operator==(const IeRann &a, const IeRann &b);
std::ostream &operator<<(std::ostream &os, const IeRann &rann);
} // namespace dot11s
} // namespace ns3

#endif
