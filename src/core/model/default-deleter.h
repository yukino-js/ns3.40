
#ifndef DEFAULT_DELETER_H
#define DEFAULT_DELETER_H

namespace ns3 {

template <typename T> struct DefaultDeleter {
  inline static void Delete(T *object) { delete object; }
};

} // namespace ns3

#endif
