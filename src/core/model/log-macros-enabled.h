
#ifndef NS3_LOG_MACROS_ENABLED_H
#define NS3_LOG_MACROS_ENABLED_H

#define NS_LOG_APPEND_TIME_PREFIX_IMPL                                         \
  do {                                                                         \
    ns3::TimePrinter printer = ns3::LogGetTimePrinter();                       \
    if (printer != 0) {                                                        \
      (*printer)(std::clog);                                                   \
      std::clog << " ";                                                        \
    }                                                                          \
  } while (false)

#define NS_LOG_APPEND_NODE_PREFIX_IMPL                                         \
  do {                                                                         \
    ns3::NodePrinter printer = ns3::LogGetNodePrinter();                       \
    if (printer != 0) {                                                        \
      (*printer)(std::clog);                                                   \
      std::clog << " ";                                                        \
    }                                                                          \
  } while (false)

#ifdef NS3_LOG_ENABLE

#define NS_LOG_APPEND_TIME_PREFIX                                              \
  if (g_log.IsEnabled(ns3::LOG_PREFIX_TIME)) {                                 \
    NS_LOG_APPEND_TIME_PREFIX_IMPL;                                            \
  }

#define NS_LOG_APPEND_NODE_PREFIX                                              \
  if (g_log.IsEnabled(ns3::LOG_PREFIX_NODE)) {                                 \
    NS_LOG_APPEND_NODE_PREFIX_IMPL;                                            \
  }

#define NS_LOG_APPEND_FUNC_PREFIX                                              \
  if (g_log.IsEnabled(ns3::LOG_PREFIX_FUNC)) {                                 \
    std::clog << g_log.Name() << ":" << __FUNCTION__ << "(): ";                \
  }

#define NS_LOG_APPEND_LEVEL_PREFIX(level)                                      \
  if (g_log.IsEnabled(ns3::LOG_PREFIX_LEVEL)) {                                \
    std::clog << "[" << g_log.GetLevelLabel(level) << "] ";                    \
  }

#ifndef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT
#endif

#ifndef NS_LOG_CONDITION
#define NS_LOG_CONDITION
#endif

#define NS_LOG(level, msg)                                                     \
  NS_LOG_CONDITION                                                             \
  do {                                                                         \
    if (g_log.IsEnabled(level)) {                                              \
      NS_LOG_APPEND_TIME_PREFIX;                                               \
      NS_LOG_APPEND_NODE_PREFIX;                                               \
      NS_LOG_APPEND_CONTEXT;                                                   \
      NS_LOG_APPEND_FUNC_PREFIX;                                               \
      NS_LOG_APPEND_LEVEL_PREFIX(level);                                       \
      std::clog << msg << std::endl;                                           \
    }                                                                          \
  } while (false)

#define NS_LOG_FUNCTION_NOARGS()                                               \
  NS_LOG_CONDITION                                                             \
  do {                                                                         \
    if (g_log.IsEnabled(ns3::LOG_FUNCTION)) {                                  \
      NS_LOG_APPEND_TIME_PREFIX;                                               \
      NS_LOG_APPEND_NODE_PREFIX;                                               \
      NS_LOG_APPEND_CONTEXT;                                                   \
      std::clog << g_log.Name() << ":" << __FUNCTION__ << "()" << std::endl;   \
    }                                                                          \
  } while (false)

#define NS_LOG_FUNCTION(parameters)                                            \
  NS_LOG_CONDITION                                                             \
  do {                                                                         \
    if (g_log.IsEnabled(ns3::LOG_FUNCTION)) {                                  \
      NS_LOG_APPEND_TIME_PREFIX;                                               \
      NS_LOG_APPEND_NODE_PREFIX;                                               \
      NS_LOG_APPEND_CONTEXT;                                                   \
      std::clog << g_log.Name() << ":" << __FUNCTION__ << "(";                 \
      ns3::ParameterLogger(std::clog) << parameters;                           \
      std::clog << ")" << std::endl;                                           \
    }                                                                          \
  } while (false)

#define NS_LOG_UNCOND(msg)                                                     \
  NS_LOG_CONDITION                                                             \
  do {                                                                         \
    std::clog << msg << std::endl;                                             \
  } while (false)

#endif

#endif
