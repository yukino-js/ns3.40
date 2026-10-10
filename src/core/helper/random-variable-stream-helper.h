#ifndef RANDOM_VARIABLE_STREAM_HELPER_H
#define RANDOM_VARIABLE_STREAM_HELPER_H

#include <cstdint>
#include <string>

namespace ns3 {

class RandomVariableStreamHelper {
public:
  static int64_t AssignStreams(std::string path, int64_t stream);
};

} // namespace ns3

#endif
