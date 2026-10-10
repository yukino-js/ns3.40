
#ifndef HASH_MURMUR3_H
#define HASH_MURMUR3_H

#include "hash-function.h"

namespace ns3 {

namespace Hash {

namespace Function {

class Murmur3 : public Implementation {
public:
  Murmur3();
  uint32_t GetHash32(const char *buffer, const std::size_t size) override;
  uint64_t GetHash64(const char *buffer, const std::size_t size) override;
  void clear() override;

private:
  static constexpr auto SEED{0x8BADF00D};

  uint32_t m_hash32;
  std::size_t m_size32;

  uint64_t m_hash64[2];
  std::size_t m_size64;
};

} // namespace Function

} // namespace Hash

} // namespace ns3

#endif
