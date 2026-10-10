#include "packet.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include <cstdarg>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Packet");

uint32_t Packet::m_globalUid = 0;

TypeId ByteTagIterator::Item::GetTypeId() const { return m_tid; }

uint32_t ByteTagIterator::Item::GetStart() const { return m_start; }

uint32_t ByteTagIterator::Item::GetEnd() const { return m_end; }

void ByteTagIterator::Item::GetTag(Tag &tag) const {
  if (tag.GetInstanceTypeId() != GetTypeId()) {
    NS_FATAL_ERROR("The tag you provided is not of the right type.");
  }
  tag.Deserialize(m_buffer);
}

ByteTagIterator::Item::Item(TypeId tid, uint32_t start, uint32_t end,
                            TagBuffer buffer)
    : m_tid(tid), m_start(start), m_end(end), m_buffer(buffer) {}

bool ByteTagIterator::HasNext() const { return m_current.HasNext(); }

ByteTagIterator::Item ByteTagIterator::Next() {
  ByteTagList::Iterator::Item i = m_current.Next();
  return ByteTagIterator::Item(i.tid, i.start - m_current.GetOffsetStart(),
                               i.end - m_current.GetOffsetStart(), i.buf);
}

ByteTagIterator::ByteTagIterator(ByteTagList::Iterator i) : m_current(i) {}

PacketTagIterator::PacketTagIterator(const PacketTagList::TagData *head)
    : m_current(head) {}

bool PacketTagIterator::HasNext() const { return m_current != nullptr; }

PacketTagIterator::Item PacketTagIterator::Next() {
  NS_ASSERT(HasNext());
  const PacketTagList::TagData *prev = m_current;
  m_current = m_current->next;
  return PacketTagIterator::Item(prev);
}

PacketTagIterator::Item::Item(const PacketTagList::TagData *data)
    : m_data(data) {}

TypeId PacketTagIterator::Item::GetTypeId() const { return m_data->tid; }

void PacketTagIterator::Item::GetTag(Tag &tag) const {
  NS_ASSERT(tag.GetInstanceTypeId() == m_data->tid);
  tag.Deserialize(TagBuffer((uint8_t *)m_data->data,
                            (uint8_t *)m_data->data + m_data->size));
}

Ptr<Packet> Packet::Copy() const {
  return Ptr<Packet>(new Packet(*this), false);
}

Packet::Packet()
    : m_buffer(), m_byteTagList(), m_packetTagList(),
      m_metadata(static_cast<uint64_t>(Simulator::GetSystemId()) << 32 |
                     m_globalUid,
                 0),
      m_nixVector(nullptr) {
  m_globalUid++;
}

Packet::Packet(const Packet &o)
    : m_buffer(o.m_buffer), m_byteTagList(o.m_byteTagList),
      m_packetTagList(o.m_packetTagList), m_metadata(o.m_metadata) {
  o.m_nixVector ? m_nixVector = o.m_nixVector->Copy() : m_nixVector = nullptr;
}

Packet &Packet::operator=(const Packet &o) {
  if (this == &o) {
    return *this;
  }
  m_buffer = o.m_buffer;
  m_byteTagList = o.m_byteTagList;
  m_packetTagList = o.m_packetTagList;
  m_metadata = o.m_metadata;
  o.m_nixVector ? m_nixVector = o.m_nixVector->Copy() : m_nixVector = nullptr;
  return *this;
}

Packet::Packet(uint32_t size)
    : m_buffer(size), m_byteTagList(), m_packetTagList(),
      m_metadata(static_cast<uint64_t>(Simulator::GetSystemId()) << 32 |
                     m_globalUid,
                 size),
      m_nixVector(nullptr) {
  m_globalUid++;
}

Packet::Packet(const uint8_t *buffer, uint32_t size, bool magic)
    : m_buffer(0, false), m_byteTagList(), m_packetTagList(), m_metadata(0, 0),
      m_nixVector(nullptr) {
  NS_ASSERT(magic);
  Deserialize(buffer, size);
}

