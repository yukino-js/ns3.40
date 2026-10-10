

#include "ns3/core-module.h"
#include "ns3/hash.h"

#include <algorithm>
#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Hasher");

namespace Hash {

namespace Example {

class Collider {
public:
  std::string m_name;
  Hasher m_hash;

  enum Bits { Bits32, Bits64 };

  Collider(const std::string name, Hasher hash, const Bits bits)
      : m_name(name), m_hash(hash), m_bits(bits) {}

  bool Add(const std::string phrase) {
    uint64_t h = GetHash(phrase);

    if (m_dict.count(h) > 0) {
      if (phrase == m_dict[h]) {
        return false;
      }

      if (m_dict[h] < phrase) {
        m_coll.emplace_back(h, phrase);
      } else {
        m_coll.emplace_back(h, m_dict[h]);
        m_dict[h] = phrase;
      }
    } else {
      m_dict.insert(std::make_pair(h, phrase));
    }
    return true;
  }

  std::string GetName() const {
    std::string name = m_name;

    switch (m_bits) {
    case Bits32:
      name += " (32-bit version)";
      break;
    case Bits64:
      name += " (64-bit version)";
      break;
    default:
      name += " (unknown!?!)";
    }
    return name;
  }

  void Report() const {
    std::cout << std::endl;

    std::cout << GetName() << ": " << m_coll.size()
              << " collisions:" << std::endl;
    for (const auto &collision : m_coll) {
      uint64_t h = collision.first;

      std::cout << std::setfill('0') << std::hex << std::setw(8) << h
                << std::dec << std::setfill(' ') << "  " << std::setw(20)
                << std::left << m_dict.find(h)->second << collision.second
                << std::right << std::endl;
    }
  }

private:
  uint64_t GetHash(const std::string phrase) {
    m_hash.clear();
    uint64_t h = 0;

    if (m_bits == Bits32) {
      h = m_hash.GetHash32(phrase);
    } else {
      h = m_hash.GetHash64(phrase);
    }
    return h;
  }

  Bits m_bits;

  typedef std::map<uint64_t, std::string> hashdict_t;

  hashdict_t m_dict;

  typedef std::vector<std::pair<uint64_t, std::string>> collision_t;

  collision_t m_coll;
};

class Dictionary {
public:
  Dictionary() : m_nphrases(0) { m_words.reserve(320000); }

  void Add(Collider c) { m_hashes.push_back(c); }

  void Add(const std::string phrase) {
    if (phrase.empty()) {
      return;
    }

    bool newPhrases = false;
    for (auto &collider : m_hashes) {
      newPhrases |= collider.Add(phrase);
    }

    if (newPhrases) {
      ++m_nphrases;
      m_words.push_back(phrase);
    }
  }

  void ReportExpectedCollisions() const {
    long double k32 = 0xFFFFFFFF;
    auto k64 = static_cast<long double>(0xFFFFFFFFFFFFFFFFULL);

    long double n = m_nphrases;
    long double Ec32 = n * (n - 1) / (2 * k32) * (1 - (n - 2) / (3 * k32));
    long double Ec64 = n * (n - 1) / (2 * k64) * (1 - (n - 2) / (3 * k64));

    std::cout << "" << std::endl;
    std::cout << "Number of words or phrases: " << n << std::endl;
    std::cout << "Expected number of collisions: (32-bit table) " << Ec32
              << std::endl;
    std::cout << "Expected number of collisions: (64-bit table) " << Ec64
              << std::endl;
  }

  void Report() const {
    ReportExpectedCollisions();

    for (const auto &collider : m_hashes) {
      collider.Report();
    }
  }

  void TimeOne(const Collider &collider) {
    uint32_t reps = 100;
    Hasher h = collider.m_hash;
    int start = clock();
    for (const auto &word : m_words) {
      for (uint32_t i = 0; i < reps; ++i) {
        h.clear().GetHash32(word);
      }
    }
    int stop = clock();
    double delta = stop - start;
    double per = 1e9 * delta / (m_nphrases * reps * CLOCKS_PER_SEC);

    std::cout << std::left << std::setw(32) << collider.GetName() << std::right
              << std::setw(10) << m_nphrases << std::setw(10) << reps
              << std::setw(10) << stop - start << std::setw(12) << per
              << std::endl;
  }

  void Time() {
    std::cout << "" << std::endl;
    std::cout << std::left << std::setw(32) << "Hash timing" << std::right
              << std::setw(10) << "Phrases" << std::setw(10) << "Reps"
              << std::setw(10) << "Ticks" << std::setw(12) << "ns/hash"
              << std::endl;

    for (const auto &collider : m_hashes) {
      TimeOne(collider);
    }
  }

private:
  unsigned long m_nphrases;
  std::vector<Collider> m_hashes;
  std::vector<std::string> m_words;
};

class DictFiles {
public:
  bool Add(const std::string &file) {
    if (std::find(m_files.begin(), m_files.end(), file) == m_files.end()) {
      m_files.push_back(file);
    }

    return true;
  }

  static std::string GetDefault() { return "/usr/share/dict/words"; }

  void ReadInto(Dictionary &dict) {
    if (m_files.empty()) {
      Add(GetDefault());
    }

    std::cout << "Hashing the dictionar" << (m_files.size() == 1 ? "y" : "ies")
              << std::endl;

    for (const auto &dictFile : m_files) {
      std::cout << "Dictionary file: " << dictFile << std::endl;

      std::ifstream dictStream;
      dictStream.open(dictFile);
      if (!dictStream.is_open()) {
        std::cerr << "Failed to open dictionary file."
                  << "'" << dictFile << "'" << std::endl;
        continue;
      }

      while (dictStream.good()) {
        std::string phrase;
        getline(dictStream, phrase);
        dict.Add(phrase);
      }

      dictStream.close();
    }
  }

private:
  std::vector<std::string> m_files;
};

} // namespace Example

} // namespace Hash

} // namespace ns3

using namespace ns3;
using namespace ns3::Hash::Example;

int main(int argc, char *argv[]) {
  std::cout << std::endl;
  std::cout << "Hasher" << std::endl;

  bool timing = false;
  DictFiles files;

  CommandLine cmd(__FILE__);
  cmd.Usage("Find hash collisions in the dictionary.");
  cmd.AddValue("dict", "Dictionary file to hash",
               MakeCallback(&DictFiles::Add, &files), DictFiles::GetDefault());

  cmd.AddValue("time", "Run timing test", timing);
  cmd.Parse(argc, argv);

  Dictionary dict;
  dict.Add(Collider("FNV1a", Hasher(Create<Hash::Function::Fnv1a>()),
                    Collider::Bits32));
  dict.Add(Collider("FNV1a", Hasher(Create<Hash::Function::Fnv1a>()),
                    Collider::Bits64));

  dict.Add(Collider("Murmur3", Hasher(Create<Hash::Function::Murmur3>()),
                    Collider::Bits32));
  dict.Add(Collider("Murmur3", Hasher(Create<Hash::Function::Murmur3>()),
                    Collider::Bits64));

  files.ReadInto(dict);

  dict.Report();

  if (timing) {
    dict.Time();
  }

  return 0;
}
