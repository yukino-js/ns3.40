

#ifndef LR_WPAN_MAC_HEADER_H
#define LR_WPAN_MAC_HEADER_H

#include <ns3/header.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>

namespace ns3 {

class LrWpanMacHeader : public Header {
public:
  enum LrWpanMacType {
    LRWPAN_MAC_BEACON = 0,
    LRWPAN_MAC_DATA = 1,
    LRWPAN_MAC_ACKNOWLEDGMENT = 2,
    LRWPAN_MAC_COMMAND = 3,
    LRWPAN_MAC_RESERVED
  };

  enum AddrModeType { NOADDR = 0, RESADDR = 1, SHORTADDR = 2, EXTADDR = 3 };

  enum KeyIdModeType {
    IMPLICIT = 0,
    NOKEYSOURCE = 1,
    SHORTKEYSOURCE = 2,
    LONGKEYSOURCE = 3
  };

  LrWpanMacHeader();

  LrWpanMacHeader(LrWpanMacType wpanMacType, uint8_t seqNum);

  ~LrWpanMacHeader() override;

  LrWpanMacType GetType() const;
  uint16_t GetFrameControl() const;
  bool IsSecEnable() const;
  bool IsFrmPend() const;
  bool IsAckReq() const;
  bool IsPanIdComp() const;
  uint8_t GetFrmCtrlRes() const;
  uint8_t GetDstAddrMode() const;
  uint8_t GetFrameVer() const;
  uint8_t GetSrcAddrMode() const;
  uint8_t GetSeqNum() const;
  uint16_t GetDstPanId() const;
  Mac16Address GetShortDstAddr() const;
  Mac64Address GetExtDstAddr() const;
  uint16_t GetSrcPanId() const;
  Mac16Address GetShortSrcAddr() const;
  Mac64Address GetExtSrcAddr() const;
  uint8_t GetSecControl() const;
  uint32_t GetFrmCounter() const;

  uint8_t GetSecLevel() const;
  uint8_t GetKeyIdMode() const;
  uint8_t GetSecCtrlReserved() const;
  uint32_t GetKeyIdSrc32() const;
  uint64_t GetKeyIdSrc64() const;
  uint8_t GetKeyIdIndex() const;
  bool IsBeacon() const;
  bool IsData() const;
  bool IsAcknowledgment() const;
  bool IsCommand() const;
  void SetType(LrWpanMacType wpanMacType);
  void SetFrameControl(uint16_t frameControl);
  void SetSecEnable();
  void SetSecDisable();
  void SetFrmPend();
  void SetNoFrmPend();
  void SetAckReq();
  void SetNoAckReq();
  void SetPanIdComp();
  void SetNoPanIdComp();
  void SetFrmCtrlRes(uint8_t res);
  void SetDstAddrMode(uint8_t addrMode);
  void SetFrameVer(uint8_t ver);
  void SetSrcAddrMode(uint8_t addrMode);
  void SetSeqNum(uint8_t seqNum);
  void SetSrcAddrFields(uint16_t panId, Mac16Address addr);
  void SetSrcAddrFields(uint16_t panId, Mac64Address addr);
  void SetDstAddrFields(uint16_t panId, Mac16Address addr);
  void SetDstAddrFields(uint16_t panId, Mac64Address addr);

  void SetSecControl(uint8_t secLevel);
  void SetFrmCounter(uint32_t frmCntr);
  void SetSecLevel(uint8_t secLevel);
  void SetKeyIdMode(uint8_t keyIdMode);
  void SetSecCtrlReserved(uint8_t res);
  void SetKeyId(uint8_t keyIndex);
  void SetKeyId(uint32_t keySrc, uint8_t keyIndex);
  void SetKeyId(uint64_t keySrc, uint8_t keyIndex);
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_fctrlFrmType;
  uint8_t m_fctrlSecU;
  uint8_t m_fctrlFrmPending;
  uint8_t m_fctrlAckReq;
  uint8_t m_fctrlPanIdComp;
  uint8_t m_fctrlReserved;
  uint8_t m_fctrlDstAddrMode;
  uint8_t m_fctrlFrmVer;
  uint8_t m_fctrlSrcAddrMode;

  uint8_t m_SeqNum;

  uint16_t m_addrDstPanId;
  Mac16Address m_addrShortDstAddr;
  Mac64Address m_addrExtDstAddr;
  uint16_t m_addrSrcPanId;
  Mac16Address m_addrShortSrcAddr;
  Mac64Address m_addrExtSrcAddr;

  uint32_t m_auxFrmCntr;

  uint8_t m_secctrlSecLevel;
  uint8_t m_secctrlKeyIdMode;

  uint8_t m_secctrlReserved;

  union {
    uint32_t m_auxKeyIdKeySrc32;
    uint64_t m_auxKeyIdKeySrc64;
  };

  uint8_t m_auxKeyIdKeyIndex;
};

}; // namespace ns3

#endif