Packet::Packet(const uint8_t *buffer, uint32_t size)
    : m_buffer(), m_byteTagList(), m_packetTagList(),
      m_metadata(static_cast<uint64_t>(Simulator::GetSystemId()) << 32 |
                     m_globalUid,
                 size),
      m_nixVector(nullptr) {
  m_globalUid++;
  m_buffer.AddAtStart(size);
  Buffer::Iterator i = m_buffer.Begin();
  i.Write(buffer, size);
}

Packet::Packet(const Buffer &buffer, const ByteTagList &byteTagList,
               const PacketTagList &packetTagList,
               const PacketMetadata &metadata)
    : m_buffer(buffer), m_byteTagList(byteTagList),
      m_packetTagList(packetTagList), m_metadata(metadata),
      m_nixVector(nullptr) {}

Ptr<Packet> Packet::CreateFragment(uint32_t start, uint32_t length) const {
  NS_LOG_FUNCTION(this << start << length);
  Buffer buffer = m_buffer.CreateFragment(start, length);
  ByteTagList byteTagList = m_byteTagList;
  byteTagList.Adjust(-start);
  NS_ASSERT(m_buffer.GetSize() >= start + length);
  uint32_t end = m_buffer.GetSize() - (start + length);
  PacketMetadata metadata = m_metadata.CreateFragment(start, end);
  Ptr<Packet> ret = Ptr<Packet>(
      new Packet(buffer, byteTagList, m_packetTagList, metadata), false);
  ret->SetNixVector(GetNixVector());
  return ret;
}

void Packet::SetNixVector(Ptr<NixVector> nixVector) const {
  m_nixVector = nixVector;
}

Ptr<NixVector> Packet::GetNixVector() const { return m_nixVector; }

void Packet::AddHeader(const Header &header) {
  uint32_t size = header.GetSerializedSize();
  NS_LOG_FUNCTION(this << header.GetInstanceTypeId().GetName() << size);
  m_buffer.AddAtStart(size);
  m_byteTagList.Adjust(size);
  m_byteTagList.AddAtStart(size);
  header.Serialize(m_buffer.Begin());
  m_metadata.AddHeader(header, size);
}

uint32_t Packet::RemoveHeader(Header &header, uint32_t size) {
  Buffer::Iterator end;
  end = m_buffer.Begin();
  end.Next(size);
  uint32_t deserialized = header.Deserialize(m_buffer.Begin(), end);
  NS_LOG_FUNCTION(this << header.GetInstanceTypeId().GetName() << deserialized);
  m_buffer.RemoveAtStart(deserialized);
  m_byteTagList.Adjust(-deserialized);
  m_metadata.RemoveHeader(header, deserialized);
  return deserialized;
}

uint32_t Packet::RemoveHeader(Header &header) {
  uint32_t deserialized = header.Deserialize(m_buffer.Begin());
  NS_LOG_FUNCTION(this << header.GetInstanceTypeId().GetName() << deserialized);
  m_buffer.RemoveAtStart(deserialized);
  m_byteTagList.Adjust(-deserialized);
  m_metadata.RemoveHeader(header, deserialized);
  return deserialized;
}

uint32_t Packet::PeekHeader(Header &header) const {
  uint32_t deserialized = header.Deserialize(m_buffer.Begin());
  NS_LOG_FUNCTION(this << header.GetInstanceTypeId().GetName() << deserialized);
  return deserialized;
}

uint32_t Packet::PeekHeader(Header &header, uint32_t size) const {
  Buffer::Iterator end;
  end = m_buffer.Begin();
  end.Next(size);
  uint32_t deserialized = header.Deserialize(m_buffer.Begin(), end);
  NS_LOG_FUNCTION(this << header.GetInstanceTypeId().GetName() << deserialized);
  return deserialized;
}

void Packet::AddTrailer(const Trailer &trailer) {
  uint32_t size = trailer.GetSerializedSize();
  NS_LOG_FUNCTION(this << trailer.GetInstanceTypeId().GetName() << size);
  m_byteTagList.AddAtEnd(GetSize());
  m_buffer.AddAtEnd(size);
  Buffer::Iterator end = m_buffer.End();
  trailer.Serialize(end);
  m_metadata.AddTrailer(trailer, size);
}

