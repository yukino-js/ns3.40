
#ifndef CTRL_HEADERS_H
#define CTRL_HEADERS_H

#include "block-ack-type.h"
#include "wifi-phy-common.h"

#include "ns3/he-ru.h"
#include "ns3/header.h"
#include "ns3/mac48-address.h"

#include <list>
#include <vector>

namespace ns3 {

class WifiTxVector;
enum AcIndex : uint8_t;

class CtrlBAckRequestHeader : public Header {
public:
  CtrlBAckRequestHeader();
  ~CtrlBAckRequestHeader() override;
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetHtImmediateAck(bool immediateAck);
  void SetType(BlockAckReqType type);
  void SetTidInfo(uint8_t tid);
  void SetStartingSequence(uint16_t seq);

  bool MustSendHtImmediateAck() const;
  BlockAckReqType GetType() const;
  uint8_t GetTidInfo() const;
  uint16_t GetStartingSequence() const;
  bool IsBasic() const;
  bool IsCompressed() const;
  bool IsExtendedCompressed() const;
  bool IsMultiTid() const;

  uint16_t GetStartingSequenceControl() const;

private:
  void SetStartingSequenceControl(uint16_t seqControl);
  uint16_t GetBarControl() const;
  void SetBarControl(uint16_t bar);

  bool m_barAckPolicy;
  BlockAckReqType m_barType;
  uint16_t m_tidInfo;
  uint16_t m_startingSeq;
};

class CtrlBAckResponseHeader : public Header {
public:
  CtrlBAckResponseHeader();
  ~CtrlBAckResponseHeader() override;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetHtImmediateAck(bool immediateAck);
  void SetType(BlockAckType type);
  void SetTidInfo(uint8_t tid, std::size_t index = 0);
  void SetStartingSequence(uint16_t seq, std::size_t index = 0);

  bool MustSendHtImmediateAck() const;
  BlockAckType GetType() const;
  uint8_t GetTidInfo(std::size_t index = 0) const;
  uint16_t GetStartingSequence(std::size_t index = 0) const;
  bool IsBasic() const;
  bool IsCompressed() const;
  bool IsExtendedCompressed() const;
  bool IsMultiTid() const;
  bool IsMultiSta() const;

  void SetAid11(uint16_t aid, std::size_t index);
  uint16_t GetAid11(std::size_t index) const;
  void SetAckType(bool type, std::size_t index);
  bool GetAckType(std::size_t index) const;
  void SetUnassociatedStaAddress(const Mac48Address &ra, std::size_t index);
  Mac48Address GetUnassociatedStaAddress(std::size_t index) const;
  std::size_t GetNPerAidTidInfoSubfields() const;
  std::vector<uint32_t> FindPerAidTidInfoWithAid(uint16_t aid) const;

  void SetReceivedPacket(uint16_t seq, std::size_t index = 0);
  void SetReceivedFragment(uint16_t seq, uint8_t frag);
  bool IsPacketReceived(uint16_t seq, std::size_t index = 0) const;
  bool IsFragmentReceived(uint16_t seq, uint8_t frag) const;

  uint16_t GetStartingSequenceControl(std::size_t index = 0) const;
  void SetStartingSequenceControl(uint16_t seqControl, std::size_t index = 0);
  const std::vector<uint8_t> &GetBitmap(std::size_t index = 0) const;

  void ResetBitmap(std::size_t index = 0);

private:
  uint16_t GetBaControl() const;
  void SetBaControl(uint16_t ba);

  Buffer::Iterator SerializeBitmap(Buffer::Iterator start,
                                   std::size_t index = 0) const;
  Buffer::Iterator DeserializeBitmap(Buffer::Iterator start,
                                     std::size_t index = 0);

  uint16_t IndexInBitmap(uint16_t seq, std::size_t index = 0) const;

  bool IsInBitmap(uint16_t seq, std::size_t index = 0) const;

  bool m_baAckPolicy;
  BlockAckType m_baType;
  uint16_t m_tidInfo;

  struct BaInfoInstance {
    uint16_t m_aidTidInfo;
    uint16_t m_startingSeq;
    std::vector<uint8_t> m_bitmap;
    Mac48Address m_ra;
  };

