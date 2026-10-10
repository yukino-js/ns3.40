#ifndef LR_WPAN_FIELDS_H
#define LR_WPAN_FIELDS_H

#include "ns3/buffer.h"
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>

#include <array>

namespace ns3 {

enum DeviceType { RFD = 0, FFD = 1 };

class SuperframeField {
public:
  SuperframeField();
  void SetSuperframe(uint16_t superFrm);
  void SetBeaconOrder(uint8_t bcnOrder);
  void SetSuperframeOrder(uint8_t frmOrder);
  void SetFinalCapSlot(uint8_t capSlot);
  void SetBattLifeExt(bool battLifeExt);
  void SetPanCoor(bool panCoor);
  void SetAssocPermit(bool assocPermit);
  uint8_t GetBeaconOrder() const;
  uint8_t GetFrameOrder() const;
  uint8_t GetFinalCapSlot() const;
  bool IsBattLifeExt() const;
  bool IsPanCoor() const;
  bool IsAssocPermit() const;
  uint16_t GetSuperframe() const;
  uint32_t GetSerializedSize() const;
  Buffer::Iterator Serialize(Buffer::Iterator i) const;
  Buffer::Iterator Deserialize(Buffer::Iterator i);

private:
  uint8_t m_sspecBcnOrder;
  uint8_t m_sspecSprFrmOrder;
  uint8_t m_sspecFnlCapSlot;
  bool m_sspecBatLifeExt;
  bool m_sspecPanCoor;
  bool m_sspecAssocPermit;
};

std::ostream &operator<<(std::ostream &os,
                         const SuperframeField &superframeField);

class GtsFields {
public:
  GtsFields();
  uint8_t GetGtsSpecField() const;
  uint8_t GetGtsDirectionField() const;
  void SetGtsSpecField(uint8_t gtsSpec);
  void SetGtsDirectionField(uint8_t gtsDir);
  bool GetGtsPermit() const;
  uint32_t GetSerializedSize() const;
  Buffer::Iterator Serialize(Buffer::Iterator i) const;
  Buffer::Iterator Deserialize(Buffer::Iterator i);

private:
  struct GtsDescriptor {
    Mac16Address m_gtsDescDevShortAddr;
    uint8_t m_gtsDescStartSlot;
    uint8_t m_gtsDescLength;
  };

  uint8_t m_gtsSpecDescCount;
  uint8_t m_gtsSpecPermit;
  uint8_t m_gtsDirMask;
  GtsDescriptor m_gtsList[7];
};

std::ostream &operator<<(std::ostream &os, const GtsFields &gtsFields);

class PendingAddrFields {
public:
  PendingAddrFields();
  void AddAddress(Mac16Address shortAddr);
  void AddAddress(Mac64Address extAddr);
  bool SearchAddress(Mac16Address shortAddr);
  bool SearchAddress(Mac64Address extAddr);
  uint8_t GetPndAddrSpecField() const;
  uint8_t GetNumShortAddr() const;
  uint8_t GetNumExtAddr() const;

  void SetPndAddrSpecField(uint8_t pndAddrSpecField);
  uint32_t GetSerializedSize() const;
  Buffer::Iterator Serialize(Buffer::Iterator i) const;
  Buffer::Iterator Deserialize(Buffer::Iterator i);

private:
  uint8_t m_pndAddrSpecNumShortAddr;
  uint8_t m_pndAddrSpecNumExtAddr;
  std::array<Mac16Address, 7> m_shortAddrList;
  std::array<Mac64Address, 7> m_extAddrList;
};

std::ostream &operator<<(std::ostream &os,
                         const PendingAddrFields &pendingAddrFields);

class CapabilityField {
public:
  CapabilityField();

  CapabilityField(uint8_t bitmap);

  uint8_t GetCapability() const;

  void SetCapability(uint8_t bitmap);

  uint32_t GetSerializedSize() const;

  Buffer::Iterator Serialize(Buffer::Iterator i) const;

  Buffer::Iterator Deserialize(Buffer::Iterator i);

  bool IsDeviceTypeFfd() const;

  bool IsPowSrcAvailable() const;

  bool IsReceiverOnWhenIdle() const;

  bool IsSecurityCapability() const;

  bool IsShortAddrAllocOn() const;

  void SetFfdDevice(bool devType);

  void SetPowSrcAvailable(bool pow);

  void SetRxOnWhenIdle(bool rxIdle);

  void SetSecurityCap(bool sec);

  void SetShortAddrAllocOn(bool addrAlloc);

private:
  bool m_reservedBit0;
  bool m_deviceType;
  bool m_powerSource;
  bool m_receiverOnWhenIdle;
  uint8_t m_reservedBit45;
  bool m_securityCap;
  bool m_allocAddr;
};

std::ostream &operator<<(std::ostream &os,
                         const CapabilityField &capabilityField);

} // namespace ns3

#endif
