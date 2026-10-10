
#ifndef HASHFUNCTION_H
#define HASHFUNCTION_H

#include "simple-ref-count.h"

#include <cstring>

namespace ns3 {

namespace Hash {

class Implementation : public SimpleRefCount<Implementation> {
public:
  virtual uint32_t GetHash32(const char *buffer, const std::size_t size) = 0;
  virtual uint64_t GetHash64(const char *buffer, const std::size_t size);
  virtual void clear() = 0;

  Implementation() {}

  virtual ~Implementation() {}
};

typedef uint32_t (*Hash32Function_ptr)(const char *, const std::size_t);
typedef uint64_t (*Hash64Function_ptr)(const char *, const std::size_t);

namespace Function {

class Hash32 : public Implementation {
public:
  Hash32(Hash32Function_ptr hp) : m_fp(hp) {}

  uint32_t GetHash32(const char *buffer, const std::size_t size) override {
    return (*m_fp)(buffer, size);
  }

  void clear() override {}

private:
  Hash32Function_ptr m_fp;
};

class Hash64 : public Implementation {
public:
  Hash64(Hash64Function_ptr hp) : m_fp(hp) {}

  uint64_t GetHash64(const char *buffer, const std::size_t size) override {
    return (*m_fp)(buffer, size);
  }

  uint32_t GetHash32(const char *buffer, const std::size_t size) override {
    uint32_t hash32;
    uint64_t hash64 = GetHash64(buffer, size);

    memcpy(&hash32, &hash64, sizeof(hash32));
    return hash32;
  }

  void clear() override {}

private:
  Hash64Function_ptr m_fp;
};

} // namespace Function

} // namespace Hash

} // namespace ns3

#endif
