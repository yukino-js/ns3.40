
#include "nix-vector.h"

#include "ns3/fatal-error.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NixVector");

typedef std::vector<uint32_t> NixBits_t;

NixVector::NixVector()
    : m_nixVector(0), m_used(0), m_totalBitSize(0), m_epoch(0) {
  NS_LOG_FUNCTION(this);
}

NixVector::~NixVector() { NS_LOG_FUNCTION(this); }

NixVector::NixVector(const NixVector &o)
    : m_nixVector(o.m_nixVector), m_used(o.m_used),
      m_totalBitSize(o.m_totalBitSize), m_epoch(o.m_epoch) {}

NixVector &NixVector::operator=(const NixVector &o) {
  if (this == &o) {
    return *this;
  }
  m_nixVector = o.m_nixVector;
  m_used = o.m_used;
  m_totalBitSize = o.m_totalBitSize;
  m_epoch = o.m_epoch;
  return *this;
}

Ptr<NixVector> NixVector::Copy() const {
  NS_LOG_FUNCTION(this);
  return Ptr<NixVector>(new NixVector(*this), false);
}

std::ostream &operator<<(std::ostream &os, const NixVector &nix) {
  nix.DumpNixVector(os);
  os << " (" << nix.GetRemainingBits() << " bits left)";
  return os;
}

void NixVector::AddNeighborIndex(uint32_t newBits, uint32_t numberOfBits) {
  NS_LOG_FUNCTION(this << newBits << numberOfBits);

  if (numberOfBits > 32) {
    NS_FATAL_ERROR("Can't add more than 32 bits to a nix-vector at one time");
  }

  uint32_t currentVectorBitSize = m_totalBitSize % 32;

  if (currentVectorBitSize == 0) {
    m_nixVector.push_back(0);
  }

  if (currentVectorBitSize + numberOfBits > 32) {
    uint32_t tempBits = newBits;
    tempBits = newBits << currentVectorBitSize;
    tempBits |= m_nixVector.back();
    m_nixVector.back() = tempBits;

    newBits = newBits >> (32 - currentVectorBitSize);
    m_nixVector.push_back(newBits);
  } else {
    newBits = newBits << currentVectorBitSize;
    newBits |= m_nixVector.back();

    m_nixVector.back() = newBits;
  }
  m_totalBitSize += numberOfBits;
}

uint32_t NixVector::ExtractNeighborIndex(uint32_t numberOfBits) {
  NS_LOG_FUNCTION(this << numberOfBits);

  if (numberOfBits > 32) {
    NS_FATAL_ERROR(
        "Can't extract more than 32 bits to a nix-vector at one time");
  }

  uint32_t vectorIndex = 0;
  uint32_t extractedBits = 0;
  uint32_t totalRemainingBits = GetRemainingBits();

  if (numberOfBits > totalRemainingBits) {
    NS_FATAL_ERROR("You've tried to extract too many bits of the Nix-vector, "
                   << this << ". NumberBits: " << numberOfBits
                   << " Remaining: " << totalRemainingBits);
  }

  if (numberOfBits <= 0) {
    NS_FATAL_ERROR("You've specified a number of bits for Nix-vector <= 0!");
  }

  vectorIndex = ((totalRemainingBits - 1) / 32);

  if (vectorIndex > 0) {
    if ((numberOfBits - 1) > ((totalRemainingBits - 1) % 32)) {
      extractedBits = m_nixVector.at(vectorIndex)
                      << (32 - (totalRemainingBits % 32));
      extractedBits =
          extractedBits >> ((32 - (totalRemainingBits % 32)) -
                            (numberOfBits - (totalRemainingBits % 32)));
      extractedBits |= (m_nixVector.at(vectorIndex - 1) >>
                        (32 - (numberOfBits - (totalRemainingBits % 32))));
      m_used += numberOfBits;
      return extractedBits;
    }
  }

  extractedBits = m_nixVector.at(vectorIndex)
                  << (32 - (totalRemainingBits % 32));
  extractedBits = extractedBits >> (32 - (numberOfBits));
  m_used += numberOfBits;
  return extractedBits;
}

