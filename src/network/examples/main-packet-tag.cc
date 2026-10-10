#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

#include <iostream>

using namespace ns3;

class MyTag : public Tag {
public:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

  void SetSimpleValue(uint8_t value);
  uint8_t GetSimpleValue() const;

private:
  uint8_t m_simpleValue;
};

TypeId MyTag::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::MyTag")
          .SetParent<Tag>()
          .AddConstructor<MyTag>()
          .AddAttribute("SimpleValue", "A simple value", EmptyAttributeValue(),
                        MakeUintegerAccessor(&MyTag::GetSimpleValue),
                        MakeUintegerChecker<uint8_t>());
  return tid;
}

TypeId MyTag::GetInstanceTypeId() const { return GetTypeId(); }

uint32_t MyTag::GetSerializedSize() const { return 1; }

void MyTag::Serialize(TagBuffer i) const { i.WriteU8(m_simpleValue); }

void MyTag::Deserialize(TagBuffer i) { m_simpleValue = i.ReadU8(); }

void MyTag::Print(std::ostream &os) const {
  os << "v=" << (uint32_t)m_simpleValue;
}

void MyTag::SetSimpleValue(uint8_t value) { m_simpleValue = value; }

uint8_t MyTag::GetSimpleValue() const { return m_simpleValue; }

int main(int argc, char *argv[]) {
  MyTag tag;
  tag.SetSimpleValue(0x56);

  Ptr<Packet> p = Create<Packet>(100);
  p->AddPacketTag(tag);

  Ptr<Packet> aCopy = p->Copy();

  MyTag tagCopy;
  p->PeekPacketTag(tagCopy);

  NS_ASSERT(tagCopy.GetSimpleValue() == tag.GetSimpleValue());

  aCopy->PrintPacketTags(std::cout);
  std::cout << std::endl;

  return 0;
}
