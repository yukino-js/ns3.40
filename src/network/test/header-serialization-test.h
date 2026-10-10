
#ifndef NS3_TEST_HDR_SERIALIZE_H
#define NS3_TEST_HDR_SERIALIZE_H

#include "ns3/buffer.h"
#include "ns3/test.h"

namespace ns3 {

class HeaderSerializationTestCase : public TestCase {
protected:
  HeaderSerializationTestCase(std::string name) : TestCase(name) {}

public:
  template <typename T, typename... Args>
  void TestHeaderSerialization(const T &hdr, Args &&...args);
};

} // namespace ns3

namespace ns3 {

template <typename T, typename... Args>
void HeaderSerializationTestCase::TestHeaderSerialization(const T &hdr,
                                                          Args &&...args) {
  Buffer buffer;
  buffer.AddAtStart(hdr.GetSerializedSize());
  hdr.Serialize(buffer.Begin());

  T otherHdr(std::forward<Args>(args)...);
  otherHdr.Deserialize(buffer.Begin());
  Buffer otherBuffer;
  otherBuffer.AddAtStart(otherHdr.GetSerializedSize());
  otherHdr.Serialize(otherBuffer.Begin());

  NS_TEST_ASSERT_MSG_EQ(buffer.GetSize(), otherBuffer.GetSize(),
                        "Size of buffers differs");

  Buffer::Iterator bufferIterator = buffer.Begin();
  Buffer::Iterator otherBufferIterator = otherBuffer.Begin();
  for (uint32_t j = 0; j < buffer.GetSize(); j++) {
    uint8_t bufferVal = bufferIterator.ReadU8();
    uint8_t otherBufferVal = otherBufferIterator.ReadU8();
    NS_TEST_EXPECT_MSG_EQ(+bufferVal, +otherBufferVal,
                          "Serialization -> Deserialization -> Serialization "
                              << "is different than Serialization at byte "
                              << j);
  }
}

} // namespace ns3

#endif
