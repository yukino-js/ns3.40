
#ifndef PACKETBB_H
#define PACKETBB_H

#include "ns3/address.h"
#include "ns3/buffer.h"
#include "ns3/header.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

#include <list>

namespace ns3 {

class PbbMessage;
class PbbAddressBlock;
class PbbTlv;
class PbbAddressTlv;

enum PbbAddressLength {
  IPV4 = 3,
  IPV6 = 15,
};

class PbbTlvBlock {
public:
  typedef std::list<Ptr<PbbTlv>>::iterator Iterator;
  typedef std::list<Ptr<PbbTlv>>::const_iterator ConstIterator;

  PbbTlvBlock();
  ~PbbTlvBlock();

  Iterator Begin();

  ConstIterator Begin() const;

  Iterator End();

  ConstIterator End() const;

  int Size() const;

  bool Empty() const;

  Ptr<PbbTlv> Front() const;

  Ptr<PbbTlv> Back() const;

  void PushFront(Ptr<PbbTlv> tlv);

  void PopFront();

  void PushBack(Ptr<PbbTlv> tlv);

  void PopBack();

  Iterator Insert(Iterator position, const Ptr<PbbTlv> tlv);

  Iterator Erase(Iterator position);

  Iterator Erase(Iterator first, Iterator last);

  void Clear();

  uint32_t GetSerializedSize() const;

  void Serialize(Buffer::Iterator &start) const;

  void Deserialize(Buffer::Iterator &start);

  void Print(std::ostream &os) const;

  void Print(std::ostream &os, int level) const;

  bool operator==(const PbbTlvBlock &other) const;
  bool operator!=(const PbbTlvBlock &other) const;

private:
  std::list<Ptr<PbbTlv>> m_tlvList;
};

class PbbAddressTlvBlock {
public:
  typedef std::list<Ptr<PbbAddressTlv>>::iterator Iterator;
  typedef std::list<Ptr<PbbAddressTlv>>::const_iterator ConstIterator;

  PbbAddressTlvBlock();
  ~PbbAddressTlvBlock();

  Iterator Begin();

  ConstIterator Begin() const;

  Iterator End();

  ConstIterator End() const;

  int Size() const;

  bool Empty() const;

  Ptr<PbbAddressTlv> Front() const;

  Ptr<PbbAddressTlv> Back() const;

  void PushFront(Ptr<PbbAddressTlv> tlv);

  void PopFront();

  void PushBack(Ptr<PbbAddressTlv> tlv);

  void PopBack();

  Iterator Insert(Iterator position, const Ptr<PbbAddressTlv> tlv);

  Iterator Erase(Iterator position);

  Iterator Erase(Iterator first, Iterator last);

  void Clear();

  uint32_t GetSerializedSize() const;

  void Serialize(Buffer::Iterator &start) const;

  void Deserialize(Buffer::Iterator &start);

  void Print(std::ostream &os) const;

  void Print(std::ostream &os, int level) const;

  bool operator==(const PbbAddressTlvBlock &other) const;

  bool operator!=(const PbbAddressTlvBlock &other) const;

private:
  std::list<Ptr<PbbAddressTlv>> m_tlvList;
};

class PbbPacket : public SimpleRefCount<PbbPacket, Header> {
public:
  typedef std::list<Ptr<PbbTlv>>::iterator TlvIterator;
  typedef std::list<Ptr<PbbTlv>>::const_iterator ConstTlvIterator;
  typedef std::list<Ptr<PbbMessage>>::iterator MessageIterator;
  typedef std::list<Ptr<PbbMessage>>::const_iterator ConstMessageIterator;

  PbbPacket();
  ~PbbPacket() override;

  uint8_t GetVersion() const;

  void SetSequenceNumber(uint16_t number);

  uint16_t GetSequenceNumber() const;

  bool HasSequenceNumber() const;

  TlvIterator TlvBegin();

  ConstTlvIterator TlvBegin() const;

  TlvIterator TlvEnd();

  ConstTlvIterator TlvEnd() const;

  int TlvSize() const;

  bool TlvEmpty() const;

  Ptr<PbbTlv> TlvFront();

  const Ptr<PbbTlv> TlvFront() const;

  Ptr<PbbTlv> TlvBack();

  const Ptr<PbbTlv> TlvBack() const;

  void TlvPushFront(Ptr<PbbTlv> tlv);

  void TlvPopFront();

  void TlvPushBack(Ptr<PbbTlv> tlv);

  void TlvPopBack();

  TlvIterator Erase(TlvIterator position);

