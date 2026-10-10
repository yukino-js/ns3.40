
#ifndef TRAILER_H
#define TRAILER_H

#include "buffer.h"
#include "chunk.h"

#include <stdint.h>

namespace ns3 {

class Trailer : public Chunk {
public:
  static TypeId GetTypeId();
  ~Trailer() override;
  virtual uint32_t GetSerializedSize() const = 0;
  virtual void Serialize(Buffer::Iterator start) const = 0;
  uint32_t Deserialize(Buffer::Iterator end) override = 0;
  uint32_t Deserialize(Buffer::Iterator start, Buffer::Iterator end) override;
  void Print(std::ostream &os) const override = 0;
};

std::ostream &operator<<(std::ostream &os, const Trailer &trailer);

} // namespace ns3

#endif
