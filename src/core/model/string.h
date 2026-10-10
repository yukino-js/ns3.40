
#ifndef NS3_STRING_H
#define NS3_STRING_H

#include "attribute-helper.h"

#include <string>
#include <vector>

namespace ns3 {

using StringVector = std::vector<std::string>;

StringVector SplitString(const std::string &str, const std::string &delim);

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(std::string, String);
ATTRIBUTE_ACCESSOR_DEFINE(String);
ATTRIBUTE_CHECKER_DEFINE(String);

} // namespace ns3

#endif
