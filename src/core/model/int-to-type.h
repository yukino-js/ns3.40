
#ifndef INT_TO_TYPE_H
#define INT_TO_TYPE_H

namespace ns3 {

template <int v> struct IntToType {
  enum v_e { value = v };
};

} // namespace ns3

#endif
