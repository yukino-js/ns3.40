
#ifndef NS3_BUILD_PROFILE_H
#define NS3_BUILD_PROFILE_H

#define NS_BUILD_PROFILE_NOOP(code)                                            \
  do                                                                           \
    if (false) {                                                               \
      code;                                                                    \
    }                                                                          \
  while (false)

#define NS_BUILD_PROFILE_OP(code)                                              \
  do {                                                                         \
    code;                                                                      \
  } while (false)

#ifdef NS3_BUILD_PROFILE_DEBUG
#define NS_BUILD_DEBUG(code) NS_BUILD_PROFILE_OP(code)
#else
#define NS_BUILD_DEBUG(code) NS_BUILD_PROFILE_NOOP(code)
#endif

#ifdef NS3_BUILD_PROFILE_RELEASE
#define NS_BUILD_RELEASE(code) NS_BUILD_PROFILE_OP(code)
#else
#define NS_BUILD_RELEASE(code) NS_BUILD_PROFILE_NOOP(code)
#endif

#ifdef NS3_BUILD_PROFILE_OPTIMIZED
#define NS_BUILD_OPTIMIZED(code) NS_BUILD_PROFILE_OP(code)
#else
#define NS_BUILD_OPTIMIZED(code) NS_BUILD_PROFILE_NOOP(code)
#endif

#endif
