
#include "hash-function.h"

#include "log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("HashFunction");

namespace Hash {

uint64_t Implementation::GetHash64(const char *buffer, const std::size_t size) {
  NS_LOG_WARN("64-bit hash requested, only 32-bit implementation available");
  return GetHash32(buffer, size);
}

} // namespace Hash

} // namespace ns3