uint32_t NixVector::GetSerializedSize() const {
  NS_LOG_FUNCTION(this);

  if (m_totalBitSize == 0) {
    return sizeof(m_totalBitSize);
  }

  return sizeof(m_used) + sizeof(m_totalBitSize) +
         (sizeof(uint32_t) * m_nixVector.size()) + sizeof(m_epoch);
}

uint32_t NixVector::Serialize(uint32_t *buffer, uint32_t maxSize) const {
  NS_LOG_FUNCTION(this << buffer << maxSize);
  uint32_t *p = buffer;

  if (maxSize < GetSerializedSize()) {
    return 0;
  }

  *p++ = m_totalBitSize;

  if (m_totalBitSize) {
    *p++ = m_used;
    for (uint32_t j = 0; j < m_nixVector.size(); j++) {
      *p++ = m_nixVector.at(j);
    }
    *p++ = m_epoch;
  }

  return 1;
}

uint32_t NixVector::Deserialize(const uint32_t *buffer, uint32_t size) {
  NS_LOG_FUNCTION(this << buffer << size);
  const uint32_t *p = buffer;

  NS_ASSERT_MSG(size >= sizeof(m_totalBitSize),
                "NixVector minimum serialized length is "
                    << sizeof(m_totalBitSize) << " bytes");
  if (size < sizeof(m_totalBitSize)) {
    return 0;
  }

  m_totalBitSize = *p++;

  if (m_totalBitSize) {
    m_used = *p++;

    uint32_t nixVectorLength = m_totalBitSize / 32;
    nixVectorLength += (m_totalBitSize % 32) ? 1 : 0;

    NS_ASSERT_MSG(size >= 16 + nixVectorLength,
                  "NixVector serialized length should have been "
                      << 16 + nixVectorLength << " but buffer is shorter");
    if (size < 16 + nixVectorLength * 4) {
      return 0;
    }

    m_nixVector.clear();
    for (uint32_t j = 0; j < nixVectorLength; j++) {
      uint32_t nix = *p++;
      m_nixVector.push_back(nix);
    }

    m_epoch = *p++;
  }

  return (GetSerializedSize());
}

void NixVector::DumpNixVector(std::ostream &os) const {
  NS_LOG_FUNCTION(this << &os);

  if (m_nixVector.empty()) {
    os << "0";
    return;
  }

  std::vector<uint32_t>::const_reverse_iterator rIter;
  bool first = true;

  for (rIter = m_nixVector.rbegin(); rIter != m_nixVector.rend();) {
    if (m_totalBitSize % 32 != 0 && first) {
      PrintDec2BinNix(*rIter, m_totalBitSize % 32, os);
    } else {
      PrintDec2BinNix(*rIter, 32, os);
    }
    first = false;

    rIter++;
    if (rIter != m_nixVector.rend()) {
      os << "--";
    }
  }
}

uint32_t NixVector::GetRemainingBits() const {
  NS_LOG_FUNCTION(this);

  return (m_totalBitSize - m_used);
}

uint32_t NixVector::BitCount(uint32_t numberOfNeighbors) const {
  NS_LOG_FUNCTION(this << numberOfNeighbors);

  uint32_t bitCount = 0;

  if (numberOfNeighbors < 2) {
    return 1;
  } else {
    for (numberOfNeighbors -= 1; numberOfNeighbors != 0;
         numberOfNeighbors >>= 1) {
      bitCount++;
    }
    return bitCount;
  }
}

void NixVector::PrintDec2BinNix(uint32_t decimalNum, uint32_t bitCount,
                                std::ostream &os) const {
  NS_LOG_FUNCTION(this << decimalNum << bitCount << &os);
  if (decimalNum == 0) {
    for (; bitCount > 0; bitCount--) {
      os << 0;
    }
    return;
  }
  if (decimalNum == 1) {
    for (; bitCount > 1; bitCount--) {
      os << 0;
    }
    os << 1;
  } else {
    PrintDec2BinNix(decimalNum / 2, bitCount - 1, os);
    os << decimalNum % 2;
  }
}

void NixVector::SetEpoch(uint32_t epoch) { m_epoch = epoch; }

uint32_t NixVector::GetEpoch() const { return m_epoch; }

} // namespace ns3