  TlvIterator Erase(TlvIterator first, TlvIterator last);

  void TlvClear();

  MessageIterator MessageBegin();

  ConstMessageIterator MessageBegin() const;

  MessageIterator MessageEnd();

  ConstMessageIterator MessageEnd() const;

  int MessageSize() const;

  bool MessageEmpty() const;

  Ptr<PbbMessage> MessageFront();

  const Ptr<PbbMessage> MessageFront() const;

  Ptr<PbbMessage> MessageBack();

  const Ptr<PbbMessage> MessageBack() const;

  void MessagePushFront(Ptr<PbbMessage> message);

  void MessagePopFront();

  void MessagePushBack(Ptr<PbbMessage> message);

  void MessagePopBack();

  MessageIterator Erase(MessageIterator position);

  MessageIterator Erase(MessageIterator first, MessageIterator last);

  void MessageClear();

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void Print(std::ostream &os) const override;

  bool operator==(const PbbPacket &other) const;

  bool operator!=(const PbbPacket &other) const;

protected:
private:
  PbbTlvBlock m_tlvList;
  std::list<Ptr<PbbMessage>> m_messageList;

  uint8_t m_version;

  bool m_hasseqnum;
  uint16_t m_seqnum;
};

class PbbMessage : public SimpleRefCount<PbbMessage> {
public:
  typedef std::list<Ptr<PbbTlv>>::iterator TlvIterator;
  typedef std::list<Ptr<PbbTlv>>::const_iterator ConstTlvIterator;
  typedef std::list<Ptr<PbbAddressBlock>>::iterator AddressBlockIterator;
  typedef std::list<Ptr<PbbAddressBlock>>::const_iterator
      ConstAddressBlockIterator;

  PbbMessage();
  virtual ~PbbMessage();

  void SetType(uint8_t type);

  uint8_t GetType() const;

  void SetOriginatorAddress(Address address);

  Address GetOriginatorAddress() const;

  bool HasOriginatorAddress() const;

  void SetHopLimit(uint8_t hoplimit);

  uint8_t GetHopLimit() const;

  bool HasHopLimit() const;

  void SetHopCount(uint8_t hopcount);

  uint8_t GetHopCount() const;

  bool HasHopCount() const;

  void SetSequenceNumber(uint16_t seqnum);

  uint16_t GetSequenceNumber() const;

  bool HasSequenceNumber() const;

  TlvIterator TlvBegin();

  ConstTlvIterator TlvBegin() const;

  TlvIterator TlvEnd();

  ConstTlvIterator TlvEnd() const;

  int TlvSize() const;

  bool TlvEmpty() const;

  Ptr<PbbTlv> TlvFront();

  const Ptr<PbbTlv> TlvFront() const;

  Ptr<PbbTlv> TlvBack();

  const Ptr<PbbTlv> TlvBack() const;

  void TlvPushFront(Ptr<PbbTlv> tlv);

  void TlvPopFront();

  void TlvPushBack(Ptr<PbbTlv> tlv);

  void TlvPopBack();

  TlvIterator TlvErase(TlvIterator position);

  TlvIterator TlvErase(TlvIterator first, TlvIterator last);

  void TlvClear();

  AddressBlockIterator AddressBlockBegin();

  ConstAddressBlockIterator AddressBlockBegin() const;

  AddressBlockIterator AddressBlockEnd();

  ConstAddressBlockIterator AddressBlockEnd() const;

  int AddressBlockSize() const;

  bool AddressBlockEmpty() const;

  Ptr<PbbAddressBlock> AddressBlockFront();

  const Ptr<PbbAddressBlock> AddressBlockFront() const;

  Ptr<PbbAddressBlock> AddressBlockBack();

  const Ptr<PbbAddressBlock> AddressBlockBack() const;

  void AddressBlockPushFront(Ptr<PbbAddressBlock> block);

  void AddressBlockPopFront();

  void AddressBlockPushBack(Ptr<PbbAddressBlock> block);

  void AddressBlockPopBack();

  AddressBlockIterator AddressBlockErase(AddressBlockIterator position);

  AddressBlockIterator AddressBlockErase(AddressBlockIterator first,
                                         AddressBlockIterator last);

  void AddressBlockClear();

  static Ptr<PbbMessage> DeserializeMessage(Buffer::Iterator &start);

  uint32_t GetSerializedSize() const;

  void Serialize(Buffer::Iterator &start) const;

  void Deserialize(Buffer::Iterator &start);

  void Print(std::ostream &os) const;

  void Print(std::ostream &os, int level) const;

