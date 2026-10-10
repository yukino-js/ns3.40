
#ifndef GET_WILDCARD_MATCHES_H
#define GET_WILDCARD_MATCHES_H

#include <string>

namespace ns3 {

std::string GetWildcardMatches(const std::string &configPath,
                               const std::string &matchedPath,
                               const std::string &wildcardSeparator = " ");

}

#endif
