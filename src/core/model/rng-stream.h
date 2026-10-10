
#ifndef RNGSTREAM_H
#define RNGSTREAM_H
#include <stdint.h>
#include <string>

namespace ns3 {

class RngStream {
public:
  RngStream(uint32_t seed, uint64_t stream, uint64_t substream);
  RngStream(const RngStream &r);
  double RandU01();

private:
  void AdvanceNthBy(uint64_t nth, int by, double state[6]);

  double m_currentState[6];
};

} // namespace ns3

#endif
