
#ifndef LTE_UE_NET_DEVICE_H
#define LTE_UE_NET_DEVICE_H

#include "component-carrier-ue.h"
#include "lte-net-device.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"

#include <map>
#include <vector>

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class LtePhy;
class LteUePhy;
class LteEnbNetDevice;
class LteUeMac;
class LteUeRrc;
class EpcUeNas;
class EpcTft;
class LteUeComponentCarrierManager;

class LteUeNetDevice : public LteNetDevice {
public:
  static TypeId GetTypeId();

  LteUeNetDevice();
  ~LteUeNetDevice() override;
  void DoDispose() override;

  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;

  Ptr<LteUeMac> GetMac() const;

  Ptr<LteUeRrc> GetRrc() const;

  Ptr<LteUePhy> GetPhy() const;

  Ptr<EpcUeNas> GetNas() const;

  Ptr<LteUeComponentCarrierManager> GetComponentCarrierManager() const;

  uint64_t GetImsi() const;

  uint32_t GetDlEarfcn() const;

  void SetDlEarfcn(uint32_t earfcn);

  uint32_t GetCsgId() const;

  void SetCsgId(uint32_t csgId);

  void SetTargetEnb(Ptr<LteEnbNetDevice> enb);

  Ptr<LteEnbNetDevice> GetTargetEnb();

  void SetCcMap(std::map<uint8_t, Ptr<ComponentCarrierUe>> ccm);

  std::map<uint8_t, Ptr<ComponentCarrierUe>> GetCcMap();

protected:
  void DoInitialize() override;

private:
  bool m_isConstructed;

  void UpdateConfig();

  Ptr<LteEnbNetDevice> m_targetEnb;

  Ptr<LteUeRrc> m_rrc;
  Ptr<EpcUeNas> m_nas;
  Ptr<LteUeComponentCarrierManager> m_componentCarrierManager;

  uint64_t m_imsi;

  uint32_t m_dlEarfcn;

  uint32_t m_csgId;

  std::map<uint8_t, Ptr<ComponentCarrierUe>> m_ccMap;
};

} // namespace ns3

#endif
