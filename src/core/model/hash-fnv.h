
#ifndef HASH_FNV_H
#define HASH_FNV_H

#include "hash-function.h"

namespace ns3 {

namespace Hash {

namespace Function {

class Fnv1a : public Implementation {
public:
  Fnv1a();
  uint32_t GetHash32(const char *buffer, const size_t size) override;
  uint64_t GetHash64(const char *buffer, const size_t size) override;
  void clear() override;

private:
  static constexpr auto SEED{0x8BADF00D};

  uint32_t m_hash32;
  uint64_t m_hash64;
};

} // namespace Function

} // namespace Hash

} // namespace ns3

#endif
