
#ifndef STATUS_CODE_H
#define STATUS_CODE_H

#include "ns3/buffer.h"

namespace ns3 {

class StatusCode {
public:
  StatusCode();
  void SetSuccess();
  void SetFailure();

  bool IsSuccess() const;

  uint32_t GetSerializedSize() const;
  Buffer::Iterator Serialize(Buffer::Iterator start) const;
  Buffer::Iterator Deserialize(Buffer::Iterator start);

private:
  uint16_t m_code;
};

std::ostream &operator<<(std::ostream &os, const StatusCode &code);

} // namespace ns3

#endif
