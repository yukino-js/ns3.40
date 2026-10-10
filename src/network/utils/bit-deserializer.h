
#ifndef BITDESERIALIZER_H_
#define BITDESERIALIZER_H_

#include <cstdint>
#include <deque>
#include <vector>

namespace ns3 {

class BitDeserializer {
public:
  BitDeserializer();

  void PushBytes(std::vector<uint8_t> bytes);

  void PushBytes(uint8_t *bytes, uint32_t size);

  void PushByte(uint8_t byte);

  uint64_t GetBits(uint8_t size);

private:
  void PrepareDeserialization();

  std::deque<bool> m_blob;
  std::vector<uint8_t> m_bytesBlob;
  bool m_deserializing;
};

} // namespace ns3

#endif
