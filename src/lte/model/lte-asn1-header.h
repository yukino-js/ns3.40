
#ifndef ASN1_HEADER_H
#define ASN1_HEADER_H

#include "ns3/header.h"

#include <bitset>
#include <string>

namespace ns3 {

class Asn1Header : public Header {
public:
  Asn1Header();
  ~Asn1Header() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator bIterator) const override;

  uint32_t Deserialize(Buffer::Iterator bIterator) override = 0;
  void Print(std::ostream &os) const override = 0;

  virtual void PreSerialize() const = 0;

protected:
  mutable uint8_t m_serializationPendingBits;
  mutable uint8_t m_numSerializationPendingBits;
  mutable bool m_isDataSerialized;
  mutable Buffer m_serializationResult;

  void WriteOctet(uint8_t octet) const;

  void SerializeBoolean(bool value) const;
  void SerializeInteger(int n, int nmin, int nmax) const;
  void SerializeSequenceOf(int numElems, int nMax, int nMin) const;
  void SerializeChoice(int numOptions, int selectedOption,
                       bool isExtensionMarkerPresent) const;
  void SerializeEnum(int numElems, int selectedElem) const;
  void SerializeNull() const;
  void FinalizeSerialization() const;

  template <int N> void SerializeBitset(std::bitset<N> data) const;

  template <int N>
  void SerializeSequence(std::bitset<N> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<0> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<1> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<2> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<3> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<4> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<5> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<6> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<7> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<9> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<10> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;
  void SerializeSequence(std::bitset<11> optionalOrDefaultMask,
                         bool isExtensionMarkerPresent) const;

  template <int N> void SerializeBitstring(std::bitset<N> bitstring) const;
  void SerializeBitstring(std::bitset<1> bitstring) const;
  void SerializeBitstring(std::bitset<2> bitstring) const;
  void SerializeBitstring(std::bitset<8> bitstring) const;
  void SerializeBitstring(std::bitset<10> bitstring) const;
  void SerializeBitstring(std::bitset<16> bitstring) const;
  void SerializeBitstring(std::bitset<27> bitstring) const;
  void SerializeBitstring(std::bitset<28> bitstring) const;
  void SerializeBitstring(std::bitset<32> bitstring) const;

  template <int N>
  Buffer::Iterator DeserializeBitset(std::bitset<N> *data,
                                     Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitset(std::bitset<8> *data,
                                     Buffer::Iterator bIterator);

  Buffer::Iterator DeserializeBoolean(bool *value, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeInteger(int *n, int nmin, int nmax,
                                      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeChoice(int numOptions,
                                     bool isExtensionMarkerPresent,
                                     int *selectedOption,
                                     Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeEnum(int numElems, int *selectedElem,
                                   Buffer::Iterator bIterator);

  template <int N>
  Buffer::Iterator DeserializeSequence(std::bitset<N> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<0> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<1> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<2> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<3> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<4> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<5> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<6> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);

  Buffer::Iterator DeserializeSequence(std::bitset<7> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<9> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<10> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequence(std::bitset<11> *optionalOrDefaultMask,
                                       bool isExtensionMarkerPresent,
                                       Buffer::Iterator bIterator);

  template <int N>
  Buffer::Iterator DeserializeBitstring(std::bitset<N> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<1> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<2> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<8> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<10> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<16> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<27> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<28> *bitstring,
                                        Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeBitstring(std::bitset<32> *bitstring,
                                        Buffer::Iterator bIterator);

  Buffer::Iterator DeserializeNull(Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSequenceOf(int *numElems, int nMax, int nMin,
                                         Buffer::Iterator bIterator);
};

} // namespace ns3

#endif
