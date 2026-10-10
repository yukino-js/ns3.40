
#include "ns3/header-serialization-test.h"
#include "ns3/log.h"
#include "ns3/supported-rates.h"
#include "ns3/wifi-information-element.h"
#include "ns3/wifi-mgt-header.h"

#include <list>
#include <numeric>
#include <optional>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiIeFragmentTest");

static bool g_extendedIe = false;

class TestWifiSubElement : public WifiInformationElement {
public:
  TestWifiSubElement() = default;

  TestWifiSubElement(uint16_t count, uint8_t start);

  WifiInformationElementId ElementId() const override;

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  std::list<uint8_t> m_content;
};

TestWifiSubElement::TestWifiSubElement(uint16_t count, uint8_t start) {
  NS_LOG_FUNCTION(this << count << +start);
  m_content.resize(count);
  std::iota(m_content.begin(), m_content.end(), start);
}

WifiInformationElementId TestWifiSubElement::ElementId() const { return 0; }

uint16_t TestWifiSubElement::GetInformationFieldSize() const {
  return m_content.size();
}

void TestWifiSubElement::SerializeInformationField(
    Buffer::Iterator start) const {
  NS_LOG_FUNCTION(this);
  for (const auto &byte : m_content) {
    start.WriteU8(byte);
  }
}

uint16_t TestWifiSubElement::DeserializeInformationField(Buffer::Iterator start,
                                                         uint16_t length) {
  NS_LOG_FUNCTION(this << length);
  m_content.clear();
  for (uint16_t i = 0; i < length; i++) {
    m_content.push_back(start.ReadU8());
  }
  return length;
}

class TestWifiInformationElement : public WifiInformationElement {
public:
  TestWifiInformationElement(bool extended);

  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;
  void AddSubelement(TestWifiSubElement &&subelement);

private:
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  bool m_extended;
  std::list<TestWifiSubElement> m_content;
};

TestWifiInformationElement::TestWifiInformationElement(bool extended)
    : m_extended(extended) {
  NS_LOG_FUNCTION(this << extended);
}

WifiInformationElementId TestWifiInformationElement::ElementId() const {
  return m_extended ? 255 : 2;
}

WifiInformationElementId TestWifiInformationElement::ElementIdExt() const {
  NS_ABORT_IF(!m_extended);
  return 32;
}

void TestWifiInformationElement::AddSubelement(
    TestWifiSubElement &&subelement) {
  NS_LOG_FUNCTION(this);
  m_content.push_back(std::move(subelement));
}

uint16_t TestWifiInformationElement::GetInformationFieldSize() const {
  uint16_t size = (m_extended ? 1 : 0);
  for (const auto &subelement : m_content) {
    size += subelement.GetSerializedSize();
  }
  return size;
}

void TestWifiInformationElement::SerializeInformationField(
    Buffer::Iterator start) const {
  NS_LOG_FUNCTION(this);
  for (const auto &subelement : m_content) {
    start = subelement.Serialize(start);
  }
}

uint16_t
TestWifiInformationElement::DeserializeInformationField(Buffer::Iterator start,
                                                        uint16_t length) {
  NS_LOG_FUNCTION(this << length);

  Buffer::Iterator i = start;
  uint16_t count = 0;

  while (count < length) {
    TestWifiSubElement subelement;
    i = subelement.Deserialize(i);
    m_content.push_back(std::move(subelement));
    count = i.GetDistanceFrom(start);
  }
  return count;
}

class TestHeader
    : public WifiMgtHeader<
          TestHeader, std::tuple<std::vector<TestWifiInformationElement>>> {
  friend class WifiMgtHeader<
      TestHeader, std::tuple<std::vector<TestWifiInformationElement>>>;

public:
  ~TestHeader() override = default;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

private:
  void
  InitForDeserialization(std::optional<TestWifiInformationElement> &optElem);
};

NS_OBJECT_ENSURE_REGISTERED(TestHeader);

ns3::TypeId TestHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::TestHeader")
                          .SetParent<Header>()
                          .SetGroupName("Wifi")
                          .AddConstructor<TestHeader>();
  return tid;
}

TypeId TestHeader::GetInstanceTypeId() const { return GetTypeId(); }

void TestHeader::InitForDeserialization(
    std::optional<TestWifiInformationElement> &optElem) {
  optElem.emplace(g_extendedIe);
}

class WifiIeFragmentationTest : public HeaderSerializationTestCase {
public:
  WifiIeFragmentationTest(bool extended);
  ~WifiIeFragmentationTest() override = default;

  Buffer SerializeIntoBuffer(const WifiInformationElement &element);

  void CheckSerializedByte(const Buffer &buffer, uint32_t position,
                           uint8_t value);

private:
  void DoRun() override;

  bool m_extended;
};

