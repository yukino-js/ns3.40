
#include "ns3/hash.h"
#include "ns3/test.h"

#include <iomanip>
#include <string>

namespace ns3 {

namespace tests {

class HashTestCase : public TestCase {
public:
  HashTestCase(const std::string name);
  ~HashTestCase() override;

protected:
  void Check(const std::string hashName, const uint32_t hash);
  void Check(const std::string hashName, const uint64_t hash);

  std::string key;
  uint32_t hash32Reference;
  uint64_t hash64Reference;

private:
  void Check(const std::string hashName, const int bits, const uint64_t hash);
  void DoRun() override;
};

HashTestCase::HashTestCase(const std::string name)
    : TestCase(name), key("The quick brown fox jumped over the lazy dogs.") {}

HashTestCase::~HashTestCase() {}

void HashTestCase::Check(const std::string hashName, const uint32_t hash) {
  Check(hashName, 32, hash);
}

void HashTestCase::Check(const std::string hashName, const uint64_t hash) {
  Check(hashName, 64, hash);
}

void HashTestCase::Check(std::string hashName, int bits, uint64_t hash) {
  int w;
  std::string type;
  uint64_t hashRef;

  if (bits == 32) {
    w = 8;
    type = "Hash32";
    hashRef = hash32Reference;
  } else {
    w = 16;
    type = "Hash64";
    hashRef = hash64Reference;
  }

  std::cout << GetName() << "checking " << hashName << " " << bits
            << "-bit result...";
  NS_TEST_EXPECT_MSG_EQ(hash, hashRef,
                        hashName << " " << type << " produced " << std::hex
                                 << std::setw(w) << hash << ", expected "
                                 << std::hex << std::setw(w) << hashRef
                                 << std::dec);
  std::cout << std::hex << std::setw(w) << hash << ", ok" << std::dec
            << std::endl;
}

void HashTestCase::DoRun() {}

class DefaultHashTestCase : public HashTestCase {
public:
  DefaultHashTestCase();
  ~DefaultHashTestCase() override;

private:
  void DoRun() override;
};

DefaultHashTestCase::DefaultHashTestCase() : HashTestCase("DefaultHash: ") {}

DefaultHashTestCase::~DefaultHashTestCase() {}

void DefaultHashTestCase::DoRun() {
  std::cout << GetName() << "checking with key: \"" << key << "\"" << std::endl;

  hash32Reference = 0x463d70e2;
  Check("default", Hash32(key));

  hash64Reference = 0xa750412079d53e04ULL;
  Check("default", Hash64(key));
}

class Fnv1aTestCase : public HashTestCase {
public:
  Fnv1aTestCase();
  ~Fnv1aTestCase() override;

private:
  void DoRun() override;
};

Fnv1aTestCase::Fnv1aTestCase() : HashTestCase("Fnv1a: ") {}

Fnv1aTestCase::~Fnv1aTestCase() {}

void Fnv1aTestCase::DoRun() {
  Hasher hasher = Hasher(Create<Hash::Function::Fnv1a>());
  hash32Reference = 0xa3fc0d6d;
  Check("FNV1a", hasher.clear().GetHash32(key));

  hash64Reference = 0x88f6cdbe0a31098dULL;
  Check("FNV1a", hasher.clear().GetHash64(key));
}

class Murmur3TestCase : public HashTestCase {
public:
  Murmur3TestCase();
  ~Murmur3TestCase() override;

private:
  void DoRun() override;
};

Murmur3TestCase::Murmur3TestCase() : HashTestCase("Murmur3: ") {}

Murmur3TestCase::~Murmur3TestCase() {}

void Murmur3TestCase::DoRun() {
  Hasher hasher = Hasher(Create<Hash::Function::Murmur3>());
  hash32Reference = 0x463d70e2;
  Check("murmur3", hasher.clear().GetHash32(key));

  hash64Reference = 0xa750412079d53e04ULL;
  Check("murmur3", hasher.clear().GetHash64(key));
}

uint16_t gnu_sum(const char *buffer, const std::size_t size) {
  const char *p = buffer;
  const char *const pend = p + size;

  uint16_t checksum = 0;

  while (p != pend) {
    checksum = (checksum >> 1) + ((checksum & 1) << 15);
    checksum += *p++;
  }
  return checksum;
}

uint32_t gnu_sum32(const char *buffer, const std::size_t size) {
  uint32_t h = gnu_sum(buffer, size);
  return (uint32_t)((h << 16) + h);
}

uint64_t gnu_sum64(const char *buffer, const std::size_t size) {
  uint64_t h = gnu_sum32(buffer, size);
  return (uint64_t)((h << 32) + h);
}

class Hash32FunctionPtrTestCase : public HashTestCase {
public:
  Hash32FunctionPtrTestCase();
  ~Hash32FunctionPtrTestCase() override;

private:
  void DoRun() override;
};

Hash32FunctionPtrTestCase::Hash32FunctionPtrTestCase()
    : HashTestCase("Hash32FunctionPtr: ") {}

Hash32FunctionPtrTestCase::~Hash32FunctionPtrTestCase() {}

void Hash32FunctionPtrTestCase::DoRun() {
  Hasher hasher = Hasher(Create<Hash::Function::Hash32>(&gnu_sum32));
  hash32Reference = 0x41264126;
  Check("gnu_sum32", hasher.clear().GetHash32(key));
}

class Hash64FunctionPtrTestCase : public HashTestCase {
public:
  Hash64FunctionPtrTestCase();
  ~Hash64FunctionPtrTestCase() override;

private:
  void DoRun() override;
};

Hash64FunctionPtrTestCase::Hash64FunctionPtrTestCase()
    : HashTestCase("Hash64FunctionPtr: ") {}

Hash64FunctionPtrTestCase::~Hash64FunctionPtrTestCase() {}

void Hash64FunctionPtrTestCase::DoRun() {
  Hasher hasher = Hasher(Create<Hash::Function::Hash64>(&gnu_sum64));
  hash64Reference = 0x4126412641264126ULL;
  Check("gnu_sum64", hasher.clear().GetHash64(key));
}

class IncrementalTestCase : public HashTestCase {
public:
  IncrementalTestCase();
  ~IncrementalTestCase() override;

private:
  void DoRun() override;
  void DoHash(const std::string name, Hasher hasher);
  std::string key1;
  std::string key2;
  std::string key12;
};

IncrementalTestCase::IncrementalTestCase() : HashTestCase("Incremental: ") {}

IncrementalTestCase::~IncrementalTestCase() {}

void IncrementalTestCase::DoHash(const std::string name, Hasher hasher) {
  hash32Reference = hasher.clear().GetHash32(key12);
  hasher.clear().GetHash32(key1);
  Check(name, hasher.GetHash32(key2));

  hash64Reference = hasher.clear().GetHash64(key12);
  hasher.clear().GetHash64(key1);
  Check(name, hasher.GetHash64(key2));
}

void IncrementalTestCase::DoRun() {
  key1 = "The quick brown ";
  key2 = "Incremental.";
  key12 = key1 + key2;

  std::cout << GetName() << "checking with key: "
            << "\"" << key1 << "\"[" << key1.size() << "] + "
            << "\"" << key2 << "\"[" << key2.size() << "]" << std::endl;
  std::cout << GetName() << "equivalent to:     "
            << "\"" << key12 << "\"[" << key12.size() << "]" << std::endl;

  DoHash("default", Hasher());
  DoHash("murmur3", Hasher(Create<Hash::Function::Murmur3>()));
  DoHash("FNV1a", Hasher(Create<Hash::Function::Fnv1a>()));
}

class HashTestSuite : public TestSuite {
public:
  HashTestSuite();
};

HashTestSuite::HashTestSuite() : TestSuite("hash") {
  AddTestCase(new DefaultHashTestCase);
  AddTestCase(new Murmur3TestCase);
  AddTestCase(new Fnv1aTestCase);
  AddTestCase(new IncrementalTestCase);
  AddTestCase(new Hash32FunctionPtrTestCase);
  AddTestCase(new Hash64FunctionPtrTestCase);
}

static HashTestSuite g_hashTestSuite;

} // namespace tests

} // namespace ns3
