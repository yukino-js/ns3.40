
#ifndef NS3_WARNINGS_H
#define NS3_WARNINGS_H

#if defined(_MSC_VER)
#define NS_WARNING_PUSH __pragma(warning(push))
#define NS_WARNING_SILENCE_DEPRECATED __pragma(warning(disable : 4996))
#define NS_WARNING_POP __pragma(warning(pop))

#elif defined(__GNUC__) || defined(__clang__)
#define NS_WARNING_PUSH _Pragma("GCC diagnostic push")
#define NS_WARNING_SILENCE_DEPRECATED                                          \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define NS_WARNING_POP _Pragma("GCC diagnostic pop")

#else
#define NS_WARNING_PUSH
#define NS_WARNING_SILENCE_DEPRECATED
#define NS_WARNING_POP

#endif

#define NS_WARNING_PUSH_DEPRECATED                                             \
  NS_WARNING_PUSH;                                                             \
  NS_WARNING_SILENCE_DEPRECATED

#endif