  bool operator==(const PbbMessage &other) const;
  bool operator!=(const PbbMessage &other) const;

protected:
  virtual PbbAddressLength GetAddressLength() const = 0;

  virtual void SerializeOriginatorAddress(Buffer::Iterator &start) const = 0;
  virtual Address
  DeserializeOriginatorAddress(Buffer::Iterator &start) const = 0;
  virtual void PrintOriginatorAddress(std::ostream &os) const = 0;

  virtual Ptr<PbbAddressBlock>
  AddressBlockDeserialize(Buffer::Iterator &start) const = 0;

private:
  PbbTlvBlock m_tlvList;
  std::list<Ptr<PbbAddressBlock>> m_addressBlockList;

  uint8_t m_type;
  PbbAddressLength m_addrSize;

  bool m_hasOriginatorAddress;
  Address m_originatorAddress;

  bool m_hasHopLimit;
  uint8_t m_hopLimit;

  bool m_hasHopCount;
  uint8_t m_hopCount;

  bool m_hasSequenceNumber;
  uint16_t m_sequenceNumber;
};

class PbbMessageIpv4 : public PbbMessage {
public:
  PbbMessageIpv4();

protected:
  PbbAddressLength GetAddressLength() const override;

  void SerializeOriginatorAddress(Buffer::Iterator &start) const override;
  Address DeserializeOriginatorAddress(Buffer::Iterator &start) const override;
  void PrintOriginatorAddress(std::ostream &os) const override;

  Ptr<PbbAddressBlock>
  AddressBlockDeserialize(Buffer::Iterator &start) const override;
};

class PbbMessageIpv6 : public PbbMessage {
public:
  PbbMessageIpv6();

protected:
  PbbAddressLength GetAddressLength() const override;

  void SerializeOriginatorAddress(Buffer::Iterator &start) const override;
  Address DeserializeOriginatorAddress(Buffer::Iterator &start) const override;
  void PrintOriginatorAddress(std::ostream &os) const override;

  Ptr<PbbAddressBlock>
  AddressBlockDeserialize(Buffer::Iterator &start) const override;
};

class PbbAddressBlock : public SimpleRefCount<PbbAddressBlock> {
public:
  typedef std::list<Address>::iterator AddressIterator;
  typedef std::list<Address>::const_iterator ConstAddressIterator;

  typedef std::list<uint8_t>::iterator PrefixIterator;
  typedef std::list<uint8_t>::const_iterator ConstPrefixIterator;

  typedef PbbAddressTlvBlock::Iterator TlvIterator;
  typedef PbbAddressTlvBlock::ConstIterator ConstTlvIterator;

  PbbAddressBlock();
  virtual ~PbbAddressBlock();

  AddressIterator AddressBegin();

  ConstAddressIterator AddressBegin() const;

  AddressIterator AddressEnd();

  ConstAddressIterator AddressEnd() const;

  int AddressSize() const;

  bool AddressEmpty() const;

  Address AddressFront() const;

  Address AddressBack() const;

  void AddressPushFront(Address address);

  void AddressPopFront();

  void AddressPushBack(Address address);

  void AddressPopBack();

  AddressIterator AddressInsert(AddressIterator position, const Address value);

  AddressIterator AddressErase(AddressIterator position);

  AddressIterator AddressErase(AddressIterator first, AddressIterator last);

  void AddressClear();

  PrefixIterator PrefixBegin();

  ConstPrefixIterator PrefixBegin() const;

  PrefixIterator PrefixEnd();

  ConstPrefixIterator PrefixEnd() const;

  int PrefixSize() const;

  bool PrefixEmpty() const;

  uint8_t PrefixFront() const;

  uint8_t PrefixBack() const;

  void PrefixPushFront(uint8_t prefix);

  void PrefixPopFront();

  void PrefixPushBack(uint8_t prefix);

  void PrefixPopBack();

  PrefixIterator PrefixInsert(PrefixIterator position, const uint8_t value);

  PrefixIterator PrefixErase(PrefixIterator position);

  PrefixIterator PrefixErase(PrefixIterator first, PrefixIterator last);

  void PrefixClear();

  TlvIterator TlvBegin();

  ConstTlvIterator TlvBegin() const;

  TlvIterator TlvEnd();

  ConstTlvIterator TlvEnd() const;

  int TlvSize() const;

  bool TlvEmpty() const;

  Ptr<PbbAddressTlv> TlvFront();

  const Ptr<PbbAddressTlv> TlvFront() const;

  Ptr<PbbAddressTlv> TlvBack();

