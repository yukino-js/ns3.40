
#ifndef WIFI_PROTECTION_MANAGER_H
#define WIFI_PROTECTION_MANAGER_H

#include "wifi-protection.h"

#include "ns3/object.h"

#include <memory>

namespace ns3 {

class WifiTxParameters;
class WifiMpdu;
class WifiMac;
class WifiRemoteStationManager;

class WifiProtectionManager : public Object {
public:
  static TypeId GetTypeId();
  WifiProtectionManager();
  ~WifiProtectionManager() override;

  void SetWifiMac(Ptr<WifiMac> mac);
  void SetLinkId(uint8_t linkId);

  virtual std::unique_ptr<WifiProtection>
  TryAddMpdu(Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) = 0;

  virtual std::unique_ptr<WifiProtection>
  TryAggregateMsdu(Ptr<const WifiMpdu> msdu,
                   const WifiTxParameters &txParams) = 0;

protected:
  void DoDispose() override;

  Ptr<WifiRemoteStationManager> GetWifiRemoteStationManager() const;

  void AddUserInfoToMuRts(CtrlTriggerHeader &muRts, uint16_t txWidth,
                          const Mac48Address &receiver) const;

  Ptr<WifiMac> m_mac;
  uint8_t m_linkId;
};

} // namespace ns3

#endif
