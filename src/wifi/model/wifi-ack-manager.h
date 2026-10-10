
#ifndef WIFI_ACK_MANAGER_H
#define WIFI_ACK_MANAGER_H

#include "wifi-acknowledgment.h"

#include "ns3/object.h"

#include <memory>

namespace ns3 {

class WifiTxParameters;
class WifiMpdu;
class WifiPsdu;
class WifiMac;
class WifiRemoteStationManager;

class WifiAckManager : public Object {
public:
  static TypeId GetTypeId();
  WifiAckManager();
  ~WifiAckManager() override;

  void SetWifiMac(Ptr<WifiMac> mac);
  void SetLinkId(uint8_t linkId);

  static void SetQosAckPolicy(Ptr<WifiMpdu> item,
                              const WifiAcknowledgment *acknowledgment);

  static void SetQosAckPolicy(Ptr<WifiPsdu> psdu,
                              const WifiAcknowledgment *acknowledgment);

  virtual std::unique_ptr<WifiAcknowledgment>
  TryAddMpdu(Ptr<const WifiMpdu> mpdu, const WifiTxParameters &txParams) = 0;

  virtual std::unique_ptr<WifiAcknowledgment>
  TryAggregateMsdu(Ptr<const WifiMpdu> msdu,
                   const WifiTxParameters &txParams) = 0;

protected:
  void DoDispose() override;

  Ptr<WifiRemoteStationManager> GetWifiRemoteStationManager() const;

  Ptr<WifiMac> m_mac;
  uint8_t m_linkId;
};

} // namespace ns3

#endif
