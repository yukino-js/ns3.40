
#ifndef WIFI_PREQ_INFORMATION_ELEMENT_H
#define WIFI_PREQ_INFORMATION_ELEMENT_H

#include "ns3/mac48-address.h"
#include "ns3/mesh-information-element-vector.h"

#include <vector>

namespace ns3 {
namespace dot11s {
class DestinationAddressUnit : public SimpleRefCount<DestinationAddressUnit> {
public:
  DestinationAddressUnit();
  void SetFlags(bool doFlag, bool rfFlag, bool usnFlag);
  void SetDestinationAddress(Mac48Address dest_address);
  void SetDestSeqNumber(uint32_t dest_seq_number);
  bool IsDo() const;
  bool IsRf() const;
  bool IsUsn() const;
  Mac48Address GetDestinationAddress() const;
  uint32_t GetDestSeqNumber() const;

private:
  bool m_do;
  bool m_rf;
  bool m_usn;
  Mac48Address m_destinationAddress;
  uint32_t m_destSeqNumber;

  friend bool operator==(const DestinationAddressUnit &a,
                         const DestinationAddressUnit &b);
};

class IePreq : public WifiInformationElement {
public:
  IePreq();
  ~IePreq() override;
  void AddDestinationAddressElement(bool doFlag, bool rfFlag,
                                    Mac48Address dest_address,
                                    uint32_t dest_seq_number);
  void DelDestinationAddressElement(Mac48Address dest_address);
  void ClearDestinationAddressElements();
  std::vector<Ptr<DestinationAddressUnit>> GetDestinationList();
  void SetUnicastPreq();
  void SetNeedNotPrep();

  void SetHopcount(uint8_t hopcount);
  void SetTTL(uint8_t ttl);
  void SetPreqID(uint32_t id);
  void SetOriginatorAddress(Mac48Address originator_address);
  void SetOriginatorSeqNumber(uint32_t originator_seq_number);
  void SetLifetime(uint32_t lifetime);
  void SetMetric(uint32_t metric);
  void SetDestCount(uint8_t dest_count);

  bool IsUnicastPreq() const;
  bool IsNeedNotPrep() const;
  uint8_t GetHopCount() const;
  uint8_t GetTtl() const;
  uint32_t GetPreqID() const;
  Mac48Address GetOriginatorAddress() const;
  uint32_t GetOriginatorSeqNumber() const;
  uint32_t GetLifetime() const;
  uint32_t GetMetric() const;
  uint8_t GetDestCount() const;

  void DecrementTtl();
  void IncrementMetric(uint32_t metric);
  bool MayAddAddress(Mac48Address originator);
  bool IsFull() const;

  WifiInformationElementId ElementId() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator i,
                                       uint16_t length) override;
  uint16_t GetInformationFieldSize() const override;
  void Print(std::ostream &os) const override;

private:
  uint8_t m_maxSize;

  uint8_t m_flags;
  uint8_t m_hopCount;
  uint8_t m_ttl;
  uint32_t m_preqId;
  Mac48Address m_originatorAddress;
  uint32_t m_originatorSeqNumber;
  uint32_t m_lifetime;
  uint32_t m_metric;
  uint8_t m_destCount;
  std::vector<Ptr<DestinationAddressUnit>> m_destinations;

  friend bool operator==(const IePreq &a, const IePreq &b);
};

bool operator==(const DestinationAddressUnit &a,
                const DestinationAddressUnit &b);
bool operator==(const IePreq &a, const IePreq &b);
std::ostream &operator<<(std::ostream &os, const IePreq &preq);

} // namespace dot11s
} // namespace ns3
#endif
