
#ifndef LR_WPAN_MAC_PL_HEADERS_H
#define LR_WPAN_MAC_PL_HEADERS_H

#include "lr-wpan-fields.h"

#include <ns3/header.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>

namespace ns3 {

class BeaconPayloadHeader : public Header {
public:
  BeaconPayloadHeader();
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  void SetSuperframeSpecField(SuperframeField sfrmField);
  void SetGtsFields(GtsFields gtsFields);
  void SetPndAddrFields(PendingAddrFields pndAddrFields);
  SuperframeField GetSuperframeSpecField() const;
  GtsFields GetGtsFields() const;
  PendingAddrFields GetPndAddrFields() const;

private:
  SuperframeField m_superframeField;
  GtsFields m_gtsFields;
  PendingAddrFields m_pndAddrFields;
};

class CommandPayloadHeader : public Header {
public:
  enum MacCommand {
    ASSOCIATION_REQ = 0x01,
    ASSOCIATION_RESP = 0x02,
    DISASSOCIATION_NOTIF = 0x03,
    DATA_REQ = 0x04,
    PANID_CONFLICT = 0x05,
    ORPHAN_NOTIF = 0x06,
    BEACON_REQ = 0x07,
    COOR_REALIGN = 0x08,
    GTS_REQ = 0x09,
    CMD_RESERVED = 0xff
  };

  enum AssocStatus {
    SUCCESSFUL = 0x00,
    FULL_CAPACITY = 0x01,
    ACCESS_DENIED = 0x02
  };

  CommandPayloadHeader();
  CommandPayloadHeader(MacCommand macCmd);
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  void SetCommandFrameType(MacCommand macCmd);
  void SetCapabilityField(CapabilityField cap);
  void SetCoordShortAddr(Mac16Address addr);
  void SetChannel(uint8_t channel);
  void SetPage(uint8_t page);
  void SetPanId(uint16_t id);
  void SetShortAddr(Mac16Address shortAddr);
  void SetAssociationStatus(AssocStatus status);
  Mac16Address GetShortAddr() const;
  AssocStatus GetAssociationStatus() const;
  MacCommand GetCommandFrameType() const;
  CapabilityField GetCapabilityField() const;
  Mac16Address GetCoordShortAddr() const;
  uint8_t GetChannel() const;
  uint8_t GetPage() const;
  uint16_t GetPanId() const;

private:
  MacCommand m_cmdFrameId;
  CapabilityField m_capabilityInfo;
  Mac16Address m_shortAddr;
  Mac16Address m_coordShortAddr;
  uint16_t m_panid;
  uint8_t m_logCh;
  uint8_t m_logChPage;
  AssocStatus m_assocStatus;
};

} // namespace ns3

#endif
