

#include "packet-tag-list.h"

#include "tag-buffer.h"
#include "tag.h"

#include "ns3/fatal-error.h"
#include "ns3/log.h"

#include <cstring>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PacketTagList");

PacketTagList::TagData *PacketTagList::CreateTagData(size_t dataSize) {
  NS_ASSERT_MSG(dataSize < std::numeric_limits<decltype(TagData::size)>::max(),
                "Requested TagData size "
                    << dataSize << " exceeds maximum "
                    << std::numeric_limits<decltype(TagData::size)>::max());

  void *p = std::malloc(sizeof(TagData) + dataSize - 1);

  auto tag = new (p) TagData;
  tag->size = dataSize;
  return tag;
}

bool PacketTagList::COWTraverse(Tag &tag, PacketTagList::COWWriter Writer) {
  TypeId tid = tag.GetInstanceTypeId();
  NS_LOG_FUNCTION(this << tid);
  NS_LOG_INFO("looking for " << tid);

  if (m_next == nullptr) {
    return false;
  }

  bool found = false;

  TagData **prevNext = &m_next;
  TagData *cur = m_next;
  TagData *it = nullptr;

  while (cur != nullptr) {
    if (cur->count > 1) {
      NS_LOG_INFO("found initial merge before tid");
      break;
    } else if (cur->tid == tid) {
      NS_LOG_INFO("found tid before initial merge, calling writer");
      found = (this->*Writer)(tag, true, cur, prevNext);
      break;
    } else {
      prevNext = &cur->next;
      cur = cur->next;
    }
  }

  if (cur == nullptr || found) {
    NS_LOG_INFO("returning after header with found: " << found);
    return found;
  }

  for (it = cur; it != nullptr; it = it->next) {
    if (it->tid == tid) {
      break;
    }
  }
  if (it == nullptr) {
    NS_LOG_INFO("tid not found after first merge");
    return found;
  }

  NS_ASSERT(cur != nullptr);
  NS_ASSERT(cur->count > 1);

  while (cur->tid != tid) {
    NS_ASSERT(cur != nullptr);
    NS_ASSERT(cur->count > 1);
    cur->count--;
    TagData *copy = CreateTagData(cur->size);
    copy->tid = cur->tid;
    copy->count = 1;
    copy->size = cur->size;
    memcpy(copy->data, cur->data, copy->size);
    copy->next = cur->next;
    copy->next->count++;
    *prevNext = copy;
    prevNext = &copy->next;
    cur = copy->next;
  }
  NS_ASSERT(cur != nullptr);
  NS_ASSERT(cur->tid == tid);
  NS_ASSERT(cur->count > 1);

  found = (this->*Writer)(tag, false, cur, prevNext);
  return found;
}

bool PacketTagList::Remove(Tag &tag) {
  return COWTraverse(tag, &PacketTagList::RemoveWriter);
}

bool PacketTagList::RemoveWriter(Tag &tag, bool preMerge,
                                 PacketTagList::TagData *cur,
                                 PacketTagList::TagData **prevNext) {
  NS_LOG_FUNCTION_NOARGS();

  bool found = true;
  tag.Deserialize(TagBuffer(cur->data, cur->data + cur->size));
  *prevNext = cur->next;

  if (preMerge) {
    cur->~TagData();
    std::free(cur);
  } else {
    cur->count--;
    if (cur->next != nullptr) {
      cur->next->count++;
    }
  }
  return found;
}

bool PacketTagList::Replace(Tag &tag) {
  bool found = COWTraverse(tag, &PacketTagList::ReplaceWriter);
  if (!found) {
    Add(tag);
  }
  return found;
}

bool PacketTagList::ReplaceWriter(Tag &tag, bool preMerge,
                                  PacketTagList::TagData *cur,
                                  PacketTagList::TagData **prevNext) {
  NS_LOG_FUNCTION_NOARGS();

  bool found = true;
  if (preMerge) {
    tag.Serialize(TagBuffer(cur->data, cur->data + cur->size));
  } else {
    cur->count--;
    TagData *copy = CreateTagData(tag.GetSerializedSize());
    copy->tid = tag.GetInstanceTypeId();
    copy->count = 1;
    tag.Serialize(TagBuffer(copy->data, copy->data + copy->size));
    copy->next = cur->next;
    if (copy->next != nullptr) {
      copy->next->count++;
    }
    *prevNext = copy;
  }
  return found;
}

