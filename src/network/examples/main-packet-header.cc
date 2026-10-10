
#include "ns3/header.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"

#include <iostream>

using namespace ns3;

class MyHeader : public Header {
public:
  MyHeader();
  ~MyHeader() override;

  void SetData(uint16_t data);
  uint16_t GetData() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  uint32_t GetSerializedSize() const override;

private:
  uint16_t m_data;
};

MyHeader::MyHeader() {}

MyHeader::~MyHeader() {}

TypeId MyHeader::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::MyHeader").SetParent<Header>().AddConstructor<MyHeader>();
  return tid;
}

TypeId MyHeader::GetInstanceTypeId() const { return GetTypeId(); }

void MyHeader::Print(std::ostream &os) const { os << "data=" << m_data; }

uint32_t MyHeader::GetSerializedSize() const { return 2; }

void MyHeader::Serialize(Buffer::Iterator start) const {
  start.WriteHtonU16(m_data);
}

uint32_t MyHeader::Deserialize(Buffer::Iterator start) {
  m_data = start.ReadNtohU16();

  return 2;
}

void MyHeader::SetData(uint16_t data) { m_data = data; }

uint16_t MyHeader::GetData() const { return m_data; }

int main(int argc, char *argv[]) {
  Packet::EnablePrinting();

  MyHeader sourceHeader;
  sourceHeader.SetData(2);

  Ptr<Packet> p = Create<Packet>();

  p->AddHeader(sourceHeader);

  p->Print(std::cout);
  std::cout << std::endl;

  MyHeader destinationHeader;
  p->RemoveHeader(destinationHeader);

  NS_ASSERT(sourceHeader.GetData() == destinationHeader.GetData());

  return 0;
}
