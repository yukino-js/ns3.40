
#include "wifi-information-element.h"

namespace ns3 {

WifiInformationElement::~WifiInformationElement() {}

void WifiInformationElement::Print(std::ostream &os) const {}

uint16_t WifiInformationElement::GetSerializedSize() const {
  uint16_t size = GetInformationFieldSize();

  if (size <= 255) {
    return (2 + size);
  }

  uint16_t m = size / 255;
  uint8_t remainder = size % 255;
  uint8_t n = (remainder > 0) ? 1 : 0;

  return m * (2 + 255) + n * (2 + remainder);
}

WifiInformationElementId WifiInformationElement::ElementIdExt() const {
  return 0;
}

Buffer::Iterator WifiInformationElement::Serialize(Buffer::Iterator i) const {
  auto size = GetInformationFieldSize();

  if (size > 255) {
    return SerializeFragments(i, size);
  }

  i.WriteU8(ElementId());
  i.WriteU8(size);
  if (ElementId() == IE_EXTENSION) {
    i.WriteU8(ElementIdExt());
    SerializeInformationField(i);
    i.Next(size - 1);
  } else {
    SerializeInformationField(i);
    i.Next(size);
  }
  return i;
}

Buffer::Iterator
WifiInformationElement::SerializeFragments(Buffer::Iterator i,
                                           uint16_t size) const {
  NS_ASSERT(size > 255);
  Buffer buffer;
  buffer.AddAtStart(size);
  Buffer::Iterator source = buffer.Begin();
  SerializeInformationField(source);

  uint16_t m = size / 255;

  for (uint16_t index = 0; index < m; index++) {
    i.WriteU8((index == 0) ? ElementId() : IE_FRAGMENT);
    i.WriteU8(255);
    uint8_t length = 255;
    if (index == 0 && ElementId() == IE_EXTENSION) {
      i.WriteU8(ElementIdExt());
      length = 254;
    }
    for (uint8_t count = 0; count < length; count++) {
      i.WriteU8(source.ReadU8());
    }
  }

  uint8_t remainder = size % 255;

  if (remainder > 0) {
    i.WriteU8(IE_FRAGMENT);
    i.WriteU8(remainder);
    for (uint8_t count = 0; count < remainder; count++) {
      i.WriteU8(source.ReadU8());
    }
  }

  return i;
}

Buffer::Iterator WifiInformationElement::Deserialize(Buffer::Iterator i) {
  Buffer::Iterator start = i;
  i = DeserializeIfPresent(i);
  NS_ASSERT(i.GetDistanceFrom(start) != 0);
  return i;
}

Buffer::Iterator
WifiInformationElement::DeserializeIfPresent(Buffer::Iterator i) {
  if (i.IsEnd()) {
    return i;
  }
  Buffer::Iterator start = i;
  uint8_t elementId = i.ReadU8();

  if (elementId != ElementId()) {
    return start;
  }

  uint16_t length = i.ReadU8();
  if (ElementId() == IE_EXTENSION) {
    uint8_t elementIdExt = i.ReadU8();
    if (elementIdExt != ElementIdExt()) {
      return start;
    }
    length--;
  }

  return DoDeserialize(i, length);
}

Buffer::Iterator WifiInformationElement::DoDeserialize(Buffer::Iterator i,
                                                       uint16_t length) {
  uint16_t limit = (ElementId() == IE_EXTENSION) ? 254 : 255;

  auto tmp = i;
  tmp.Next(length);

  if (length < limit || tmp.IsEnd() || (tmp.PeekU8() != IE_FRAGMENT)) {
    DeserializeInformationField(i, length);
    return tmp;
  }

  NS_ASSERT(length == limit);

  Buffer buffer;
  buffer.AddAtStart(length);
  Buffer::Iterator bufferIt = buffer.Begin();

  uint16_t count = length;
  length = 0;

  while (true) {
    for (uint16_t index = 0; index < count; index++) {
      bufferIt.WriteU8(i.ReadU8());
    }
    length += count;

    if (i.IsEnd() || (i.PeekU8() != IE_FRAGMENT)) {
      break;
    }
    i.Next(1);
    count = i.ReadU8();

    buffer.AddAtEnd(count);
    bufferIt = buffer.Begin();
    bufferIt.Next(length);
  }

  DeserializeInformationField(buffer.Begin(), length);
  return i;
}

bool WifiInformationElement::operator==(const WifiInformationElement &a) const {
  if (ElementId() != a.ElementId()) {
    return false;
  }

  if (ElementIdExt() != a.ElementIdExt()) {
    return false;
  }

  uint32_t ieSize = GetSerializedSize();

  if (ieSize != a.GetSerializedSize()) {
    return false;
  }

  Buffer myIe;
  Buffer hisIe;
  myIe.AddAtEnd(ieSize);
  hisIe.AddAtEnd(ieSize);

  Serialize(myIe.Begin());
  a.Serialize(hisIe.Begin());

  return (memcmp(myIe.PeekData(), hisIe.PeekData(), ieSize) == 0);
}

std::ostream &operator<<(std::ostream &os,
                         const WifiInformationElement &element) {
  element.Print(os);
  return os;
}

} // namespace ns3
