#include "wimax-mac-to-mac-header.h"

#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(WimaxMacToMacHeader);

WimaxMacToMacHeader::WimaxMacToMacHeader() : m_len(0) {}

WimaxMacToMacHeader::WimaxMacToMacHeader(uint32_t len) : m_len(len) {}

WimaxMacToMacHeader::~WimaxMacToMacHeader() {}

TypeId WimaxMacToMacHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::WimaxMacToMacHeader")
                          .SetParent<Header>()
                          .SetGroupName("Wimax")
                          .AddConstructor<WimaxMacToMacHeader>();
  return tid;
}

TypeId WimaxMacToMacHeader::GetInstanceTypeId() const { return GetTypeId(); }

uint8_t WimaxMacToMacHeader::GetSizeOfLen() const {
  uint8_t sizeOfLen = 1;

  if (m_len > 127) {
    sizeOfLen = 2;
    uint64_t testValue = 0xFF;
    while (m_len > testValue) {
      sizeOfLen++;
      testValue *= 0xFF;
    }
  }
  return sizeOfLen;
}

uint32_t WimaxMacToMacHeader::GetSerializedSize() const {
  uint8_t sizeOfLen = GetSizeOfLen();
  if (sizeOfLen == 1) {
    return 20;
  } else {
    return 20 + sizeOfLen - 1;
  }
}

void WimaxMacToMacHeader::Serialize(Buffer::Iterator i) const {

  uint8_t zero = 0;

  for (int j = 0; j < 12; j++) {
    i.WriteU8(zero);
  }
  i.WriteU16(0xf008);
  i.WriteU16(0x0100);
  i.WriteU16(0x0100);
  i.WriteU8(0x09);
  uint8_t lenSize = GetSizeOfLen();
  if (lenSize == 1) {
    i.WriteU8(m_len);
  } else {
    i.WriteU8((lenSize - 1) | 0x80);
    for (int j = 0; j < lenSize - 1; j++) {
      i.WriteU8((uint8_t)(m_len >> ((lenSize - 1 - 1 - j) * 8)));
    }
  }
}

uint32_t WimaxMacToMacHeader::Deserialize(Buffer::Iterator start) { return 20; }

void WimaxMacToMacHeader::Print(std::ostream &os) const {}
}; // namespace ns3