  std::vector<BaInfoInstance> m_baInfo;
};

enum class TriggerFrameType : uint8_t {
  BASIC_TRIGGER = 0,
  BFRP_TRIGGER = 1,
  MU_BAR_TRIGGER = 2,
  MU_RTS_TRIGGER = 3,
  BSRP_TRIGGER = 4,
  GCR_MU_BAR_TRIGGER = 5,
  BQRP_TRIGGER = 6,
  NFRP_TRIGGER = 7
};

enum class TriggerFrameVariant : uint8_t { HE = 0, EHT };

class CtrlTriggerUserInfoField {
public:
  CtrlTriggerUserInfoField(TriggerFrameType triggerType,
                           TriggerFrameVariant variant);
  CtrlTriggerUserInfoField &operator=(const CtrlTriggerUserInfoField &userInfo);
  ~CtrlTriggerUserInfoField();
  void Print(std::ostream &os) const;
  uint32_t GetSerializedSize() const;
  Buffer::Iterator Serialize(Buffer::Iterator start) const;
  Buffer::Iterator Deserialize(Buffer::Iterator start);
  TriggerFrameType GetType() const;
  WifiPreamble GetPreambleType() const;
  void SetAid12(uint16_t aid);
  uint16_t GetAid12() const;
  bool HasRaRuForAssociatedSta() const;
  bool HasRaRuForUnassociatedSta() const;
  void SetRuAllocation(HeRu::RuSpec ru);
  HeRu::RuSpec GetRuAllocation() const;
  void SetMuRtsRuAllocation(uint8_t value);
  uint8_t GetMuRtsRuAllocation() const;
  void SetUlFecCodingType(bool ldpc);
  bool GetUlFecCodingType() const;
  void SetUlMcs(uint8_t mcs);
  uint8_t GetUlMcs() const;
  void SetUlDcm(bool dcm);
  bool GetUlDcm() const;
  void SetSsAllocation(uint8_t startingSs, uint8_t nSs);
  uint8_t GetStartingSs() const;
  uint8_t GetNss() const;
  void SetRaRuInformation(uint8_t nRaRu, bool moreRaRu);
  uint8_t GetNRaRus() const;
  bool GetMoreRaRu() const;
  void SetUlTargetRssiMaxTxPower();
  void SetUlTargetRssi(int8_t dBm);
  bool IsUlTargetRssiMaxTxPower() const;
  int8_t GetUlTargetRssi() const;
  void SetBasicTriggerDepUserInfo(uint8_t spacingFactor, uint8_t tidLimit,
                                  AcIndex prefAc);
  uint8_t GetMpduMuSpacingFactor() const;
  uint8_t GetTidAggregationLimit() const;
  AcIndex GetPreferredAc() const;
  void SetMuBarTriggerDepUserInfo(const CtrlBAckRequestHeader &bar);
  const CtrlBAckRequestHeader &GetMuBarTriggerDepUserInfo() const;

private:
  TriggerFrameVariant m_variant;

  uint16_t m_aid12;
  uint8_t m_ruAllocation;
  bool m_ulFecCodingType;
  uint8_t m_ulMcs;
  bool m_ulDcm;
  bool m_ps160;

  union {
    struct {
      uint8_t startingSs;
      uint8_t nSs;
    } ssAllocation;

    struct {
      uint8_t nRaRu;
      bool moreRaRu;
    } raRuInformation;
  } m_bits26To31;

  uint8_t m_ulTargetRssi;
  TriggerFrameType m_triggerType;
  uint8_t m_basicTriggerDependentUserInfo;
  CtrlBAckRequestHeader m_muBarTriggerDependentUserInfo;
};

class CtrlTriggerHeader : public Header {
public:
  CtrlTriggerHeader();
  CtrlTriggerHeader(TriggerFrameType type, const WifiTxVector &txVector);
  ~CtrlTriggerHeader() override;
  CtrlTriggerHeader &operator=(const CtrlTriggerHeader &trigger);
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetVariant(TriggerFrameVariant variant);
  TriggerFrameVariant GetVariant() const;
  void SetType(TriggerFrameType type);
  TriggerFrameType GetType() const;
  const char *GetTypeString() const;
  static const char *GetTypeString(TriggerFrameType type);
  bool IsBasic() const;
  bool IsBfrp() const;
  bool IsMuBar() const;
  bool IsMuRts() const;
  bool IsBsrp() const;
  bool IsGcrMuBar() const;
  bool IsBqrp() const;
  bool IsNfrp() const;
  void SetUlLength(uint16_t len);
  uint16_t GetUlLength() const;
  WifiTxVector GetHeTbTxVector(uint16_t staId) const;
  void SetMoreTF(bool more);
  bool GetMoreTF() const;
  void SetCsRequired(bool cs);
  bool GetCsRequired() const;
  void SetUlBandwidth(uint16_t bw);
  uint16_t GetUlBandwidth() const;
  void SetGiAndLtfType(uint16_t guardInterval, uint8_t ltfType);
  uint16_t GetGuardInterval() const;
  uint8_t GetLtfType() const;
  void SetApTxPower(int8_t power);
  int8_t GetApTxPower() const;
  void SetUlSpatialReuse(uint16_t sr);
  uint16_t GetUlSpatialReuse() const;
  void SetPaddingSize(std::size_t size);
  std::size_t GetPaddingSize() const;
  CtrlTriggerHeader GetCommonInfoField() const;

  CtrlTriggerUserInfoField &AddUserInfoField();
  CtrlTriggerUserInfoField &
  AddUserInfoField(const CtrlTriggerUserInfoField &userInfo);

  typedef std::list<CtrlTriggerUserInfoField>::const_iterator ConstIterator;

  typedef std::list<CtrlTriggerUserInfoField>::iterator Iterator;

  ConstIterator begin() const;
  ConstIterator end() const;
  Iterator begin();
  Iterator end();
  std::size_t GetNUserInfoFields() const;
  ConstIterator FindUserInfoWithAid(ConstIterator start, uint16_t aid12) const;
  ConstIterator FindUserInfoWithAid(uint16_t aid12) const;
  ConstIterator FindUserInfoWithRaRuAssociated(ConstIterator start) const;
  ConstIterator FindUserInfoWithRaRuAssociated() const;
  ConstIterator FindUserInfoWithRaRuUnassociated(ConstIterator start) const;
  ConstIterator FindUserInfoWithRaRuUnassociated() const;
  bool IsValid() const;

private:
  TriggerFrameVariant m_variant;
  TriggerFrameType m_triggerType;
  uint16_t m_ulLength;
  bool m_moreTF;
  bool m_csRequired;
  uint8_t m_ulBandwidth;
  uint8_t m_giAndLtfType;
  uint8_t m_apTxPower;
  uint16_t m_ulSpatialReuse;
  std::size_t m_padding;

  std::list<CtrlTriggerUserInfoField> m_userInfoFields;
};

} // namespace ns3

#endif
