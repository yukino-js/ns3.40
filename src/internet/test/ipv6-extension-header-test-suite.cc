

#include "ns3/ipv6-extension-header.h"
#include "ns3/ipv6-option-header.h"
#include "ns3/test.h"

using namespace ns3;

class TestEmptyOptionField : public TestCase {
public:
  TestEmptyOptionField() : TestCase("TestEmptyOptionField") {}

  void DoRun() override {
    Ipv6ExtensionDestinationHeader header;
    NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize() % 8, 0,
                          "length of extension header is not a multiple of 8");

    Buffer buf;
    buf.AddAtStart(header.GetSerializedSize());
    header.Serialize(buf.Begin());

    const uint8_t *data = buf.PeekData();
    NS_TEST_EXPECT_MSG_EQ(*(data + 2), 1, "padding is missing");
  }
};

class OptionWithoutAlignmentHeader : public Ipv6OptionHeader {
public:
  static const uint8_t TYPE = 42;

  uint32_t GetSerializedSize() const override { return 4; }

  void Serialize(Buffer::Iterator start) const override {
    start.WriteU8(TYPE);
    start.WriteU8(GetSerializedSize() - 2);
    start.WriteU16(0);
  }
};

class TestOptionWithoutAlignment : public TestCase {
public:
  TestOptionWithoutAlignment() : TestCase("TestOptionWithoutAlignment") {}

  void DoRun() override {
    Ipv6ExtensionDestinationHeader header;
    OptionWithoutAlignmentHeader optionHeader;
    header.AddOption(optionHeader);

    NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize() % 8, 0,
                          "length of extension header is not a multiple of 8");

    Buffer buf;
    buf.AddAtStart(header.GetSerializedSize());
    header.Serialize(buf.Begin());

    const uint8_t *data = buf.PeekData();
    NS_TEST_EXPECT_MSG_EQ(
        *(data + 2), OptionWithoutAlignmentHeader::TYPE,
        "option without alignment is not first in header field");
  }
};

class OptionWithAlignmentHeader : public Ipv6OptionHeader {
public:
  static const uint8_t TYPE = 73;

  uint32_t GetSerializedSize() const override { return 4; }

  void Serialize(Buffer::Iterator start) const override {
    start.WriteU8(TYPE);
    start.WriteU8(GetSerializedSize() - 2);
    start.WriteU16(0);
  }

  Alignment GetAlignment() const override { return (Alignment){4, 0}; }
};

class TestOptionWithAlignment : public TestCase {
public:
  TestOptionWithAlignment() : TestCase("TestOptionWithAlignment") {}

  void DoRun() override {
    Ipv6ExtensionDestinationHeader header;
    OptionWithAlignmentHeader optionHeader;
    header.AddOption(optionHeader);
    Ipv6OptionJumbogramHeader jumboHeader;
    header.AddOption(jumboHeader);

    NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize() % 8, 0,
                          "length of extension header is not a multiple of 8");

    Buffer buf;
    buf.AddAtStart(header.GetSerializedSize());
    header.Serialize(buf.Begin());

    const uint8_t *data = buf.PeekData();
    NS_TEST_EXPECT_MSG_EQ(*(data + 2), 1, "padding is missing");
    NS_TEST_EXPECT_MSG_EQ(*(data + 4), OptionWithAlignmentHeader::TYPE,
                          "option with alignment is not padded correctly");
    NS_TEST_EXPECT_MSG_EQ(*(data + 8), 1, "padding is missing");
    NS_TEST_EXPECT_MSG_EQ(*(data + 10), jumboHeader.GetType(),
                          "option with alignment is not padded correctly");
  }
};

class TestFulfilledAlignment : public TestCase {
public:
  TestFulfilledAlignment() : TestCase("TestCorrectAlignment") {}

  void DoRun() override {
    Ipv6ExtensionDestinationHeader header;
    Ipv6OptionJumbogramHeader jumboHeader;
    header.AddOption(jumboHeader);
    OptionWithAlignmentHeader optionHeader;
    header.AddOption(optionHeader);

    NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize() % 8, 0,
                          "length of extension header is not a multiple of 8");

    Buffer buf;
    buf.AddAtStart(header.GetSerializedSize());
    header.Serialize(buf.Begin());

    const uint8_t *data = buf.PeekData();
    NS_TEST_EXPECT_MSG_EQ(*(data + 2), jumboHeader.GetType(),
                          "option with fulfilled alignment is padded anyway");
    NS_TEST_EXPECT_MSG_EQ(*(data + 8), OptionWithAlignmentHeader::TYPE,
                          "option with fulfilled alignment is padded anyway");
  }
};

class Ipv6ExtensionHeaderTestSuite : public TestSuite {
public:
  Ipv6ExtensionHeaderTestSuite() : TestSuite("ipv6-extension-header", UNIT) {
    AddTestCase(new TestEmptyOptionField, TestCase::QUICK);
    AddTestCase(new TestOptionWithoutAlignment, TestCase::QUICK);
    AddTestCase(new TestOptionWithAlignment, TestCase::QUICK);
    AddTestCase(new TestFulfilledAlignment, TestCase::QUICK);
  }
};

static Ipv6ExtensionHeaderTestSuite ipv6ExtensionHeaderTestSuite;
