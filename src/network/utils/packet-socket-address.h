#ifndef PACKET_SOCKET_ADDRESS_H
#define PACKET_SOCKET_ADDRESS_H

#include "mac48-address.h"
#include "mac64-address.h"

#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/ptr.h"

namespace ns3 {

class NetDevice;

class PacketSocketAddress {
public:
  PacketSocketAddress();

  void SetProtocol(uint16_t protocol);

  void SetAllDevices();

  void SetSingleDevice(uint32_t device);

  void SetPhysicalAddress(const Address address);

  uint16_t GetProtocol() const;

  uint32_t GetSingleDevice() const;

  bool IsSingleDevice() const;

  Address GetPhysicalAddress() const;

  operator Address() const;

  static PacketSocketAddress ConvertFrom(const Address &address);

  Address ConvertTo() const;

  static bool IsMatchingType(const Address &address);

private:
  static uint8_t GetType();

  uint16_t m_protocol;
  bool m_isSingleDevice;
  uint32_t m_device;
  Address m_address;
};

} // namespace ns3

#endif
