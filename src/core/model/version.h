
#ifndef BUILD_VERSION_H_
#define BUILD_VERSION_H_

#include "int64x64.h"

#include <string>

namespace ns3 {

class Version {
public:
  static std::string VersionTag();

  static std::string ClosestAncestorTag();

  static uint32_t Major();

  static uint32_t Minor();

  static uint32_t Patch();

  static std::string ReleaseCandidate();

  static uint32_t TagDistance();

  static bool DirtyWorkingTree();

  static std::string CommitHash();

  static std::string BuildProfile();

  static std::string ShortVersion();

  static std::string BuildSummary();

  static std::string LongVersion();
};

} // namespace ns3

#endif
