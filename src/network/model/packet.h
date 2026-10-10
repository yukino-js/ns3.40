#ifndef PACKET_H
#define PACKET_H

#include "buffer.h"
#include "byte-tag-list.h"
#include "header.h"
#include "nix-vector.h"
#include "packet-metadata.h"
#include "packet-tag-list.h"
#include "tag.h"
#include "trailer.h"

#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/mac48-address.h"
#include "ns3/ptr.h"

#include <stdint.h>

namespace ns3 {

class Address;

class ByteTagIterator {
public:
  class Item {
  public:
    TypeId GetTypeId() const;
    uint32_t GetStart() const;
    uint32_t GetEnd() const;
    void GetTag(Tag &tag) const;

  private:
    friend class ByteTagIterator;
    Item(TypeId tid, uint32_t start, uint32_t end, TagBuffer buffer);

    TypeId m_tid;
    uint32_t m_start;
    uint32_t m_end;
    TagBuffer m_buffer;
  };

  bool HasNext() const;
  Item Next();

private:
  friend class Packet;
  ByteTagIterator(ByteTagList::Iterator i);
  ByteTagList::Iterator m_current;
};

class PacketTagIterator {
public:
  class Item {
  public:
    TypeId GetTypeId() const;
    void GetTag(Tag &tag) const;

  private:
    friend class PacketTagIterator;
    Item(const PacketTagList::TagData *data);
    const PacketTagList::TagData *m_data;
  };

  bool HasNext() const;
  Item Next();

private:
  friend class Packet;
  PacketTagIterator(const PacketTagList::TagData *head);
  const PacketTagList::TagData *m_current;
};

class Packet : public SimpleRefCount<Packet> {
public:
  Packet();
  Packet(const Packet &o);
  Packet &operator=(const Packet &o);
  Packet(uint32_t size);
  Packet(const uint8_t *buffer, uint32_t size, bool magic);
  Packet(const uint8_t *buffer, uint32_t size);
  Ptr<Packet> CreateFragment(uint32_t start, uint32_t length) const;
  inline uint32_t GetSize() const;
  void AddHeader(const Header &header);
  uint32_t RemoveHeader(Header &header);
  uint32_t RemoveHeader(Header &header, uint32_t size);
  uint32_t PeekHeader(Header &header) const;
  uint32_t PeekHeader(Header &header, uint32_t size) const;
  void AddTrailer(const Trailer &trailer);
  uint32_t RemoveTrailer(Trailer &trailer);
  uint32_t PeekTrailer(Trailer &trailer);

  void AddAtEnd(Ptr<const Packet> packet);
  void AddPaddingAtEnd(uint32_t size);
  void RemoveAtEnd(uint32_t size);
  void RemoveAtStart(uint32_t size);

  uint32_t CopyData(uint8_t *buffer, uint32_t size) const;

  void CopyData(std::ostream *os, uint32_t size) const;

  Ptr<Packet> Copy() const;

  uint64_t GetUid() const;

  void Print(std::ostream &os) const;

  std::string ToString() const;

  PacketMetadata::ItemIterator BeginItem() const;

  static void EnablePrinting();
  static void EnableChecking();

  uint32_t GetSerializedSize() const;

  uint32_t Serialize(uint8_t *buffer, uint32_t maxSize) const;

  void AddByteTag(const Tag &tag) const;

  void AddByteTag(const Tag &tag, uint32_t start, uint32_t end) const;
  ByteTagIterator GetByteTagIterator() const;
  bool FindFirstMatchingByteTag(Tag &tag) const;

  void RemoveAllByteTags();

  void PrintByteTags(std::ostream &os) const;

  void AddPacketTag(const Tag &tag) const;
  bool RemovePacketTag(Tag &tag);
  bool ReplacePacketTag(Tag &tag);
  bool PeekPacketTag(Tag &tag) const;
  void RemoveAllPacketTags();

  void PrintPacketTags(std::ostream &os) const;

  PacketTagIterator GetPacketTagIterator() const;

  void SetNixVector(Ptr<NixVector> nixVector) const;
  Ptr<NixVector> GetNixVector() const;

  typedef void (*TracedCallback)(Ptr<const Packet> packet);

  typedef void (*AddressTracedCallback)(Ptr<const Packet> packet,
                                        const Address &address);

  typedef void (*TwoAddressTracedCallback)(const Ptr<const Packet> packet,
                                           const Address &srcAddress,
                                           const Address &destAddress);

  typedef void (*Mac48AddressTracedCallback)(Ptr<const Packet> packet,
                                             Mac48Address mac);

  typedef void (*SizeTracedCallback)(uint32_t oldSize, uint32_t newSize);

  typedef void (*SinrTracedCallback)(Ptr<const Packet> packet, double sinr);

private:
  Packet(const Buffer &buffer, const ByteTagList &byteTagList,
         const PacketTagList &packetTagList, const PacketMetadata &metadata);

  uint32_t Deserialize(const uint8_t *buffer, uint32_t size);

  Buffer m_buffer;
  ByteTagList m_byteTagList;
  PacketTagList m_packetTagList;
  PacketMetadata m_metadata;

  mutable Ptr<NixVector> m_nixVector;

  static uint32_t m_globalUid;
};

std::ostream &operator<<(std::ostream &os, const Packet &packet);

} // namespace ns3

namespace ns3 {

uint32_t Packet::GetSize() const { return m_buffer.GetSize(); }

} // namespace ns3

#endif
