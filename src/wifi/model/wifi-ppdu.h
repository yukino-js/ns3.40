
#ifndef WIFI_PPDU_H
#define WIFI_PPDU_H

#include "wifi-tx-vector.h"

#include "ns3/nstime.h"

#include <list>
#include <optional>
#include <unordered_map>

namespace ns3 {

class Packet;
class WifiPsdu;
class WifiPhyOperatingChannel;

typedef std::unordered_map<uint16_t, Ptr<const WifiPsdu>> WifiConstPsduMap;

class WifiPpdu : public SimpleRefCount<WifiPpdu> {
public:
  WifiPpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
           const WifiPhyOperatingChannel &channel, uint64_t uid = UINT64_MAX);
  WifiPpdu(const WifiConstPsduMap &psdus, const WifiTxVector &txVector,
           const WifiPhyOperatingChannel &channel, uint64_t uid);
  virtual ~WifiPpdu();

  const WifiTxVector &GetTxVector() const;

  void ResetTxVector() const;

  void UpdateTxVector(const WifiTxVector &updatedTxVector) const;

  Ptr<const WifiPsdu> GetPsdu() const;

  bool IsTruncatedTx() const;

  void SetTruncatedTx();

  virtual Time GetTxDuration() const;

  virtual uint16_t GetTxChannelWidth() const;

  uint16_t GetTxCenterFreq() const;

  bool DoesOverlapChannel(uint16_t minFreq, uint16_t maxFreq) const;

  WifiModulationClass GetModulation() const;

  uint64_t GetUid() const;

  WifiPreamble GetPreamble() const;

  void Print(std::ostream &os) const;
  virtual Ptr<WifiPpdu> Copy() const;

  virtual WifiPpduType GetType() const;

  virtual uint16_t GetStaId() const;

protected:
  virtual std::string PrintPayload() const;

  WifiPreamble m_preamble;
  WifiModulationClass m_modulation;
  WifiConstPsduMap m_psdus;
  uint16_t m_txCenterFreq;
  uint64_t m_uid;
  mutable std::optional<WifiTxVector> m_txVector;
  const WifiPhyOperatingChannel &m_operatingChannel;

private:
  virtual WifiTxVector DoGetTxVector() const;

  bool m_truncatedTx;
  uint8_t m_txPowerLevel;
  uint8_t m_txAntennas;

  uint16_t m_txChannelWidth;
};

std::ostream &operator<<(std::ostream &os, const Ptr<const WifiPpdu> &ppdu);

std::ostream &operator<<(std::ostream &os, const WifiConstPsduMap &psdus);

} // namespace ns3

#endif
