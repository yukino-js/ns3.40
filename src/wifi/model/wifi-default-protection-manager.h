
#ifndef WIFI_DEFAULT_PROTECTION_MANAGER_H
#define WIFI_DEFAULT_PROTECTION_MANAGER_H

#include "wifi-protection-manager.h"

namespace ns3 {

class WifiTxParameters;
class WifiMpdu;
class WifiMacHeader;

class WifiDefaultProtectionManager : public WifiProtectionManager {
public:
  static TypeId GetTypeId();

  WifiDefaultProtectionManager();
  ~WifiDefaultProtectionManager() override;

  std::unique_ptr<WifiProtection>
  TryAddMpdu(Ptr<const WifiMpdu> mpdu,
             const WifiTxParameters &txParams) override;
  std::unique_ptr<WifiProtection>
  TryAggregateMsdu(Ptr<const WifiMpdu> msdu,
                   const WifiTxParameters &txParams) override;

protected:
  virtual std::unique_ptr<WifiProtection>
  GetPsduProtection(const WifiMacHeader &hdr, uint32_t size,
                    const WifiTxVector &txVector) const;

private:
  virtual std::unique_ptr<WifiProtection>
  TryAddMpduToMuPpdu(Ptr<const WifiMpdu> mpdu,
                     const WifiTxParameters &txParams);

  virtual std::unique_ptr<WifiProtection>
  TryUlMuTransmission(Ptr<const WifiMpdu> mpdu,
                      const WifiTxParameters &txParams);

  bool m_sendMuRts;
};

} // namespace ns3

#endif
