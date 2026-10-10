#ifndef NS3_ABORT_H
#define NS3_ABORT_H

#include "fatal-error.h"

#define NS_ABORT_MSG(msg)                                                      \
  do {                                                                         \
    std::cerr << "aborted. ";                                                  \
    NS_FATAL_ERROR(msg);                                                       \
  } while (false)

#define NS_ABORT_IF(cond)                                                      \
  do {                                                                         \
    if (cond) {                                                                \
      std::cerr << "aborted. cond=\"" << #cond << ", ";                        \
      NS_FATAL_ERROR_NO_MSG();                                                 \
    }                                                                          \
  } while (false)

#define NS_ABORT_MSG_IF(cond, msg)                                             \
  do {                                                                         \
    if (cond) {                                                                \
      std::cerr << "aborted. cond=\"" << #cond << "\", ";                      \
      NS_FATAL_ERROR(msg);                                                     \
    }                                                                          \
  } while (false)

#define NS_ABORT_UNLESS(cond) NS_ABORT_IF(!(cond))

#define NS_ABORT_MSG_UNLESS(cond, msg) NS_ABORT_MSG_IF(!(cond), msg)

#endif
