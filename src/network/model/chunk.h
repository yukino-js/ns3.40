
#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"

#include "ns3/object-base.h"

namespace ns3 {

class Chunk : public ObjectBase {
public:
  static TypeId GetTypeId();

  virtual uint32_t Deserialize(Buffer::Iterator start) = 0;

  virtual uint32_t Deserialize(Buffer::Iterator start, Buffer::Iterator end);

  virtual void Print(std::ostream &os) const = 0;
};

} // namespace ns3

#endif
