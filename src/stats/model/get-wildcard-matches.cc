
#include "get-wildcard-matches.h"

#include "ns3/assert.h"

#include <string>
#include <vector>

namespace ns3 {

std::string GetWildcardMatches(const std::string &configPath,
                               const std::string &matchedPath,
                               const std::string &wildcardSeparator) {
  if (configPath == "*") {
    return matchedPath;
  }

  std::vector<std::string> nonWildcardTokens;
  std::vector<std::size_t> nonWildcardTokenPositions;

  size_t nonWildcardTokenCount;
  size_t wildcardCount = 0;

  size_t tokenStart;
  size_t asterisk = -1;
  do {
    tokenStart = asterisk + 1;
    asterisk = configPath.find('*', tokenStart);

    if (asterisk != std::string::npos) {
      wildcardCount++;
    }

    nonWildcardTokens.push_back(
        configPath.substr(tokenStart, asterisk - tokenStart));
  } while (asterisk != std::string::npos);

  if (wildcardCount == 0) {
    return "";
  }

  nonWildcardTokenCount = nonWildcardTokens.size();

  size_t i;

  size_t token;
  tokenStart = 0;
  for (i = 0; i < nonWildcardTokenCount; i++) {
    token = matchedPath.find(nonWildcardTokens[i], tokenStart);

    if (token == std::string::npos) {
      NS_ASSERT_MSG(false,
                    "Error: non-wildcard token not found in matched path");
    }

    nonWildcardTokenPositions.push_back(token);

    tokenStart = token + nonWildcardTokens[i].size();
  }

  std::string wildcardMatches = "";

  size_t wildcardMatchesSet = 0;
  size_t matchStart;
  size_t matchEnd;
  for (i = 0; i < nonWildcardTokenCount; i++) {
    matchStart = nonWildcardTokenPositions[i] + nonWildcardTokens[i].size();
    if (i != nonWildcardTokenCount - 1) {
      matchEnd = nonWildcardTokenPositions[i + 1] - 1;
    } else {
      matchEnd = matchedPath.length() - 1;
    }

    if (matchStart <= matchEnd) {
      wildcardMatches +=
          matchedPath.substr(matchStart, matchEnd - matchStart + 1);

      wildcardMatchesSet++;
      if (wildcardMatchesSet == wildcardCount) {
        break;
      } else {
        wildcardMatches += wildcardSeparator;
      }
    }
  }

  return wildcardMatches;
}

} // namespace ns3