void PacketTagList::Add(const Tag &tag) const {
  NS_LOG_FUNCTION(this << tag.GetInstanceTypeId());
  for (TagData *cur = m_next; cur != nullptr; cur = cur->next) {
    NS_ASSERT_MSG(cur->tid != tag.GetInstanceTypeId(),
                  "Error: cannot add the same kind of tag twice.");
  }
  TagData *head = CreateTagData(tag.GetSerializedSize());
  head->count = 1;
  head->next = nullptr;
  head->tid = tag.GetInstanceTypeId();
  head->next = m_next;
  tag.Serialize(TagBuffer(head->data, head->data + head->size));

  const_cast<PacketTagList *>(this)->m_next = head;
}

bool PacketTagList::Peek(Tag &tag) const {
  NS_LOG_FUNCTION(this << tag.GetInstanceTypeId());
  TypeId tid = tag.GetInstanceTypeId();
  for (TagData *cur = m_next; cur != nullptr; cur = cur->next) {
    if (cur->tid == tid) {
      tag.Deserialize(TagBuffer(cur->data, cur->data + cur->size));
      return true;
    }
  }
  return false;
}

const PacketTagList::TagData *PacketTagList::Head() const { return m_next; }

uint32_t PacketTagList::GetSerializedSize() const {
  NS_LOG_FUNCTION_NOARGS();

  uint32_t size = 0;

  size = 4;

  for (TagData *cur = m_next; cur != nullptr; cur = cur->next) {
    size += 4;

    uint32_t hashSize = (sizeof(TypeId::hash_t) + 3) & (~3);
    size += hashSize;

    uint32_t tagWordSize = (cur->size + 3) & (~3);
    size += tagWordSize;
  }

  return size;
}

uint32_t PacketTagList::Serialize(uint32_t *buffer, uint32_t maxSize) const {
  NS_LOG_FUNCTION(this << buffer << maxSize);

  uint32_t *p = buffer;
  uint32_t size = 0;

  uint32_t *numberOfTags = nullptr;

  if (size + 4 <= maxSize) {
    numberOfTags = p;
    *p++ = 0;
    size += 4;
  } else {
    return 0;
  }

  for (TagData *cur = m_next; cur != nullptr; cur = cur->next) {
    if (size + 4 <= maxSize) {
      *p++ = cur->size;
      size += 4;
    } else {
      return 0;
    }

    NS_LOG_INFO("Serializing tag id " << cur->tid);

    uint32_t hashSize = (sizeof(TypeId::hash_t) + 3) & (~3);
    if (size + hashSize <= maxSize) {
      TypeId::hash_t tid = cur->tid.GetHash();
      memcpy(p, &tid, sizeof(TypeId::hash_t));
      p += hashSize / 4;
      size += hashSize;
    } else {
      return 0;
    }

    uint32_t tagWordSize = (cur->size + 3) & (~3);
    if (size + tagWordSize <= maxSize) {
      memcpy(p, cur->data, cur->size);
      size += tagWordSize;
      p += tagWordSize / 4;
    } else {
      return 0;
    }

    (*numberOfTags)++;
  }

  return 1;
}

uint32_t PacketTagList::Deserialize(const uint32_t *buffer, uint32_t size) {
  NS_LOG_FUNCTION(this << buffer << size);
  const uint32_t *p = buffer;
  uint32_t sizeCheck = size - 4;

  NS_ASSERT(sizeCheck >= 4);
  uint32_t numberOfTags = *p++;
  sizeCheck -= 4;

  NS_LOG_INFO("Deserializing number of tags " << numberOfTags);

  TagData *prevTag = nullptr;
  for (uint32_t i = 0; i < numberOfTags; ++i) {
    NS_ASSERT(sizeCheck >= 4);
    uint32_t tagSize = *p++;
    sizeCheck -= 4;

    uint32_t hashSize = (sizeof(TypeId::hash_t) + 3) & (~3);
    NS_ASSERT(sizeCheck >= hashSize);
    TypeId::hash_t hash;
    memcpy(&hash, p, sizeof(TypeId::hash_t));
    p += hashSize / 4;
    sizeCheck -= hashSize;

    TypeId tid = TypeId::LookupByHash(hash);

    NS_LOG_INFO("Deserializing tag of type " << tid);

    TagData *newTag = CreateTagData(tagSize);
    newTag->count = 1;
    newTag->next = nullptr;
    newTag->tid = tid;

    NS_ASSERT(sizeCheck >= tagSize);
    memcpy(newTag->data, p, tagSize);

    uint32_t tagWordSize = (tagSize + 3) & (~3);
    p += tagWordSize / 4;
    sizeCheck -= tagWordSize;

    if (i == 0) {
      m_next = newTag;
    } else {
      prevTag->next = newTag;
    }

    prevTag = newTag;
  }

  NS_ASSERT(sizeCheck == 0);

  return (sizeCheck != 0) ? 0 : 1;
}

} // namespace ns3