uint32_t Packet::RemoveTrailer(Trailer &trailer) {
  uint32_t deserialized = trailer.Deserialize(m_buffer.End());
  NS_LOG_FUNCTION(this << trailer.GetInstanceTypeId().GetName()
                       << deserialized);
  m_buffer.RemoveAtEnd(deserialized);
  m_metadata.RemoveTrailer(trailer, deserialized);
  return deserialized;
}

uint32_t Packet::PeekTrailer(Trailer &trailer) {
  uint32_t deserialized = trailer.Deserialize(m_buffer.End());
  NS_LOG_FUNCTION(this << trailer.GetInstanceTypeId().GetName()
                       << deserialized);
  return deserialized;
}

void Packet::AddAtEnd(Ptr<const Packet> packet) {
  NS_LOG_FUNCTION(this << packet << packet->GetSize());
  m_byteTagList.AddAtEnd(GetSize());
  ByteTagList copy = packet->m_byteTagList;
  copy.AddAtStart(0);
  copy.Adjust(GetSize());
  m_byteTagList.Add(copy);
  m_buffer.AddAtEnd(packet->m_buffer);
  m_metadata.AddAtEnd(packet->m_metadata);
}

void Packet::AddPaddingAtEnd(uint32_t size) {
  NS_LOG_FUNCTION(this << size);
  m_byteTagList.AddAtEnd(GetSize());
  m_buffer.AddAtEnd(size);
  m_metadata.AddPaddingAtEnd(size);
}

void Packet::RemoveAtEnd(uint32_t size) {
  NS_LOG_FUNCTION(this << size);
  m_buffer.RemoveAtEnd(size);
  m_metadata.RemoveAtEnd(size);
}

void Packet::RemoveAtStart(uint32_t size) {
  NS_LOG_FUNCTION(this << size);
  m_buffer.RemoveAtStart(size);
  m_byteTagList.Adjust(-size);
  m_metadata.RemoveAtStart(size);
}

void Packet::RemoveAllByteTags() {
  NS_LOG_FUNCTION(this);
  m_byteTagList.RemoveAll();
}

uint32_t Packet::CopyData(uint8_t *buffer, uint32_t size) const {
  return m_buffer.CopyData(buffer, size);
}

void Packet::CopyData(std::ostream *os, uint32_t size) const {
  return m_buffer.CopyData(os, size);
}

uint64_t Packet::GetUid() const { return m_metadata.GetUid(); }

void Packet::PrintByteTags(std::ostream &os) const {
  ByteTagIterator i = GetByteTagIterator();
  while (i.HasNext()) {
    ByteTagIterator::Item item = i.Next();
    os << item.GetTypeId().GetName() << " [" << item.GetStart() << "-"
       << item.GetEnd() << "]";
    Callback<ObjectBase *> constructor = item.GetTypeId().GetConstructor();
    if (constructor.IsNull()) {
      if (i.HasNext()) {
        os << " ";
      }
      continue;
    }
    Tag *tag = dynamic_cast<Tag *>(constructor());
    NS_ASSERT(tag != nullptr);
    os << " ";
    item.GetTag(*tag);
    tag->Print(os);
    if (i.HasNext()) {
      os << " ";
    }
    delete tag;
  }
}

std::string Packet::ToString() const {
  std::ostringstream oss;
  Print(oss);
  return oss.str();
}

