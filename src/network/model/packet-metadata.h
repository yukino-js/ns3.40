#ifndef PACKET_METADATA_H
#define PACKET_METADATA_H

#include "buffer.h"

#include "ns3/assert.h"
#include "ns3/atomic-counter.h"
#include "ns3/callback.h"
#include "ns3/type-id.h"

#include <limits>
#include <stdint.h>
#include <vector>

namespace ns3 {

class Chunk;
class Buffer;
class Header;
class Trailer;

class PacketMetadata {
public:
  struct Item {
    enum ItemType { PAYLOAD, HEADER, TRAILER };

    ItemType type;

    bool isFragment;
    TypeId tid;
    uint32_t currentSize;
    uint32_t currentTrimmedFromStart;
    uint32_t currentTrimmedFromEnd;
    Buffer::Iterator current;
  };

  class ItemIterator {
  public:
    ItemIterator(const PacketMetadata *metadata, Buffer buffer);
    bool HasNext() const;
    Item Next();

  private:
    const PacketMetadata *m_metadata;
    Buffer m_buffer;
    uint16_t m_current;
    uint32_t m_offset;
    bool m_hasReadTail;
  };

  static void Enable();
  static void EnableChecking();

  inline PacketMetadata(uint64_t uid, uint32_t size);
  inline PacketMetadata(const PacketMetadata &o);
  inline PacketMetadata &operator=(const PacketMetadata &o);
  inline ~PacketMetadata();

  PacketMetadata() = delete;

  void AddHeader(const Header &header, uint32_t size);
  void RemoveHeader(const Header &header, uint32_t size);

  void AddTrailer(const Trailer &trailer, uint32_t size);
  void RemoveTrailer(const Trailer &trailer, uint32_t size);

  PacketMetadata CreateFragment(uint32_t start, uint32_t end) const;

  void AddAtEnd(const PacketMetadata &o);
  void AddPaddingAtEnd(uint32_t end);
  void RemoveAtStart(uint32_t start);
  void RemoveAtEnd(uint32_t end);

  uint64_t GetUid() const;

  uint32_t GetSerializedSize() const;

  ItemIterator BeginItem(Buffer buffer) const;

  uint32_t Serialize(uint8_t *buffer, uint32_t maxSize) const;
  uint32_t Deserialize(const uint8_t *buffer, uint32_t size);

private:
  static uint8_t *AddToRawU8(const uint8_t &data, uint8_t *start,
                             uint8_t *current, uint32_t maxSize);

  static uint8_t *AddToRawU16(const uint16_t &data, uint8_t *start,
                              uint8_t *current, uint32_t maxSize);

  static uint8_t *AddToRawU32(const uint32_t &data, uint8_t *start,
                              uint8_t *current, uint32_t maxSize);

  static uint8_t *AddToRawU64(const uint64_t &data, uint8_t *start,
                              uint8_t *current, uint32_t maxSize);

  static uint8_t *AddToRaw(const uint8_t *data, uint32_t dataSize,
                           uint8_t *start, uint8_t *current, uint32_t maxSize);

  static uint8_t *ReadFromRawU8(uint8_t &data, const uint8_t *start,
                                const uint8_t *current, uint32_t maxSize);

  static uint8_t *ReadFromRawU16(uint16_t &data, const uint8_t *start,
                                 const uint8_t *current, uint32_t maxSize);

  static uint8_t *ReadFromRawU32(uint32_t &data, const uint8_t *start,
                                 const uint8_t *current, uint32_t maxSize);

  static uint8_t *ReadFromRawU64(uint64_t &data, const uint8_t *start,
                                 const uint8_t *current, uint32_t maxSize);

#define PACKET_METADATA_DATA_M_DATA_SIZE 8

  struct Data {
#ifdef NS3_MTP
    AtomicCounter m_count;
#else
    uint32_t m_count;
#endif
    uint16_t m_size;
    uint16_t m_dirtyEnd;
    uint8_t m_data[PACKET_METADATA_DATA_M_DATA_SIZE];
  };

  struct SmallItem {
    uint16_t next;
    uint16_t prev;
    uint32_t typeUid;
    uint32_t size;
    uint16_t chunkUid;
  };

