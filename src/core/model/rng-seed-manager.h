
#ifndef RNG_SEED_MANAGER_H
#define RNG_SEED_MANAGER_H

#include <stdint.h>

namespace ns3 {

class RngSeedManager {
public:
  static void SetSeed(uint32_t seed);

  static uint32_t GetSeed();

  static void SetRun(uint64_t run);
  static uint64_t GetRun();

  static uint64_t GetNextStreamIndex();
};

typedef RngSeedManager SeedManager;

} // namespace ns3

#endif
