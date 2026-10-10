
#ifndef BITSERIALIZER_H_
#define BITSERIALIZER_H_

#include <cstdint>
#include <vector>

namespace ns3 {

class BitSerializer {
public:
  BitSerializer();

  void InsertPaddingAtEnd(bool padAtEnd);

  void PushBits(uint64_t value, uint8_t significantBits);

  std::vector<uint8_t> GetBytes();

  uint8_t GetBytes(uint8_t *buffer, uint32_t size);

private:
  void PadAtStart();

  void PadAtEnd();

  std::vector<bool> m_blob;
  bool m_padAtEnd;
};

} // namespace ns3

#endif
