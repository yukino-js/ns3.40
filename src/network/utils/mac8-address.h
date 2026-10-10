
#ifndef MAC8_ADDRESS_H
#define MAC8_ADDRESS_H

#include "ns3/address.h"

#include <iostream>

namespace ns3 {

class Address;

class Mac8Address {
public:
  Mac8Address();
  Mac8Address(uint8_t addr);
  virtual ~Mac8Address();

  static Mac8Address ConvertFrom(const Address &address);

  Address ConvertTo() const;

  static bool IsMatchingType(const Address &address);

  operator Address() const;

  void CopyFrom(const uint8_t *pBuffer);

  void CopyTo(uint8_t *pBuffer) const;

  static Mac8Address GetBroadcast();

  static Mac8Address Allocate();

  static void ResetAllocationIndex();

private:
  static uint8_t m_allocationIndex;
  uint8_t m_address;

  static uint8_t GetType();

  friend bool operator<(const Mac8Address &a, const Mac8Address &b);
  friend bool operator==(const Mac8Address &a, const Mac8Address &b);
  friend bool operator!=(const Mac8Address &a, const Mac8Address &b);
  friend std::ostream &operator<<(std::ostream &os, const Mac8Address &address);
  friend std::istream &operator>>(std::istream &is, Mac8Address &address);
};

bool operator<(const Mac8Address &a, const Mac8Address &b);

bool operator==(const Mac8Address &a, const Mac8Address &b);

bool operator!=(const Mac8Address &a, const Mac8Address &b);

std::ostream &operator<<(std::ostream &os, const Mac8Address &address);

std::istream &operator>>(std::istream &is, Mac8Address &address);

} // namespace ns3

#endif
