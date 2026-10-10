
#ifndef ADDRESS_H
#define ADDRESS_H

#include "tag-buffer.h"

#include "ns3/attribute-helper.h"
#include "ns3/attribute.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class Address {
public:
  static constexpr uint32_t MAX_SIZE{20};

  Address();
  Address(uint8_t type, const uint8_t *buffer, uint8_t len);
  Address(const Address &address);
  Address &operator=(const Address &address);

  bool IsInvalid() const;
  uint8_t GetLength() const;
  uint32_t CopyTo(uint8_t buffer[MAX_SIZE]) const;
  uint32_t CopyAllTo(uint8_t *buffer, uint8_t len) const;
  uint32_t CopyFrom(const uint8_t *buffer, uint8_t len);
  uint32_t CopyAllFrom(const uint8_t *buffer, uint8_t len);
  bool CheckCompatible(uint8_t type, uint8_t len) const;
  bool IsMatchingType(uint8_t type) const;
  static uint8_t Register();
  uint32_t GetSerializedSize() const;
  void Serialize(TagBuffer buffer) const;
  void Deserialize(TagBuffer buffer);

private:
  friend bool operator==(const Address &a, const Address &b);

  friend bool operator!=(const Address &a, const Address &b);

  friend bool operator<(const Address &a, const Address &b);

  friend std::ostream &operator<<(std::ostream &os, const Address &address);

  friend std::istream &operator>>(std::istream &is, Address &address);

  uint8_t m_type;
  uint8_t m_len;
  uint8_t m_data[MAX_SIZE];
};

ATTRIBUTE_HELPER_HEADER(Address);

bool operator==(const Address &a, const Address &b);
bool operator!=(const Address &a, const Address &b);
bool operator<(const Address &a, const Address &b);
std::ostream &operator<<(std::ostream &os, const Address &address);
std::istream &operator>>(std::istream &is, Address &address);

} // namespace ns3

#endif