void Packet::Print(std::ostream &os) const {
  PacketMetadata::ItemIterator i = m_metadata.BeginItem(m_buffer);
  while (i.HasNext()) {
    PacketMetadata::Item item = i.Next();
    if (item.isFragment) {
      switch (item.type) {
      case PacketMetadata::Item::PAYLOAD:
        os << "Payload";
        break;
      case PacketMetadata::Item::HEADER:
      case PacketMetadata::Item::TRAILER:
        os << item.tid.GetName();
        break;
      }
      os << " Fragment [" << item.currentTrimmedFromStart << ":"
         << (item.currentTrimmedFromStart + item.currentSize) << "]";
    } else {
      switch (item.type) {
      case PacketMetadata::Item::PAYLOAD:
        os << "Payload (size=" << item.currentSize << ")";
        break;
      case PacketMetadata::Item::HEADER:
      case PacketMetadata::Item::TRAILER:
        os << item.tid.GetName() << " (";
        {
          NS_ASSERT(item.tid.HasConstructor());
          Callback<ObjectBase *> constructor = item.tid.GetConstructor();
          NS_ASSERT(!constructor.IsNull());
          ObjectBase *instance = constructor();
          NS_ASSERT(instance != nullptr);
          auto chunk = dynamic_cast<Chunk *>(instance);
          NS_ASSERT(chunk != nullptr);
          if (item.type == PacketMetadata::Item::HEADER) {
            Buffer::Iterator end = item.current;
            end.Next(item.currentSize);
            chunk->Deserialize(item.current, end);
          } else if (item.type == PacketMetadata::Item::TRAILER) {
            Buffer::Iterator start = item.current;
            start.Prev(item.currentSize);
            chunk->Deserialize(start, item.current);
          } else {
            chunk->Deserialize(item.current);
          }
          chunk->Print(os);
          delete chunk;
        }
        os << ")";
        break;
      }
    }
    if (i.HasNext()) {
      os << " ";
    }
  }
#if 0
  PacketMetadata::ItemIterator i = m_metadata.BeginItem (m_buffer);
  while (i.HasNext ())
    {
      PacketMetadata::Item item = i.Next ();
      if (item.isFragment)
        {
          switch (item.type) {
            case PacketMetadata::Item::PAYLOAD:
              os << "Payload";
              break;
            case PacketMetadata::Item::HEADER:
            case PacketMetadata::Item::TRAILER:
              os << item.tid.GetName ();
              break;
            }
          os << " Fragment [" << item.currentTrimmedFromStart<<":"
             << (item.currentTrimmedFromStart + item.currentSize) << "]";
        }
      else
        {
          switch (item.type) {
            case PacketMetadata::Item::PAYLOAD:
              os << "Payload (size=" << item.currentSize << ")";
              break;
            case PacketMetadata::Item::HEADER:
            case PacketMetadata::Item::TRAILER:
              os << item.tid.GetName () << "(";
              {
                NS_ASSERT (item.tid.HasConstructor ());
                Callback<ObjectBase *> constructor = item.tid.GetConstructor ();
                NS_ASSERT (constructor.IsNull ());
                ObjectBase *instance = constructor ();
                NS_ASSERT (instance != 0);
                Chunk *chunk = dynamic_cast<Chunk *> (instance);
                NS_ASSERT (chunk != 0);
                chunk->Deserialize (item.current);
                for (uint32_t j = 0; j < item.tid.GetAttributeN (); j++)
                  {
                    std::string attrName = item.tid.GetAttributeName (j);
                    std::string value;
                    bool ok = chunk->GetAttribute (attrName, value);
                    NS_ASSERT (ok);
                    os << attrName << "=" << value;
                    if ((j + 1) < item.tid.GetAttributeN ())
                      {
                        os << ",";
                      }
                  }
              }
              os << ")";
              break;
            }
        }
      if (i.HasNext ())
        {
          os << " ";
        }
    }
#endif
}

PacketMetadata::ItemIterator Packet::BeginItem() const {
  return m_metadata.BeginItem(m_buffer);
}

void Packet::EnablePrinting() {
  NS_LOG_FUNCTION_NOARGS();
  PacketMetadata::Enable();
}

void Packet::EnableChecking() {
  NS_LOG_FUNCTION_NOARGS();
  PacketMetadata::EnableChecking();
}

uint32_t Packet::GetSerializedSize() const {
  uint32_t size = 0;

  if (m_nixVector) {
    size += ((m_nixVector->GetSerializedSize() + 3) & (~3));

    size += 4;
  } else {
    size += 4;
  }

  size += ((m_packetTagList.GetSerializedSize() + 3) & (~3));

  size += 4;

  size += ((m_byteTagList.GetSerializedSize() + 3) & (~3));

  size += 4;

  size += ((m_metadata.GetSerializedSize() + 3) & (~3));

  size += 4;

  size += ((m_buffer.GetSerializedSize() + 3) & (~3));

  size += 4;

  return size;
}

