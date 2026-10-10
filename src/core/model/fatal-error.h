#ifndef NS3_FATAL_ERROR_H
#define NS3_FATAL_ERROR_H

#include "fatal-impl.h"
#include "log.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>

namespace ns3 {

constexpr std::string_view NS_FATAL_MSG{"NS_FATAL, terminating"};

}

#define NS_FATAL_ERROR_IMPL_NO_MSG(fatal)                                      \
  do {                                                                         \
    NS_LOG_APPEND_TIME_PREFIX_IMPL;                                            \
    NS_LOG_APPEND_NODE_PREFIX_IMPL;                                            \
    std::cerr << "file=" << __FILE__ << ", line=" << __LINE__ << std::endl;    \
    ::ns3::FatalImpl::FlushStreams();                                          \
    if (fatal) {                                                               \
      std::cerr << ns3::NS_FATAL_MSG << std::endl;                             \
      std::terminate();                                                        \
    }                                                                          \
  } while (false)

#define NS_FATAL_ERROR_IMPL(msg, fatal)                                        \
  do {                                                                         \
    std::cerr << "msg=\"" << msg << "\", ";                                    \
    NS_FATAL_ERROR_IMPL_NO_MSG(fatal);                                         \
  } while (false)

#define NS_FATAL_ERROR_NO_MSG() NS_FATAL_ERROR_IMPL_NO_MSG(true)

#define NS_FATAL_ERROR_NO_MSG_CONT() NS_FATAL_ERROR_IMPL_NO_MSG(false)

#define NS_FATAL_ERROR(msg) NS_FATAL_ERROR_IMPL(msg, true)

#define NS_FATAL_ERROR_CONT(msg) NS_FATAL_ERROR_IMPL(msg, false)

#endif
