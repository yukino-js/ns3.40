
#ifndef NIX_VECTOR_H
#define NIX_VECTOR_H

#include "buffer.h"

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

class NixVector : public SimpleRefCount<NixVector> {
public:
  NixVector();
  ~NixVector();
  Ptr<NixVector> Copy() const;
  NixVector(const NixVector &o);
  NixVector &operator=(const NixVector &o);
  void AddNeighborIndex(uint32_t newBits, uint32_t numberOfBits);
  uint32_t ExtractNeighborIndex(uint32_t numberOfBits);
  uint32_t GetRemainingBits() const;
  uint32_t GetSerializedSize() const;
  uint32_t Serialize(uint32_t *buffer, uint32_t maxSize) const;
  uint32_t Deserialize(const uint32_t *buffer, uint32_t size);
  uint32_t BitCount(uint32_t numberOfNeighbors) const;

  void SetEpoch(uint32_t epoch);

  uint32_t GetEpoch() const;

private:
  typedef std::vector<uint32_t> NixBits_t;

  void DumpNixVector(std::ostream &os) const;

  friend std::ostream &operator<<(std::ostream &os, const NixVector &nix);

  NixBits_t m_nixVector;
  uint32_t m_used;

  uint32_t m_totalBitSize;

  uint32_t m_epoch;

  void PrintDec2BinNix(uint32_t decimalNum, uint32_t bitCount,
                       std::ostream &os) const;
};
} // namespace ns3

#endif
