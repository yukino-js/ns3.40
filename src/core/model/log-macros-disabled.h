

#ifndef NS3_LOG_MACROS_DISABLED_H
#define NS3_LOG_MACROS_DISABLED_H

#ifndef NS3_LOG_ENABLE

#define NS_LOG_NOOP_INTERNAL(msg)                                              \
  do                                                                           \
    if (false) {                                                               \
      std::clog << msg;                                                        \
    }                                                                          \
  while (false)

#define NS_LOG(level, msg) NS_LOG_NOOP_INTERNAL(msg)

#define NS_LOG_FUNCTION_NOARGS()

#define NS_LOG_NOOP_FUNC_INTERNAL(msg)                                         \
  do                                                                           \
    if (false) {                                                               \
      ns3::ParameterLogger(std::clog) << msg;                                  \
    }                                                                          \
  while (false)

#define NS_LOG_FUNCTION(parameters) NS_LOG_NOOP_FUNC_INTERNAL(parameters)

#define NS_LOG_UNCOND(msg) NS_LOG_NOOP_INTERNAL(msg)

#endif

#endif
