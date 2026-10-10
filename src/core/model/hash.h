
#ifndef HASH_H
#define HASH_H

#include "assert.h"
#include "hash-fnv.h"
#include "hash-function.h"
#include "hash-murmur3.h"
#include "ptr.h"

#include <string>

namespace ns3 {

class Hasher {
public:
  Hasher();
  Hasher(Ptr<Hash::Implementation> hp);
  uint32_t GetHash32(const char *buffer, const std::size_t size);
  uint64_t GetHash64(const char *buffer, const std::size_t size);

  uint32_t GetHash32(const std::string s);
  uint64_t GetHash64(const std::string s);
  Hasher &clear();

private:
  Ptr<Hash::Implementation> m_impl;
};

uint32_t Hash32(const char *buffer, const std::size_t size);
uint64_t Hash64(const char *buffer, const std::size_t size);

uint32_t Hash32(const std::string s);
uint64_t Hash64(const std::string s);

} // namespace ns3

namespace ns3 {

inline uint32_t Hasher::GetHash32(const char *buffer, const std::size_t size) {
  NS_ASSERT(m_impl);
  return m_impl->GetHash32(buffer, size);
}

inline uint64_t Hasher::GetHash64(const char *buffer, const std::size_t size) {
  NS_ASSERT(m_impl);
  return m_impl->GetHash64(buffer, size);
}

inline uint32_t Hasher::GetHash32(const std::string s) {
  NS_ASSERT(m_impl);
  return m_impl->GetHash32(s.c_str(), s.size());
}

inline uint64_t Hasher::GetHash64(const std::string s) {
  NS_ASSERT(m_impl);
  return m_impl->GetHash64(s.c_str(), s.size());
}

Hasher &GetStaticHash();

inline uint32_t Hash32(const char *buffer, const std::size_t size) {
#ifdef NS3_MTP
  return Hasher().GetHash32(buffer, size);
#else
  return GetStaticHash().GetHash32(buffer, size);
#endif
}

inline uint64_t Hash64(const char *buffer, const std::size_t size) {
#ifdef NS3_MTP
  return Hasher().GetHash64(buffer, size);
#else
  return GetStaticHash().GetHash64(buffer, size);
#endif
}

inline uint32_t Hash32(const std::string s) {
#ifdef NS3_MTP
  return Hasher().GetHash32(s);
#else
  return GetStaticHash().GetHash32(s);
#endif
}

inline uint64_t Hash64(const std::string s) {
#ifdef NS3_MTP
  return Hasher().GetHash64(s);
#else
  return GetStaticHash().GetHash64(s);
#endif
}

} // namespace ns3

#endif
