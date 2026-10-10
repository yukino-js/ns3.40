#ifndef PACKET_TAG_LIST_H
#define PACKET_TAG_LIST_H

#include "ns3/atomic-counter.h"
#include "ns3/type-id.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Tag;

class PacketTagList {
public:
  struct TagData {
    TagData *next;
#ifdef NS3_MTP
    AtomicCounter count;
#else
    uint32_t count;
#endif
    TypeId tid;
    uint32_t size;
    uint8_t data[1];
  };

  inline PacketTagList();
  inline PacketTagList(const PacketTagList &o);
  inline PacketTagList &operator=(const PacketTagList &o);
  inline ~PacketTagList();

  void Add(const Tag &tag) const;
  bool Remove(Tag &tag);
  bool Replace(Tag &tag);
  bool Peek(Tag &tag) const;
  inline void RemoveAll();
  const PacketTagList::TagData *Head() const;
  uint32_t GetSerializedSize() const;
  uint32_t Serialize(uint32_t *buffer, uint32_t maxSize) const;
  uint32_t Deserialize(const uint32_t *buffer, uint32_t size);

private:
  static TagData *CreateTagData(size_t dataSize);

  typedef bool (PacketTagList::*COWWriter)(Tag &tag, bool preMerge,
                                           TagData *cur, TagData **prevNext);
  bool COWTraverse(Tag &tag, PacketTagList::COWWriter Writer);
  bool RemoveWriter(Tag &tag, bool preMerge, TagData *cur, TagData **prevNext);
  bool ReplaceWriter(Tag &tag, bool preMerge, TagData *cur, TagData **prevNext);

  TagData *m_next;
};

} // namespace ns3

namespace ns3 {

PacketTagList::PacketTagList() : m_next() {}

PacketTagList::PacketTagList(const PacketTagList &o) : m_next(o.m_next) {
  if (m_next != nullptr) {
    m_next->count++;
  }
}

PacketTagList &PacketTagList::operator=(const PacketTagList &o) {
  if (m_next == o.m_next) {
    return *this;
  }
  RemoveAll();
  m_next = o.m_next;
  if (m_next != nullptr) {
    m_next->count++;
  }
  return *this;
}

PacketTagList::~PacketTagList() { RemoveAll(); }

void PacketTagList::RemoveAll() {
  TagData *prev = nullptr;
  for (TagData *cur = m_next; cur != nullptr; cur = cur->next) {
    if (cur->count-- > 1) {
      break;
    }
#ifdef NS3_MTP
    std::atomic_thread_fence(std::memory_order_acquire);
#endif
    if (prev != nullptr) {
      prev->~TagData();
      std::free(prev);
    }
    prev = cur;
  }
  if (prev != nullptr) {
    prev->~TagData();
    std::free(prev);
  }
  m_next = nullptr;
}

} // namespace ns3

#endif
