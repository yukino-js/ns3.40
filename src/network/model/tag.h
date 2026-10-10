#ifndef TAG_H
#define TAG_H

#include "tag-buffer.h"

#include "ns3/object-base.h"

#include <stdint.h>

namespace ns3 {

class Tag : public ObjectBase {
public:
  static TypeId GetTypeId();

  virtual uint32_t GetSerializedSize() const = 0;
  virtual void Serialize(TagBuffer i) const = 0;
  virtual void Deserialize(TagBuffer i) = 0;

  virtual void Print(std::ostream &os) const = 0;
};

} // namespace ns3

#endif