WifiIeFragmentationTest ::WifiIeFragmentationTest(bool extended)
    : HeaderSerializationTestCase(
          "Check fragmentation of Information Elements"),
      m_extended(extended) {}

Buffer WifiIeFragmentationTest::SerializeIntoBuffer(
    const WifiInformationElement &element) {
  Buffer buffer;
  buffer.AddAtStart(element.GetSerializedSize());
  element.Serialize(buffer.Begin());
  return buffer;
}

void WifiIeFragmentationTest::CheckSerializedByte(const Buffer &buffer,
                                                  uint32_t position,
                                                  uint8_t value) {
  Buffer::Iterator it = buffer.Begin();
  it.Next(position);
  uint8_t byte = it.ReadU8();
  NS_TEST_EXPECT_MSG_EQ(+byte, +value, "Unexpected byte at pos=" << position);
}

void WifiIeFragmentationTest::DoRun() {
  uint16_t limit = m_extended ? 254 : 255;

  TestHeader header;
  g_extendedIe = m_extended;

  uint16_t sub01Size = 50;
  uint16_t sub02Size = limit - sub01Size;

  auto sub01 = TestWifiSubElement(sub01Size - 2, 53);
  auto sub02 = TestWifiSubElement(sub02Size - 2, 26);

  auto testIe = TestWifiInformationElement(m_extended);
  testIe.AddSubelement(std::move(sub01));
  testIe.AddSubelement(std::move(sub02));

  {
    Buffer buffer = SerializeIntoBuffer(testIe);
    CheckSerializedByte(buffer, 1, 255);
    if (m_extended) {
      CheckSerializedByte(buffer, 2, testIe.ElementIdExt());
    }
    CheckSerializedByte(buffer, (m_extended ? 3 : 2),
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + 1, sub01Size - 2);
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size,
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size + 1,
                        sub02Size - 2);
  }

  header.Get<TestWifiInformationElement>().push_back(std::move(testIe));
  uint32_t expectedHdrSize = 2 + 255;
  NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize(), expectedHdrSize,
                        "Unexpected header size");
  TestHeaderSerialization(header);

  sub01Size = 65;
  sub02Size = limit + 1 - sub01Size;

  sub01 = TestWifiSubElement(sub01Size - 2, 47);
  sub02 = TestWifiSubElement(sub02Size - 2, 71);

  testIe = TestWifiInformationElement(m_extended);
  testIe.AddSubelement(std::move(sub01));
  testIe.AddSubelement(std::move(sub02));

  {
    Buffer buffer = SerializeIntoBuffer(testIe);
    CheckSerializedByte(buffer, 1, 255);
    if (m_extended) {
      CheckSerializedByte(buffer, 2, testIe.ElementIdExt());
    }
    CheckSerializedByte(buffer, (m_extended ? 3 : 2),
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + 1, sub01Size - 2);
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size,
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size + 1,
                        sub02Size - 2);
    CheckSerializedByte(buffer, 2 + 255, IE_FRAGMENT);
    CheckSerializedByte(buffer, 2 + 255 + 1, 1);
  }

  header.Get<TestWifiInformationElement>().push_back(std::move(testIe));
  expectedHdrSize += 2 + 255 + 2 + 1;
  NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize(), expectedHdrSize,
                        "Unexpected header size");
  TestHeaderSerialization(header);

  sub01Size = 200;
  sub02Size = 200;
  uint16_t sub03Size = limit + 255 - sub01Size - sub02Size;

  sub01 = TestWifiSubElement(sub01Size - 2, 16);
  sub02 = TestWifiSubElement(sub02Size - 2, 83);
  auto sub03 = TestWifiSubElement(sub03Size - 2, 98);

  testIe = TestWifiInformationElement(m_extended);
  testIe.AddSubelement(std::move(sub01));
  testIe.AddSubelement(std::move(sub02));
  testIe.AddSubelement(std::move(sub03));

  {
    Buffer buffer = SerializeIntoBuffer(testIe);
    CheckSerializedByte(buffer, 1, 255);
    if (m_extended) {
      CheckSerializedByte(buffer, 2, testIe.ElementIdExt());
    }
    CheckSerializedByte(buffer, (m_extended ? 3 : 2),
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + 1, sub01Size - 2);
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size,
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size + 1,
                        sub02Size - 2);
    CheckSerializedByte(buffer, 2 + 255, IE_FRAGMENT);
    CheckSerializedByte(buffer, 2 + 255 + 1, 255);
  }

  header.Get<TestWifiInformationElement>().push_back(std::move(testIe));
  expectedHdrSize += 2 + 255 + 2 + 255;
  NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize(), expectedHdrSize,
                        "Unexpected header size");
  TestHeaderSerialization(header);

  sub01Size = 200;
  sub02Size = 200;
  sub03Size = limit + 255 + 1 - sub01Size - sub02Size;

  sub01 = TestWifiSubElement(sub01Size - 2, 20);
  sub02 = TestWifiSubElement(sub02Size - 2, 77);
  sub03 = TestWifiSubElement(sub03Size - 2, 14);

  testIe = TestWifiInformationElement(m_extended);
  testIe.AddSubelement(std::move(sub01));
  testIe.AddSubelement(std::move(sub02));
  testIe.AddSubelement(std::move(sub03));

  {
    Buffer buffer = SerializeIntoBuffer(testIe);
    CheckSerializedByte(buffer, 1, 255);
    if (m_extended) {
      CheckSerializedByte(buffer, 2, testIe.ElementIdExt());
    }
    CheckSerializedByte(buffer, (m_extended ? 3 : 2),
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + 1, sub01Size - 2);
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size,
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + sub01Size + 1,
                        sub02Size - 2);
    CheckSerializedByte(buffer, 2 + 255, IE_FRAGMENT);
    CheckSerializedByte(buffer, 2 + 255 + 1, 255);
    CheckSerializedByte(buffer,
                        (m_extended ? 3 : 2) + sub01Size + 2 + sub02Size,
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer,
                        (m_extended ? 3 : 2) + sub01Size + 2 + sub02Size + 1,
                        sub03Size - 2);
    CheckSerializedByte(buffer, 2 * (2 + 255), IE_FRAGMENT);
    CheckSerializedByte(buffer, 2 * (2 + 255) + 1, 1);
  }

  header.Get<TestWifiInformationElement>().push_back(std::move(testIe));
  expectedHdrSize += 2 + 255 + 2 + 255 + 2 + 1;
  NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize(), expectedHdrSize,
                        "Unexpected header size");
  TestHeaderSerialization(header);

  sub01Size = 2 + 255;

  sub01 = TestWifiSubElement(sub01Size - 2, 47);

  testIe = TestWifiInformationElement(m_extended);
  testIe.AddSubelement(std::move(sub01));

  {
    Buffer buffer = SerializeIntoBuffer(testIe);
    CheckSerializedByte(buffer, 1, 255);
    if (m_extended) {
      CheckSerializedByte(buffer, 2, testIe.ElementIdExt());
    }
    CheckSerializedByte(buffer, (m_extended ? 3 : 2),
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + 1, sub01Size - 2);
    CheckSerializedByte(buffer, 2 + 255, IE_FRAGMENT);
    CheckSerializedByte(buffer, 2 + 255 + 1, (m_extended ? 3 : 2));
  }

  header.Get<TestWifiInformationElement>().push_back(std::move(testIe));
  expectedHdrSize += 2 + 255 + 2 + (m_extended ? 3 : 2);
  NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize(), expectedHdrSize,
                        "Unexpected header size");
  TestHeaderSerialization(header);

  sub01Size = 2 + 256;

  sub01 = TestWifiSubElement(sub01Size - 2, 84);

  testIe = TestWifiInformationElement(m_extended);
  testIe.AddSubelement(std::move(sub01));

  {
    Buffer buffer = SerializeIntoBuffer(testIe);
    CheckSerializedByte(buffer, 1, 255);
    if (m_extended) {
      CheckSerializedByte(buffer, 2, testIe.ElementIdExt());
    }
    CheckSerializedByte(buffer, (m_extended ? 3 : 2),
                        TestWifiSubElement().ElementId());
    CheckSerializedByte(buffer, (m_extended ? 3 : 2) + 1, 255);
    CheckSerializedByte(buffer, 2 + 255, IE_FRAGMENT);
    CheckSerializedByte(buffer, 2 + 255 + 1, (m_extended ? 6 : 5));
    CheckSerializedByte(buffer, 2 + 255 + 2 + (m_extended ? 3 : 2),
                        IE_FRAGMENT);
    CheckSerializedByte(buffer, 2 + 255 + 2 + (m_extended ? 3 : 2) + 1, 1);
  }

  header.Get<TestWifiInformationElement>().push_back(std::move(testIe));
  expectedHdrSize += 2 + 255 + 2 + (m_extended ? 6 : 5);
  NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize(), expectedHdrSize,
                        "Unexpected header size");
  TestHeaderSerialization(header);
}

class WifiIeFragmentationTestSuite : public TestSuite {
public:
  WifiIeFragmentationTestSuite();
};

WifiIeFragmentationTestSuite::WifiIeFragmentationTestSuite()
    : TestSuite("wifi-ie-fragment", UNIT) {
  AddTestCase(new WifiIeFragmentationTest(false), TestCase::QUICK);
  AddTestCase(new WifiIeFragmentationTest(true), TestCase::QUICK);
}

static WifiIeFragmentationTestSuite g_wifiIeFragmentationTestSuite;
