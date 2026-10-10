
#ifndef UNUSED_H
#define UNUSED_H

#include "deprecated.h"

#ifndef NS_UNUSED
#define NS_UNUSED(x)                                                           \
  _Pragma("GCC warning \"NS_UNUSED is deprecated, use [[maybe_unused]] "       \
          "directly\"")((void)(x))
#endif

#ifndef NS_UNUSED_GLOBAL
#if defined(__GNUC__)
#define NS_UNUSED_GLOBAL(x)                                                    \
  NS_DEPRECATED_3_36(                                                          \
      "NS_UNUSED_GLOBAL is deprecated, use [[maybe_unused]] directly")         \
  [[maybe_unused]] x
#elif defined(__LCLINT__)
#define NS_UNUSED_GLOBAL(x)                                                    \
  NS_DEPRECATED_3_36(                                                          \
      "NS_UNUSED_GLOBAL is deprecated, use [[maybe_unused]] directly")         \
  x
#else
#define NS_UNUSED_GLOBAL(x)                                                    \
  NS_DEPRECATED_3_36(                                                          \
      "NS_UNUSED_GLOBAL is deprecated, use [[maybe_unused]] directly")         \
  x
#endif
#endif

#endif
