
#ifndef HEADER_H
#define HEADER_H

#include "buffer.h"
#include "chunk.h"

#include <stdint.h>

namespace ns3 {

class Header : public Chunk {
public:
  static TypeId GetTypeId();
  ~Header() override;

  using Chunk::Deserialize;
  virtual uint32_t GetSerializedSize() const = 0;
  virtual void Serialize(Buffer::Iterator start) const = 0;
  uint32_t Deserialize(Buffer::Iterator start) override = 0;
  void Print(std::ostream &os) const override = 0;
};

std::ostream &operator<<(std::ostream &os, const Header &header);

} // namespace ns3

#endif
