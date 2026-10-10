
#ifndef AP_WIFI_MAC_H
#define AP_WIFI_MAC_H

#include "wifi-mac-header.h"
#include "wifi-mac.h"

#include <unordered_map>
#include <variant>

namespace ns3 {

struct AllSupportedRates;
class CapabilityInformation;
class DsssParameterSet;
class ErpInformation;
class EdcaParameterSet;
class MuEdcaParameterSet;
class ReducedNeighborReport;
class MultiLinkElement;
class HtOperation;
class VhtOperation;
class HeOperation;
class EhtOperation;
class CfParameterSet;
class UniformRandomVariable;
class MgtAssocRequestHeader;
class MgtReassocRequestHeader;
class MgtAssocResponseHeader;
class MgtEmlOmn;

using AssocReqRefVariant =
    std::variant<std::reference_wrapper<MgtAssocRequestHeader>,
                 std::reference_wrapper<MgtReassocRequestHeader>>;

class ApWifiMac : public WifiMac {
public:
  static TypeId GetTypeId();

  ApWifiMac();
  ~ApWifiMac() override;

  void SetLinkUpCallback(Callback<void> linkUp) override;
  bool CanForwardPacketsTo(Mac48Address to) const override;
  void Enqueue(Ptr<Packet> packet, Mac48Address to) override;
  void Enqueue(Ptr<Packet> packet, Mac48Address to, Mac48Address from) override;
  bool SupportsSendFrom() const override;
  Ptr<WifiMacQueue> GetTxopQueue(AcIndex ac) const override;
  void ConfigureStandard(WifiStandard standard) override;

  void SetBeaconInterval(Time interval);
  Time GetBeaconInterval() const;

  int64_t AssignStreams(int64_t stream);

  const std::map<uint16_t, Mac48Address> &GetStaList(uint8_t linkId) const;
  uint16_t GetAssociationId(Mac48Address addr, uint8_t linkId) const;

  std::optional<uint8_t> IsAssociated(const Mac48Address &address) const;

  std::optional<Mac48Address> GetMldOrLinkAddressByAid(uint16_t aid) const;

  uint8_t GetBufferStatus(uint8_t tid, Mac48Address address) const;
  void SetBufferStatus(uint8_t tid, Mac48Address address, uint8_t size);
  uint8_t GetMaxBufferStatus(Mac48Address address) const;

protected:
  struct ApLinkEntity : public WifiMac::LinkEntity {
    ~ApLinkEntity() override;

    EventId beaconEvent;
    std::map<uint16_t, Mac48Address> staList;
    uint16_t numNonHtStations{0};
    uint16_t numNonErpStations{0};
    bool shortSlotTimeEnabled{false};
    bool shortPreambleEnabled{false};
  };

  ApLinkEntity &GetLink(uint8_t linkId) const;

  std::map<uint16_t, Mac48Address> m_aidToMldOrLinkAddress;

private:
  std::unique_ptr<LinkEntity> CreateLinkEntity() const override;
  Mac48Address DoGetLocalAddress(const Mac48Address &remoteAddr) const override;
  void Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) override;

  bool ReceiveAssocRequest(const AssocReqRefVariant &assoc,
                           const Mac48Address &from, uint8_t linkId);

  void ParseReportedStaInfo(const AssocReqRefVariant &assoc, Mac48Address from,
                            uint8_t linkId);

  void ReceiveEmlOmn(MgtEmlOmn &frame, const Mac48Address &sender,
                     uint8_t linkId);

  void TxOk(Ptr<const WifiMpdu> mpdu);
  void TxFailed(WifiMacDropReason timeoutReason, Ptr<const WifiMpdu> mpdu);

  void DeaggregateAmsduAndForward(Ptr<const WifiMpdu> mpdu) override;
  void ForwardDown(Ptr<Packet> packet, Mac48Address from, Mac48Address to);
  void ForwardDown(Ptr<Packet> packet, Mac48Address from, Mac48Address to,
                   uint8_t tid);
  void SendProbeResp(Mac48Address to, uint8_t linkId);
  MgtAssocResponseHeader GetAssocResp(Mac48Address to, uint8_t linkId);
  using LinkIdStaAddrMap = std::map<uint8_t, Mac48Address>;
  void SetAid(MgtAssocResponseHeader &assoc,
              const LinkIdStaAddrMap &linkIdStaAddrMap);
  LinkIdStaAddrMap GetLinkIdStaAddrMap(MgtAssocResponseHeader &assoc,
                                       const Mac48Address &to, uint8_t linkId);
  void SendAssocResp(Mac48Address to, bool isReassoc, uint8_t linkId);
  void SendOneBeacon(uint8_t linkId);

  void ProcessPowerManagementFlag(Ptr<const WifiMpdu> mpdu, uint8_t linkId);
  void StaSwitchingToPsMode(const Mac48Address &staAddr, uint8_t linkId);
  void StaSwitchingToActiveModeOrDeassociated(const Mac48Address &staAddr,
                                              uint8_t linkId);

  CapabilityInformation GetCapabilities(uint8_t linkId) const;
  ErpInformation GetErpInformation(uint8_t linkId) const;
  EdcaParameterSet GetEdcaParameterSet(uint8_t linkId) const;
  std::optional<MuEdcaParameterSet> GetMuEdcaParameterSet() const;
  std::optional<ReducedNeighborReport>
  GetReducedNeighborReport(uint8_t linkId) const;
  MultiLinkElement
  GetMultiLinkElement(uint8_t linkId, WifiMacType frameType,
                      const Mac48Address &to = Mac48Address::GetBroadcast());
  HtOperation GetHtOperation(uint8_t linkId) const;
  VhtOperation GetVhtOperation(uint8_t linkId) const;
  HeOperation GetHeOperation(uint8_t linkId) const;
  EhtOperation GetEhtOperation(uint8_t linkId) const;
  AllSupportedRates GetSupportedRates(uint8_t linkId) const;
  DsssParameterSet GetDsssParameterSet(uint8_t linkId) const;
  void SetBeaconGeneration(bool enable);

  void UpdateShortSlotTimeEnabled(uint8_t linkId);
  void UpdateShortPreambleEnabled(uint8_t linkId);

  bool GetUseNonErpProtection(uint8_t linkId) const;

  void DoDispose() override;
  void DoInitialize() override;

  uint16_t GetNextAssociationId(std::list<uint8_t> linkIds);

  Ptr<Txop> m_beaconTxop;
  bool m_enableBeaconGeneration;
  Time m_beaconInterval;
  Ptr<UniformRandomVariable> m_beaconJitter;
  bool m_enableBeaconJitter;
  bool m_enableNonErpProtection;
  Time m_bsrLifetime;
  std::map<Mac48Address, EventId> m_transitionTimeoutEvents;

  struct BsrType {
    uint8_t value;
    Time timestamp;
  };

  std::unordered_map<WifiAddressTidPair, BsrType, WifiAddressTidHash>
      m_bufferStatus;

  typedef void (*AssociationCallback)(uint16_t aid, Mac48Address address);

  TracedCallback<uint16_t, Mac48Address> m_assocLogger;
  TracedCallback<uint16_t, Mac48Address> m_deAssocLogger;
};

} // namespace ns3

#endif