  struct ExtraItem {
    uint32_t fragmentStart;
    uint32_t fragmentEnd;
    uint64_t packetUid;
  };

  class DataFreeList : public std::vector<Data *> {
  public:
    ~DataFreeList();
  };

  friend DataFreeList::~DataFreeList();
  friend class ItemIterator;

  inline uint16_t AddSmall(const PacketMetadata::SmallItem *item);
  uint16_t AddBig(uint32_t head, uint32_t tail,
                  const PacketMetadata::SmallItem *item,
                  const PacketMetadata::ExtraItem *extraItem);
  void ReplaceTail(PacketMetadata::SmallItem *item,
                   PacketMetadata::ExtraItem *extraItem, uint32_t available);
  inline void UpdateHead(uint16_t written);
  inline void UpdateTail(uint16_t written);

  inline uint32_t GetUleb128Size(uint32_t value) const;
  uint32_t ReadUleb128(const uint8_t **pBuffer) const;
  inline void Append16(uint16_t value, uint8_t *buffer);
  inline void Append32(uint32_t value, uint8_t *buffer);
  inline void AppendValue(uint32_t value, uint8_t *buffer);
  void AppendValueExtra(uint32_t value, uint8_t *buffer);

  inline void Reserve(uint32_t n);
  void ReserveCopy(uint32_t n);

  uint32_t GetTotalSize() const;

  uint32_t ReadItems(uint16_t current, PacketMetadata::SmallItem *item,
                     PacketMetadata::ExtraItem *extraItem) const;
  void DoAddHeader(uint32_t uid, uint32_t size);
  bool IsStateOk() const;
  bool IsPointerOk(uint16_t pointer) const;
  bool IsSharedPointerOk(uint16_t pointer) const;

  static void Recycle(PacketMetadata::Data *data);
  static PacketMetadata::Data *Create(uint32_t size);
  static PacketMetadata::Data *Allocate(uint32_t n);
  static void Deallocate(PacketMetadata::Data *data);

#ifdef NS3_MTP
  static std::atomic<bool> m_freeListUsing;
#endif
  static DataFreeList m_freeList;
  static bool m_enable;
  static bool m_enableChecking;

  static bool m_metadataSkipped;

  static uint32_t m_maxSize;
  static uint16_t m_chunkUid;

  Data *m_data;
  uint16_t m_head;
  uint16_t m_tail;
  uint16_t m_used;
  uint64_t m_packetUid;
};

} // namespace ns3

namespace ns3 {

PacketMetadata::PacketMetadata(uint64_t uid, uint32_t size)
    : m_data(PacketMetadata::Create(10)), m_head(0xffff), m_tail(0xffff),
      m_used(0), m_packetUid(uid) {
  memset(m_data->m_data, 0xff, 4);
  if (size > 0) {
    DoAddHeader(0, size);
  }
}

PacketMetadata::PacketMetadata(const PacketMetadata &o)
    : m_data(o.m_data), m_head(o.m_head), m_tail(o.m_tail), m_used(o.m_used),
      m_packetUid(o.m_packetUid) {
  NS_ASSERT(m_data != nullptr);
  NS_ASSERT(m_data->m_count < std::numeric_limits<uint32_t>::max());
  m_data->m_count++;
}

PacketMetadata &PacketMetadata::operator=(const PacketMetadata &o) {
  if (m_data != o.m_data) {
    NS_ASSERT(m_data != nullptr);
    if (m_data->m_count-- == 1) {
      PacketMetadata::Recycle(m_data);
    }
    m_data = o.m_data;
    NS_ASSERT(m_data != nullptr);
    m_data->m_count++;
  }
  m_head = o.m_head;
  m_tail = o.m_tail;
  m_used = o.m_used;
  m_packetUid = o.m_packetUid;
  return *this;
}

PacketMetadata::~PacketMetadata() {
  NS_ASSERT(m_data != nullptr);
  if (m_data->m_count-- == 1) {
    PacketMetadata::Recycle(m_data);
  }
}

} // namespace ns3

#endif
