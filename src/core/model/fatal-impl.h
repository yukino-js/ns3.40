
#ifndef FATAL_IMPL_H
#define FATAL_IMPL_H

#include <ostream>

namespace ns3 {

namespace FatalImpl {

void RegisterStream(std::ostream *stream);

void UnregisterStream(std::ostream *stream);

void FlushStreams();

} // namespace FatalImpl
} // namespace ns3

#endif
