#ifndef TAG_BUFFER_H
#define TAG_BUFFER_H

#include <stdint.h>

#define TAG_BUFFER_USE_INLINE 1

#ifdef TAG_BUFFER_USE_INLINE
#define TAG_BUFFER_INLINE inline
#else
#define TAG_BUFFER_INLINE
#endif

namespace ns3 {

class TagBuffer {
public:
  TagBuffer(uint8_t *start, uint8_t *end);

  void TrimAtEnd(uint32_t trim);

  void CopyFrom(TagBuffer o);

  TAG_BUFFER_INLINE void WriteU8(uint8_t v);
  TAG_BUFFER_INLINE void WriteU16(uint16_t v);
  TAG_BUFFER_INLINE void WriteU32(uint32_t v);
  void WriteU64(uint64_t v);
  void WriteDouble(double v);
  void Write(const uint8_t *buffer, uint32_t size);
  TAG_BUFFER_INLINE uint8_t ReadU8();
  TAG_BUFFER_INLINE uint16_t ReadU16();
  TAG_BUFFER_INLINE uint32_t ReadU32();
  uint64_t ReadU64();
  double ReadDouble();
  void Read(uint8_t *buffer, uint32_t size);

private:
  uint8_t *m_current;
  uint8_t *m_end;
};

} // namespace ns3

#ifdef TAG_BUFFER_USE_INLINE

#include "ns3/assert.h"

namespace ns3 {

void TagBuffer::WriteU8(uint8_t v) {
  NS_ASSERT(m_current + 1 <= m_end);
  *m_current = v;
  m_current++;
}

void TagBuffer::WriteU16(uint16_t data) {
  WriteU8((data >> 0) & 0xff);
  WriteU8((data >> 8) & 0xff);
}

void TagBuffer::WriteU32(uint32_t data) {
  WriteU8((data >> 0) & 0xff);
  WriteU8((data >> 8) & 0xff);
  WriteU8((data >> 16) & 0xff);
  WriteU8((data >> 24) & 0xff);
}

uint8_t TagBuffer::ReadU8() {
  NS_ASSERT(m_current + 1 <= m_end);
  uint8_t v;
  v = *m_current;
  m_current++;
  return v;
}

uint16_t TagBuffer::ReadU16() {
  uint8_t byte0 = ReadU8();
  uint8_t byte1 = ReadU8();
  uint16_t data = byte1;
  data <<= 8;
  data |= byte0;
  return data;
}

uint32_t TagBuffer::ReadU32() {
  uint8_t byte0 = ReadU8();
  uint8_t byte1 = ReadU8();
  uint8_t byte2 = ReadU8();
  uint8_t byte3 = ReadU8();
  uint32_t data = byte3;
  data <<= 8;
  data |= byte2;
  data <<= 8;
  data |= byte1;
  data <<= 8;
  data |= byte0;
  return data;
}

} // namespace ns3

#endif

#endif
