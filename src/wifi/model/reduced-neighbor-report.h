
#ifndef REDUCED_NEIGHBOR_REPORT_H
#define REDUCED_NEIGHBOR_REPORT_H

#include "wifi-information-element.h"

#include "ns3/mac48-address.h"

#include <vector>

namespace ns3 {

class WifiPhyOperatingChannel;

class ReducedNeighborReport : public WifiInformationElement {
public:
  struct MldParameters {
    uint8_t mldId;
    uint8_t linkId;
    uint8_t bssParamsChangeCount;
  };

  struct TbttInformation {
    uint8_t neighborApTbttOffset{0};
    Mac48Address bssid;
    uint32_t shortSsid{0};
    uint8_t bssParameters{0};
    uint8_t psd20MHz{0};
    MldParameters mldParameters{0, 0, 0};
  };

  struct TbttInformationHeader {
    uint8_t type : 2;
    uint8_t filtered : 1;
    uint8_t reserved : 1;
    uint8_t tbttInfoCount : 4;
    uint8_t tbttInfoLength;
  };

  struct NeighborApInformation {
    mutable TbttInformationHeader tbttInfoHdr{0, 0, 0, 0, 0};
    uint8_t operatingClass{0};
    uint8_t channelNumber{0};
    std::vector<TbttInformation> tbttInformationSet;

    bool hasBssid{false};
    bool hasShortSsid{false};
    bool hasBssParams{false};
    bool has20MHzPsd{false};
    bool hasMldParams{false};
  };

  ReducedNeighborReport();

  WifiInformationElementId ElementId() const override;
  uint16_t GetInformationFieldSize() const override;
  void SerializeInformationField(Buffer::Iterator start) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;

  std::size_t GetNNbrApInfoFields() const;
  void AddNbrApInfoField();

  void SetOperatingChannel(std::size_t nbrApInfoId,
                           const WifiPhyOperatingChannel &channel);
  WifiPhyOperatingChannel GetOperatingChannel(std::size_t nbrApInfoId) const;

  std::size_t GetNTbttInformationFields(std::size_t nbrApInfoId) const;
  void AddTbttInformationField(std::size_t nbrApInfoId);

  void SetBssid(std::size_t nbrApInfoId, std::size_t index, Mac48Address bssid);
  bool HasBssid(std::size_t nbrApInfoId) const;
  Mac48Address GetBssid(std::size_t nbrApInfoId, std::size_t index) const;

  void SetShortSsid(std::size_t nbrApInfoId, std::size_t index,
                    uint32_t shortSsid);
  bool HasShortSsid(std::size_t nbrApInfoId) const;
  uint32_t GetShortSsid(std::size_t nbrApInfoId, std::size_t index) const;

  void SetBssParameters(std::size_t nbrApInfoId, std::size_t index,
                        uint8_t bssParameters);
  bool HasBssParameters(std::size_t nbrApInfoId) const;
  uint8_t GetBssParameters(std::size_t nbrApInfoId, std::size_t index) const;

  void SetPsd20MHz(std::size_t nbrApInfoId, std::size_t index,
                   uint8_t psd20MHz);
  bool HasPsd20MHz(std::size_t nbrApInfoId) const;
  uint8_t GetPsd20MHz(std::size_t nbrApInfoId, std::size_t index) const;

  void SetMldParameters(std::size_t nbrApInfoId, std::size_t index,
                        uint8_t mldId, uint8_t linkId, uint8_t changeSequence);
  bool HasMldParameters(std::size_t nbrApInfoId) const;
  uint8_t GetMldId(std::size_t nbrApInfoId, std::size_t index) const;
  uint8_t GetLinkId(std::size_t nbrApInfoId, std::size_t index) const;

private:
  void WriteTbttInformationCount(std::size_t nbrApInfoId) const;
  uint8_t ReadTbttInformationCount(std::size_t nbrApInfoId) const;

  void WriteTbttInformationLength(std::size_t nbrApInfoId) const;
  void ReadTbttInformationLength(std::size_t nbrApInfoId);

  std::vector<NeighborApInformation> m_nbrApInfoFields;
};

} // namespace ns3

#endif
