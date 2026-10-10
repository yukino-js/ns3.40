
#ifndef WIFI_TX_PARAMETERS_H
#define WIFI_TX_PARAMETERS_H

#include "wifi-mac-header.h"
#include "wifi-tx-vector.h"

#include "ns3/nstime.h"

#include <map>
#include <memory>
#include <set>

namespace ns3 {

class WifiMpdu;
struct WifiProtection;
struct WifiAcknowledgment;

class WifiTxParameters {
public:
  WifiTxParameters();
  WifiTxParameters(const WifiTxParameters &txParams);

  WifiTxParameters &operator=(const WifiTxParameters &txParams);

  WifiTxVector m_txVector;
  std::unique_ptr<WifiProtection> m_protection;
  std::unique_ptr<WifiAcknowledgment> m_acknowledgment;
  Time m_txDuration{Time::Min()};

  void Clear();

  void AddMpdu(Ptr<const WifiMpdu> mpdu);

  void AggregateMsdu(Ptr<const WifiMpdu> msdu);

  uint32_t GetSizeIfAddMpdu(Ptr<const WifiMpdu> mpdu) const;

  std::pair<uint32_t, uint32_t>
  GetSizeIfAggregateMsdu(Ptr<const WifiMpdu> msdu) const;

  uint32_t GetSize(Mac48Address receiver) const;

  struct PsduInfo {
    WifiMacHeader header;
    uint32_t amsduSize;
    uint32_t ampduSize;
    std::map<uint8_t, std::set<uint16_t>> seqNumbers;
  };

  const PsduInfo *GetPsduInfo(Mac48Address receiver) const;

  typedef std::map<Mac48Address, PsduInfo> PsduInfoMap;

  const PsduInfoMap &GetPsduInfoMap() const;

  void Print(std::ostream &os) const;

private:
  PsduInfoMap m_info;
};

std::ostream &operator<<(std::ostream &os, const WifiTxParameters *txParams);

} // namespace ns3

#endif
