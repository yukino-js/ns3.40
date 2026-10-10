#ifndef BREAKPOINT_H
#define BREAKPOINT_H

namespace ns3 {

#if (defined(__i386__) || defined(__amd64__) || defined(__x86_64__)) &&        \
    defined(__GNUC__) && __GNUC__ >= 2
#define NS_BREAKPOINT()                                                        \
  do {                                                                         \
    __asm__ __volatile__("int $03");                                           \
  } while (false)
#elif defined(_MSC_VER) && defined(_M_IX86)
#define NS_BREAKPOINT()                                                        \
  do {                                                                         \
    __asm int 3h                                                               \
  } while (false)
#elif defined(__alpha__) && !defined(__osf__) && defined(__GNUC__) &&          \
    __GNUC__ >= 2
#define NS_BREAKPOINT()                                                        \
  do {                                                                         \
    __asm__ __volatile__("bpt");                                               \
  } while (false)
#else
#define NS_BREAKPOINT() ns3::BreakpointFallback()
#endif

void BreakpointFallback();

} // namespace ns3

#endif
