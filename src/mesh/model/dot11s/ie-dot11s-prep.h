
#ifndef WIFI_PREP_INFORMATION_ELEMENT_H
#define WIFI_PREP_INFORMATION_ELEMENT_H

#include "ns3/mac48-address.h"
#include "ns3/mesh-information-element-vector.h"

namespace ns3 {
namespace dot11s {
class IePrep : public WifiInformationElement {
public:
  IePrep();
  ~IePrep() override;
  void SetFlags(uint8_t flags);
  void SetHopcount(uint8_t hopcount);
  void SetTtl(uint8_t ttl);
  void SetDestinationAddress(Mac48Address dest_address);
  void SetDestinationSeqNumber(uint32_t dest_seq_number);
  void SetLifetime(uint32_t lifetime);
  void SetMetric(uint32_t metric);
  void SetOriginatorAddress(Mac48Address originator_address);
  void SetOriginatorSeqNumber(uint32_t originator_seq_number);

  uint8_t GetFlags() const;
  uint8_t GetHopcount() const;
  uint32_t GetTtl() const;
  Mac48Address GetDestinationAddress() const;
  uint32_t GetDestinationSeqNumber() const;
  uint32_t GetLifetime() const;
  uint32_t GetMetric() const;
  Mac48Address GetOriginatorAddress() const;
  uint32_t GetOriginatorSeqNumber() const;

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
  Mac48Address m_destinationAddress;
  uint32_t m_destSeqNumber;
  uint32_t m_lifetime;
  uint32_t m_metric;
  Mac48Address m_originatorAddress;
  uint32_t m_originatorSeqNumber;
  friend bool operator==(const IePrep &a, const IePrep &b);
};

bool operator==(const IePrep &a, const IePrep &b);
std::ostream &operator<<(std::ostream &os, const IePrep &prep);
} // namespace dot11s
} // namespace ns3
#endif
