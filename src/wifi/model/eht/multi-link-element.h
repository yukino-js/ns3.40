
#ifndef MULTI_LINK_ELEMENT_H
#define MULTI_LINK_ELEMENT_H

#include "ns3/nstime.h"
#include "ns3/wifi-information-element.h"
#include "ns3/wifi-mac-header.h"

#include <memory>
#include <optional>
#include <variant>

namespace ns3 {

class MgtAssocRequestHeader;
class MgtReassocRequestHeader;
class MgtAssocResponseHeader;

using AssocReqRefVariant =
    std::variant<std::reference_wrapper<MgtAssocRequestHeader>,
                 std::reference_wrapper<MgtReassocRequestHeader>>;

struct CommonInfoBasicMle {
  struct MediumSyncDelayInfo {
    uint8_t mediumSyncDuration;
    uint8_t mediumSyncOfdmEdThreshold : 4;
    uint8_t mediumSyncMaxNTxops : 4;
  };

  struct EmlCapabilities {
    uint8_t emlsrSupport : 1;
    uint8_t emlsrPaddingDelay : 3;
    uint8_t emlsrTransitionDelay : 3;
    uint8_t emlmrSupport : 1;
    uint8_t emlmrDelay : 3;
    uint8_t transitionTimeout : 4;
  };

  struct MldCapabilities {
    uint8_t maxNSimultaneousLinks : 4;
    uint8_t srsSupport : 1;
    uint8_t tidToLinkMappingSupport : 2;
    uint8_t freqSepForStrApMld : 5;
    uint8_t aarSupport : 1;
  };

  Mac48Address m_mldMacAddress;
  std::optional<uint8_t> m_linkIdInfo;
  std::optional<uint8_t> m_bssParamsChangeCount;
  std::optional<MediumSyncDelayInfo> m_mediumSyncDelayInfo;
  std::optional<EmlCapabilities> m_emlCapabilities;
  std::optional<MldCapabilities> m_mldCapabilities;

  uint16_t GetPresenceBitmap() const;
  uint8_t GetSize() const;
  void Serialize(Buffer::Iterator &start) const;
  uint8_t Deserialize(Buffer::Iterator start, uint16_t presence);

  static uint8_t EncodeEmlsrPaddingDelay(Time delay);
  static Time DecodeEmlsrPaddingDelay(uint8_t value);

  static uint8_t EncodeEmlsrTransitionDelay(Time delay);
  static Time DecodeEmlsrTransitionDelay(uint8_t value);
};

class MultiLinkElement : public WifiInformationElement {
public:
  enum Variant : uint8_t { BASIC_VARIANT = 0, UNSET };

  enum SubElementId : uint8_t { PER_STA_PROFILE_SUBELEMENT_ID = 0 };

  using ContainingFrame =
      std::variant<std::monostate,
                   std::reference_wrapper<const MgtAssocRequestHeader>,
                   std::reference_wrapper<const MgtReassocRequestHeader>,
                   std::reference_wrapper<const MgtAssocResponseHeader>>;

  MultiLinkElement(ContainingFrame frame = {});
  MultiLinkElement(Variant variant, ContainingFrame frame = {});

  WifiInformationElementId ElementId() const override;
  WifiInformationElementId ElementIdExt() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  Variant GetVariant() const;

  CommonInfoBasicMle &GetCommonInfoBasic();

  const CommonInfoBasicMle &GetCommonInfoBasic() const;

  void SetMldMacAddress(Mac48Address address);

  Mac48Address GetMldMacAddress() const;

  void SetLinkIdInfo(uint8_t linkIdInfo);
  bool HasLinkIdInfo() const;
  uint8_t GetLinkIdInfo() const;

  void SetBssParamsChangeCount(uint8_t count);
  bool HasBssParamsChangeCount() const;
  uint8_t GetBssParamsChangeCount() const;

  void SetMediumSyncDelayTimer(Time delay);
  void SetMediumSyncOfdmEdThreshold(int8_t threshold);
  void SetMediumSyncMaxNTxops(uint8_t nTxops);
  bool HasMediumSyncDelayInfo() const;
  Time GetMediumSyncDelayTimer() const;
  int8_t GetMediumSyncOfdmEdThreshold() const;
  uint8_t GetMediumSyncMaxNTxops() const;

  void SetEmlsrSupported(bool supported);
  void SetEmlsrPaddingDelay(Time delay);
  void SetEmlsrTransitionDelay(Time delay);
  void SetTransitionTimeout(Time timeout);
  bool HasEmlCapabilities() const;
  bool IsEmlsrSupported() const;
  Time GetEmlsrPaddingDelay() const;
  Time GetEmlsrTransitionDelay() const;
  Time GetTransitionTimeout() const;

  mutable ContainingFrame m_containingFrame;

  class PerStaProfileSubelement : public WifiInformationElement {
  public:
    PerStaProfileSubelement(Variant variant);

    PerStaProfileSubelement(const PerStaProfileSubelement &perStaProfile);
    PerStaProfileSubelement &
    operator=(const PerStaProfileSubelement &perStaProfile);
    PerStaProfileSubelement &
    operator=(PerStaProfileSubelement &&perStaProfile) = default;

    WifiInformationElementId ElementId() const override;

    void SetLinkId(uint8_t linkId);
    uint8_t GetLinkId() const;

    void SetCompleteProfile();
    bool IsCompleteProfileSet() const;

    void SetStaMacAddress(Mac48Address address);
    bool HasStaMacAddress() const;
    Mac48Address GetStaMacAddress() const;

    void SetAssocRequest(const std::variant<MgtAssocRequestHeader,
                                            MgtReassocRequestHeader> &assoc);
    void SetAssocRequest(
        std::variant<MgtAssocRequestHeader, MgtReassocRequestHeader> &&assoc);
    bool HasAssocRequest() const;
    bool HasReassocRequest() const;
    AssocReqRefVariant GetAssocRequest() const;

    void SetAssocResponse(const MgtAssocResponseHeader &assoc);
    void SetAssocResponse(MgtAssocResponseHeader &&assoc);
    bool HasAssocResponse() const;
    MgtAssocResponseHeader &GetAssocResponse() const;

    uint8_t GetStaInfoLength() const;

    mutable ContainingFrame m_containingFrame;

  private:
    uint16_t GetInformationFieldSize() const override;
    void SerializeInformationField(Buffer::Iterator start) const override;
    uint16_t DeserializeInformationField(Buffer::Iterator start,
                                         uint16_t length) override;

    Variant m_variant;
    uint16_t m_staControl;
    Mac48Address m_staMacAddress;
    std::variant<std::monostate, std::unique_ptr<MgtAssocRequestHeader>,
                 std::unique_ptr<MgtReassocRequestHeader>,
                 std::unique_ptr<MgtAssocResponseHeader>>
        m_staProfile;
  };

  void AddPerStaProfileSubelement();
  std::size_t GetNPerStaProfileSubelements() const;
  PerStaProfileSubelement &GetPerStaProfile(std::size_t i);
  const PerStaProfileSubelement &GetPerStaProfile(std::size_t i) const;

private:
  void SetVariant(Variant variant);

  using CommonInfo = std::variant<CommonInfoBasicMle, std::monostate>;

  CommonInfo m_commonInfo;

  std::vector<PerStaProfileSubelement> m_perStaProfileSubelements;
};

} // namespace ns3

#endif