uint32_t Packet::Serialize(uint8_t *buffer, uint32_t maxSize) const {
  auto p = reinterpret_cast<uint32_t *>(buffer);
  uint32_t size = 0;

  if (m_nixVector) {
    uint32_t nixSize = m_nixVector->GetSerializedSize();
    if (size + nixSize <= maxSize) {
      *p++ = nixSize + 4;
      size += nixSize;

      uint32_t serialized = m_nixVector->Serialize(p, nixSize);
      if (serialized) {
        p += ((nixSize + 3) & (~3)) / 4;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  } else {
    if (size + 4 <= maxSize) {
      size += 4;
      *p++ = 4;
    } else {
      return 0;
    }
  }

  uint32_t byteTagSize = m_byteTagList.GetSerializedSize();
  if (size + byteTagSize <= maxSize) {
    *p++ = byteTagSize + 4;
    size += byteTagSize;

    uint32_t serialized = m_byteTagList.Serialize(p, byteTagSize);
    if (serialized) {
      p += ((byteTagSize + 3) & (~3)) / 4;
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  uint32_t packetTagSize = m_packetTagList.GetSerializedSize();
  if (size + packetTagSize <= maxSize) {
    *p++ = packetTagSize + 4;
    size += packetTagSize;

    uint32_t serialized = m_packetTagList.Serialize(p, packetTagSize);
    if (serialized) {
      p += ((packetTagSize + 3) & (~3)) / 4;
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  uint32_t metaSize = m_metadata.GetSerializedSize();
  if (size + metaSize <= maxSize) {
    *p++ = metaSize + 4;
    size += metaSize;

    uint32_t serialized =
        m_metadata.Serialize(reinterpret_cast<uint8_t *>(p), metaSize);
    if (serialized) {
      p += ((metaSize + 3) & (~3)) / 4;
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  uint32_t bufSize = m_buffer.GetSerializedSize();
  if (size + bufSize <= maxSize) {
    *p++ = bufSize + 4;

    uint32_t serialized =
        m_buffer.Serialize(reinterpret_cast<uint8_t *>(p), bufSize);
    if (!serialized) {
      return 0;
    }
  } else {
    return 0;
  }

  return 1;
}

uint32_t Packet::Deserialize(const uint8_t *buffer, uint32_t size) {
  NS_LOG_FUNCTION(this);

  auto p = reinterpret_cast<const uint32_t *>(buffer);

  NS_ASSERT(!m_nixVector);
  uint32_t nixSize = *p++;

  NS_ASSERT(size >= nixSize);

  if (nixSize > 4) {
    Ptr<NixVector> nix = Create<NixVector>();
    uint32_t nixDeserialized = nix->Deserialize(p, nixSize);
    if (!nixDeserialized) {
      return 0;
    }
    m_nixVector = nix;
    p += ((((nixSize - 4) + 3) & (~3)) / 4);
  }
  size -= nixSize;

  uint32_t byteTagSize = *p++;

  NS_ASSERT(size >= byteTagSize);

  uint32_t byteTagDeserialized = m_byteTagList.Deserialize(p, byteTagSize);
  if (!byteTagDeserialized) {
    return 0;
  }
  p += ((((byteTagSize - 4) + 3) & (~3)) / 4);
  size -= byteTagSize;

  uint32_t packetTagSize = *p++;

  NS_ASSERT(size >= packetTagSize);

  uint32_t packetTagDeserialized =
      m_packetTagList.Deserialize(p, packetTagSize);
  if (!packetTagDeserialized) {
    return 0;
  }
  p += ((((packetTagSize - 4) + 3) & (~3)) / 4);
  size -= packetTagSize;

  uint32_t metaSize = *p++;

  NS_ASSERT(size >= metaSize);

  uint32_t metadataDeserialized =
      m_metadata.Deserialize(reinterpret_cast<const uint8_t *>(p), metaSize);
  if (!metadataDeserialized) {
    return 0;
  }
  p += ((((metaSize - 4) + 3) & (~3)) / 4);
  size -= metaSize;

  uint32_t bufSize = *p++;

  NS_ASSERT(size >= bufSize);

  uint32_t bufferDeserialized =
      m_buffer.Deserialize(reinterpret_cast<const uint8_t *>(p), bufSize);
  if (!bufferDeserialized) {
    return 0;
  }
  size -= bufSize;

  return (size == 0);
}

void Packet::AddByteTag(const Tag &tag) const {
  NS_LOG_FUNCTION(this << tag.GetInstanceTypeId().GetName()
                       << tag.GetSerializedSize());
  auto list = const_cast<ByteTagList *>(&m_byteTagList);
  TagBuffer buffer =
      list->Add(tag.GetInstanceTypeId(), tag.GetSerializedSize(), 0, GetSize());
  tag.Serialize(buffer);
}

void Packet::AddByteTag(const Tag &tag, uint32_t start, uint32_t end) const {
  NS_LOG_FUNCTION(this << tag.GetInstanceTypeId().GetName()
                       << tag.GetSerializedSize());
  NS_ABORT_MSG_IF(end < start, "Invalid byte range");
  auto list = const_cast<ByteTagList *>(&m_byteTagList);
  TagBuffer buffer =
      list->Add(tag.GetInstanceTypeId(), tag.GetSerializedSize(),
                static_cast<int32_t>(start), static_cast<int32_t>(end));
  tag.Serialize(buffer);
}

ByteTagIterator Packet::GetByteTagIterator() const {
  return ByteTagIterator(m_byteTagList.Begin(0, GetSize()));
}

bool Packet::FindFirstMatchingByteTag(Tag &tag) const {
  TypeId tid = tag.GetInstanceTypeId();
  ByteTagIterator i = GetByteTagIterator();
  while (i.HasNext()) {
    ByteTagIterator::Item item = i.Next();
    if (tid == item.GetTypeId()) {
      item.GetTag(tag);
      return true;
    }
  }
  return false;
}

void Packet::AddPacketTag(const Tag &tag) const {
  NS_LOG_FUNCTION(this << tag.GetInstanceTypeId().GetName()
                       << tag.GetSerializedSize());
  m_packetTagList.Add(tag);
}

bool Packet::RemovePacketTag(Tag &tag) {
  NS_LOG_FUNCTION(this << tag.GetInstanceTypeId().GetName()
                       << tag.GetSerializedSize());
  bool found = m_packetTagList.Remove(tag);
  return found;
}

bool Packet::ReplacePacketTag(Tag &tag) {
  NS_LOG_FUNCTION(this << tag.GetInstanceTypeId().GetName()
                       << tag.GetSerializedSize());
  bool found = m_packetTagList.Replace(tag);
  return found;
}

bool Packet::PeekPacketTag(Tag &tag) const {
  bool found = m_packetTagList.Peek(tag);
  return found;
}

void Packet::RemoveAllPacketTags() {
  NS_LOG_FUNCTION(this);
  m_packetTagList.RemoveAll();
}

void Packet::PrintPacketTags(std::ostream &os) const {
  PacketTagIterator i = GetPacketTagIterator();
  while (i.HasNext()) {
    PacketTagIterator::Item item = i.Next();
    NS_ASSERT(item.GetTypeId().HasConstructor());
    Callback<ObjectBase *> constructor = item.GetTypeId().GetConstructor();
    NS_ASSERT(!constructor.IsNull());
    ObjectBase *instance = constructor();
    Tag *tag = dynamic_cast<Tag *>(instance);
    NS_ASSERT(tag != nullptr);
    item.GetTag(*tag);
    tag->Print(os);
    delete tag;
    if (i.HasNext()) {
      os << " ";
    }
  }
}

PacketTagIterator Packet::GetPacketTagIterator() const {
  return PacketTagIterator(m_packetTagList.Head());
}

std::ostream &operator<<(std::ostream &os, const Packet &packet) {
  packet.Print(os);
  return os;
}

} // namespace ns3
