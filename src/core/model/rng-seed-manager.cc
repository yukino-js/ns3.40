
#include "rng-seed-manager.h"

#include "attribute-helper.h"
#include "config.h"
#include "global-value.h"
#include "log.h"
#include "uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RngSeedManager");

static uint64_t g_nextStreamIndex = 0;
static ns3::GlobalValue g_rngSeed("RngSeed",
                                  "The global seed of all rng streams",
                                  ns3::UintegerValue(1),
                                  ns3::MakeUintegerChecker<uint32_t>());
static ns3::GlobalValue g_rngRun("RngRun",
                                 "The substream index used for all streams",
                                 ns3::UintegerValue(1),
                                 ns3::MakeUintegerChecker<uint64_t>());

uint32_t RngSeedManager::GetSeed() {
  NS_LOG_FUNCTION_NOARGS();
  UintegerValue seedValue;
  g_rngSeed.GetValue(seedValue);
  return static_cast<uint32_t>(seedValue.Get());
}

void RngSeedManager::SetSeed(uint32_t seed) {
  NS_LOG_FUNCTION(seed);
  Config::SetGlobal("RngSeed", UintegerValue(seed));
}

void RngSeedManager::SetRun(uint64_t run) {
  NS_LOG_FUNCTION(run);
  Config::SetGlobal("RngRun", UintegerValue(run));
}

uint64_t RngSeedManager::GetRun() {
  NS_LOG_FUNCTION_NOARGS();
  UintegerValue value;
  g_rngRun.GetValue(value);
  uint64_t run = value.Get();
  return run;
}

uint64_t RngSeedManager::GetNextStreamIndex() {
  NS_LOG_FUNCTION_NOARGS();
  uint64_t next = g_nextStreamIndex;
  g_nextStreamIndex++;
  return next;
}

} // namespace ns3