  const Ptr<PbbAddressTlv> TlvBack() const;

  void TlvPushFront(Ptr<PbbAddressTlv> address);

  void TlvPopFront();

  void TlvPushBack(Ptr<PbbAddressTlv> address);

  void TlvPopBack();

  TlvIterator TlvInsert(TlvIterator position, const Ptr<PbbTlv> value);

  TlvIterator TlvErase(TlvIterator position);

  TlvIterator TlvErase(TlvIterator first, TlvIterator last);

  void TlvClear();

  uint32_t GetSerializedSize() const;

  void Serialize(Buffer::Iterator &start) const;

  void Deserialize(Buffer::Iterator &start);

  void Print(std::ostream &os) const;

  void Print(std::ostream &os, int level) const;

  bool operator==(const PbbAddressBlock &other) const;

  bool operator!=(const PbbAddressBlock &other) const;

protected:
  virtual uint8_t GetAddressLength() const = 0;
  virtual void SerializeAddress(uint8_t *buffer,
                                ConstAddressIterator iter) const = 0;
  virtual Address DeserializeAddress(uint8_t *buffer) const = 0;
  virtual void PrintAddress(std::ostream &os,
                            ConstAddressIterator iter) const = 0;

private:
  uint8_t GetPrefixFlags() const;
  void GetHeadTail(uint8_t *head, uint8_t &headlen, uint8_t *tail,
                   uint8_t &taillen) const;

  bool HasZeroTail(const uint8_t *tail, uint8_t taillen) const;

  std::list<Address> m_addressList;
  std::list<uint8_t> m_prefixList;
  PbbAddressTlvBlock m_addressTlvList;
};

class PbbAddressBlockIpv4 : public PbbAddressBlock {
public:
  PbbAddressBlockIpv4();
  ~PbbAddressBlockIpv4() override;

protected:
  uint8_t GetAddressLength() const override;
  void SerializeAddress(uint8_t *buffer,
                        ConstAddressIterator iter) const override;
  Address DeserializeAddress(uint8_t *buffer) const override;
  void PrintAddress(std::ostream &os, ConstAddressIterator iter) const override;
};

class PbbAddressBlockIpv6 : public PbbAddressBlock {
public:
  PbbAddressBlockIpv6();
  ~PbbAddressBlockIpv6() override;

protected:
  uint8_t GetAddressLength() const override;
  void SerializeAddress(uint8_t *buffer,
                        ConstAddressIterator iter) const override;
  Address DeserializeAddress(uint8_t *buffer) const override;
  void PrintAddress(std::ostream &os, ConstAddressIterator iter) const override;
};

class PbbTlv : public SimpleRefCount<PbbTlv> {
public:
  PbbTlv();
  virtual ~PbbTlv();

  void SetType(uint8_t type);

  uint8_t GetType() const;

  void SetTypeExt(uint8_t type);

  uint8_t GetTypeExt() const;

  bool HasTypeExt() const;

  void SetValue(Buffer start);

  void SetValue(const uint8_t *buffer, uint32_t size);

  Buffer GetValue() const;

  bool HasValue() const;

  uint32_t GetSerializedSize() const;

  void Serialize(Buffer::Iterator &start) const;

  void Deserialize(Buffer::Iterator &start);

  void Print(std::ostream &os) const;

  void Print(std::ostream &os, int level) const;

  bool operator==(const PbbTlv &other) const;

  bool operator!=(const PbbTlv &other) const;

protected:
  void SetIndexStart(uint8_t index);
  uint8_t GetIndexStart() const;
  bool HasIndexStart() const;

  void SetIndexStop(uint8_t index);
  uint8_t GetIndexStop() const;
  bool HasIndexStop() const;

  void SetMultivalue(bool isMultivalue);
  bool IsMultivalue() const;

private:
  uint8_t m_type;

  bool m_hasTypeExt;
  uint8_t m_typeExt;

  bool m_hasIndexStart;
  uint8_t m_indexStart;

  bool m_hasIndexStop;
  uint8_t m_indexStop;

  bool m_isMultivalue;
  bool m_hasValue;
  Buffer m_value;
};

class PbbAddressTlv : public PbbTlv {
public:
  void SetIndexStart(uint8_t index);

  uint8_t GetIndexStart() const;

  bool HasIndexStart() const;

  void SetIndexStop(uint8_t index);

  uint8_t GetIndexStop() const;

  bool HasIndexStop() const;

  void SetMultivalue(bool isMultivalue);

  bool IsMultivalue() const;
};

} // namespace ns3

#endif
