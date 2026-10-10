#ifndef BUFFER_H
#define BUFFER_H

#include "ns3/assert.h"
#include "ns3/atomic-counter.h"

#include <ostream>
#include <stdint.h>
#include <vector>

#ifndef NS3_MTP
#define BUFFER_FREE_LIST 1
#endif

namespace ns3 {

class Buffer {
public:
  class Iterator {
  public:
    inline Iterator();
    inline void Next();
    inline void Prev();
    inline void Next(uint32_t delta);
    inline void Prev(uint32_t delta);
    uint32_t GetDistanceFrom(const Iterator &o) const;

    bool IsEnd() const;
    bool IsStart() const;

    inline void WriteU8(uint8_t data);
    inline void WriteU8(uint8_t data, uint32_t len);
    void WriteU16(uint16_t data);
    void WriteU32(uint32_t data);
    void WriteU64(uint64_t data);
    void WriteHtolsbU16(uint16_t data);
    void WriteHtolsbU32(uint32_t data);
    void WriteHtolsbU64(uint64_t data);
    inline void WriteHtonU16(uint16_t data);
    inline void WriteHtonU32(uint32_t data);
    void WriteHtonU64(uint64_t data);
    void Write(const uint8_t *buffer, uint32_t size);
    void Write(Iterator start, Iterator end);

    inline uint8_t PeekU8();

    inline uint8_t ReadU8();
    inline uint16_t ReadU16();
    uint32_t ReadU32();
    uint64_t ReadU64();
    inline uint16_t ReadNtohU16();
    inline uint32_t ReadNtohU32();
    uint64_t ReadNtohU64();
    uint16_t ReadLsbtohU16();
    uint32_t ReadLsbtohU32();
    uint64_t ReadLsbtohU64();
    void Read(uint8_t *buffer, uint32_t size);

    inline void Read(Iterator start, uint32_t size);

    uint16_t CalculateIpChecksum(uint16_t size);

    uint16_t CalculateIpChecksum(uint16_t size, uint32_t initialChecksum);

    uint32_t GetSize() const;

    uint32_t GetRemainingSize() const;

  private:
    friend class Buffer;
    inline Iterator(const Buffer *buffer);
    inline Iterator(const Buffer *buffer, bool dummy);
    inline void Construct(const Buffer *buffer);
    bool CheckNoZero(uint32_t start, uint32_t end) const;
    bool Check(uint32_t i) const;
    uint16_t SlowReadNtohU16();
    uint32_t SlowReadNtohU32();
    std::string GetReadErrorMessage() const;
    std::string GetWriteErrorMessage() const;

    uint32_t m_zeroStart;
    uint32_t m_zeroEnd;
    uint32_t m_dataStart;
    uint32_t m_dataEnd;
    uint32_t m_current;
    uint8_t *m_data;
  };

  inline uint32_t GetSize() const;

  const uint8_t *PeekData() const;

  void AddAtStart(uint32_t start);
  void AddAtEnd(uint32_t end);

  void AddAtEnd(const Buffer &o);
  void RemoveAtStart(uint32_t start);
  void RemoveAtEnd(uint32_t end);

  Buffer CreateFragment(uint32_t start, uint32_t length) const;

  inline Buffer::Iterator Begin() const;
  inline Buffer::Iterator End() const;

  uint32_t GetSerializedSize() const;

  uint32_t Serialize(uint8_t *buffer, uint32_t maxSize) const;

  uint32_t Deserialize(const uint8_t *buffer, uint32_t size);

  void CopyData(std::ostream *os, uint32_t size) const;

  uint32_t CopyData(uint8_t *buffer, uint32_t size) const;

  inline Buffer(const Buffer &o);
  Buffer &operator=(const Buffer &o);
  Buffer();
  Buffer(uint32_t dataSize);
  Buffer(uint32_t dataSize, bool initialize);
  ~Buffer();

private:
  struct Data {
#ifdef NS3_MTP
    AtomicCounter m_count;
#else
    uint32_t m_count;
#endif
    uint32_t m_size;
    uint32_t m_dirtyStart;
    uint32_t m_dirtyEnd;
    uint8_t m_data[1];
  };

