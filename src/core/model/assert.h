#ifndef NS_ASSERT_H
#define NS_ASSERT_H

#ifdef NS3_ASSERT_ENABLE

#include "fatal-error.h"

#include <iostream>

#define NS_ASSERT(condition)                                                   \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << "NS_ASSERT failed, cond=\"" << #condition << "\", ";        \
      NS_FATAL_ERROR_NO_MSG();                                                 \
    }                                                                          \
  } while (false)

#define NS_ASSERT_MSG(condition, message)                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << "NS_ASSERT failed, cond=\"" << #condition << "\", ";        \
      NS_FATAL_ERROR(message);                                                 \
    }                                                                          \
  } while (false)

#else

#define NS_ASSERT(condition)                                                   \
  do {                                                                         \
    (void)sizeof(condition);                                                   \
  } while (false)

#define NS_ASSERT_MSG(condition, message)                                      \
  do {                                                                         \
    (void)sizeof(condition);                                                   \
  } while (false)

#endif

#endif
