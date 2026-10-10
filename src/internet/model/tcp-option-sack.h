
#ifndef TCP_OPTION_SACK_H
#define TCP_OPTION_SACK_H

#include "tcp-option.h"

#include "ns3/sequence-number.h"

namespace ns3 {

class TcpOptionSack : public TcpOption {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  typedef std::pair<SequenceNumber32, SequenceNumber32> SackBlock;
  typedef std::list<SackBlock> SackList;

  TcpOptionSack();
  ~TcpOptionSack() override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;

  void AddSackBlock(SackBlock s);

  uint32_t GetNumSackBlocks() const;

  void ClearSackList();

  SackList GetSackList() const;

  friend std::ostream &operator<<(std::ostream &os,
                                  const TcpOptionSack &sackOption);

protected:
  SackList m_sackList;
};

std::ostream &operator<<(std::ostream &os, const TcpOptionSack &sackOption);

std::ostream &operator<<(std::ostream &os,
                         const TcpOptionSack::SackBlock &sackBlock);

} // namespace ns3

#endif