  Buffer CreateFullCopy() const;

  void TransformIntoRealBuffer() const;
  bool CheckInternalState() const;

  void Initialize(uint32_t zeroSize);

  uint32_t GetInternalSize() const;

  uint32_t GetInternalEnd() const;

  static void Recycle(Buffer::Data *data);
  static Buffer::Data *Create(uint32_t size);
  static Buffer::Data *Allocate(uint32_t reqSize);
  static void Deallocate(Buffer::Data *data);

  Data *m_data;

  uint32_t m_maxZeroAreaStart;
  static uint32_t g_recommendedStart;

  uint32_t m_zeroAreaStart;
  uint32_t m_zeroAreaEnd;
  uint32_t m_start;
  uint32_t m_end;

#ifdef BUFFER_FREE_LIST
  typedef std::vector<Buffer::Data *> FreeList;

  struct LocalStaticDestructor {
    ~LocalStaticDestructor();
  };

  static uint32_t g_maxSize;
  static FreeList *g_freeList;
  static LocalStaticDestructor g_localStaticDestructor;
#endif
};

} // namespace ns3

#include "ns3/assert.h"

#include <cstring>

namespace ns3 {

Buffer::Iterator::Iterator()
    : m_zeroStart(0), m_zeroEnd(0), m_dataStart(0), m_dataEnd(0), m_current(0),
      m_data(nullptr) {}

Buffer::Iterator::Iterator(const Buffer *buffer) {
  Construct(buffer);
  m_current = m_dataStart;
}

Buffer::Iterator::Iterator(const Buffer *buffer, bool dummy) {
  Construct(buffer);
  m_current = m_dataEnd;
}

void Buffer::Iterator::Construct(const Buffer *buffer) {
  m_zeroStart = buffer->m_zeroAreaStart;
  m_zeroEnd = buffer->m_zeroAreaEnd;
  m_dataStart = buffer->m_start;
  m_dataEnd = buffer->m_end;
  m_data = buffer->m_data->m_data;
}

void Buffer::Iterator::Next() {
  NS_ASSERT(m_current + 1 <= m_dataEnd);
  m_current++;
}

void Buffer::Iterator::Prev() {
  NS_ASSERT(m_current >= 1);
  m_current--;
}

void Buffer::Iterator::Next(uint32_t delta) {
  NS_ASSERT(m_current + delta <= m_dataEnd);
  m_current += delta;
}

void Buffer::Iterator::Prev(uint32_t delta) {
  NS_ASSERT(m_current >= delta);
  m_current -= delta;
}

void Buffer::Iterator::WriteU8(uint8_t data) {
  NS_ASSERT_MSG(Check(m_current), GetWriteErrorMessage());

  if (m_current < m_zeroStart) {
    m_data[m_current] = data;
    m_current++;
  } else {
    m_data[m_current - (m_zeroEnd - m_zeroStart)] = data;
    m_current++;
  }
}

void Buffer::Iterator::WriteU8(uint8_t data, uint32_t len) {
  NS_ASSERT_MSG(CheckNoZero(m_current, m_current + len),
                GetWriteErrorMessage());
  if (m_current <= m_zeroStart) {
    std::memset(&(m_data[m_current]), data, len);
    m_current += len;
  } else {
    uint8_t *buffer = &m_data[m_current - (m_zeroEnd - m_zeroStart)];
    std::memset(buffer, data, len);
    m_current += len;
  }
}

void Buffer::Iterator::WriteHtonU16(uint16_t data) {
  NS_ASSERT_MSG(CheckNoZero(m_current, m_current + 2), GetWriteErrorMessage());
  uint8_t *buffer;
  if (m_current + 2 <= m_zeroStart) {
    buffer = &m_data[m_current];
  } else {
    buffer = &m_data[m_current - (m_zeroEnd - m_zeroStart)];
  }
  buffer[0] = (data >> 8) & 0xff;
  buffer[1] = (data >> 0) & 0xff;
  m_current += 2;
}

void Buffer::Iterator::WriteHtonU32(uint32_t data) {
  NS_ASSERT_MSG(CheckNoZero(m_current, m_current + 4), GetWriteErrorMessage());

  uint8_t *buffer;
  if (m_current + 4 <= m_zeroStart) {
    buffer = &m_data[m_current];
  } else {
    buffer = &m_data[m_current - (m_zeroEnd - m_zeroStart)];
  }
  buffer[0] = (data >> 24) & 0xff;
  buffer[1] = (data >> 16) & 0xff;
  buffer[2] = (data >> 8) & 0xff;
  buffer[3] = (data >> 0) & 0xff;
  m_current += 4;
}

uint16_t Buffer::Iterator::ReadNtohU16() {
  uint8_t *buffer;
  if (m_current + 2 <= m_zeroStart) {
    buffer = &m_data[m_current];
  } else if (m_current >= m_zeroEnd) {
    buffer = &m_data[m_current - (m_zeroEnd - m_zeroStart)];
  } else {
    return SlowReadNtohU16();
  }
  uint16_t retval = 0;
  retval |= buffer[0];
  retval <<= 8;
  retval |= buffer[1];
  m_current += 2;
  return retval;
}

uint32_t Buffer::Iterator::ReadNtohU32() {
  uint8_t *buffer;
  if (m_current + 4 <= m_zeroStart) {
    buffer = &m_data[m_current];
  } else if (m_current >= m_zeroEnd) {
    buffer = &m_data[m_current - (m_zeroEnd - m_zeroStart)];
  } else {
    return SlowReadNtohU32();
  }
  uint32_t retval = 0;
  retval |= buffer[0];
  retval <<= 8;
  retval |= buffer[1];
  retval <<= 8;
  retval |= buffer[2];
  retval <<= 8;
  retval |= buffer[3];
  m_current += 4;
  return retval;
}

uint8_t Buffer::Iterator::PeekU8() {
  NS_ASSERT_MSG(m_current >= m_dataStart && m_current < m_dataEnd,
                GetReadErrorMessage());

  if (m_current < m_zeroStart) {
    uint8_t data = m_data[m_current];
    return data;
  } else if (m_current < m_zeroEnd) {
    return 0;
  } else {
    uint8_t data = m_data[m_current - (m_zeroEnd - m_zeroStart)];
    return data;
  }
}

uint8_t Buffer::Iterator::ReadU8() {
  uint8_t ret = PeekU8();
  m_current++;
  return ret;
}

uint16_t Buffer::Iterator::ReadU16() {
  uint8_t byte0 = ReadU8();
  uint8_t byte1 = ReadU8();
  uint16_t data = byte1;
  data <<= 8;
  data |= byte0;

  return data;
}

void Buffer::Iterator::Read(Buffer::Iterator start, uint32_t size) {
  Buffer::Iterator end = *this;
  end.Next(size);

  start.Write(*this, end);
}

Buffer::Buffer(const Buffer &o)
    : m_data(o.m_data), m_maxZeroAreaStart(o.m_zeroAreaStart),
      m_zeroAreaStart(o.m_zeroAreaStart), m_zeroAreaEnd(o.m_zeroAreaEnd),
      m_start(o.m_start), m_end(o.m_end) {
  m_data->m_count++;
  NS_ASSERT(CheckInternalState());
}

uint32_t Buffer::GetSize() const { return m_end - m_start; }

Buffer::Iterator Buffer::Begin() const {
  NS_ASSERT(CheckInternalState());
  return Buffer::Iterator(this);
}

Buffer::Iterator Buffer::End() const {
  NS_ASSERT(CheckInternalState());
  return Buffer::Iterator(this, false);
}

} // namespace ns3

#endif
