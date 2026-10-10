#ifndef BYTE_TAG_LIST_H
#define BYTE_TAG_LIST_H

#define __STDC_LIMIT_MACROS
#include "tag-buffer.h"

#include "ns3/type-id.h"

#include <stdint.h>

namespace ns3 {

struct ByteTagListData;

class ByteTagList {
public:
  class Iterator {
  public:
    struct Item {
      TypeId tid;
      uint32_t size;
      int32_t start;
      int32_t end;
      TagBuffer buf;

      Item(TagBuffer buf);

    private:
      friend class ByteTagList;
      friend class ByteTagList::Iterator;
    };

    bool HasNext() const;

    ByteTagList::Iterator::Item Next();

    uint32_t GetOffsetStart() const;

  private:
    friend class ByteTagList;

    Iterator(uint8_t *start, uint8_t *end, int32_t offsetStart,
             int32_t offsetEnd, int32_t adjustment);

    void PrepareForNext();
    uint8_t *m_current;
    uint8_t *m_end;
    int32_t m_offsetStart;
    int32_t m_offsetEnd;
    int32_t m_adjustment;
    uint32_t m_nextTid;
    uint32_t m_nextSize;
    int32_t m_nextStart;
    int32_t m_nextEnd;
  };

  ByteTagList();

  ByteTagList(const ByteTagList &o);

  ByteTagList &operator=(const ByteTagList &o);
  ~ByteTagList();

  TagBuffer Add(TypeId tid, uint32_t bufferSize, int32_t start, int32_t end);

  void Add(const ByteTagList &o);

  void RemoveAll();

  ByteTagList::Iterator Begin(int32_t offsetStart, int32_t offsetEnd) const;

  inline void Adjust(int32_t adjustment);

  void AddAtEnd(int32_t appendOffset);
  void AddAtStart(int32_t prependOffset);
  uint32_t GetSerializedSize() const;
  uint32_t Serialize(uint32_t *buffer, uint32_t maxSize) const;
  uint32_t Deserialize(const uint32_t *buffer, uint32_t size);

private:
  ByteTagList::Iterator BeginAll() const;

  ByteTagListData *Allocate(uint32_t size);

  void Deallocate(ByteTagListData *data);

  int32_t m_minStart;
  int32_t m_maxEnd;
  int32_t m_adjustment;
  uint32_t m_used;
  ByteTagListData *m_data;
};

void ByteTagList::Adjust(int32_t adjustment) { m_adjustment += adjustment; }

} // namespace ns3

#endif
