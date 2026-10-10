
#ifndef LTE_ENB_NET_DEVICE_H
#define LTE_ENB_NET_DEVICE_H

#include "component-carrier.h"
#include "lte-net-device.h"

#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"

#include <map>
#include <vector>

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class LtePhy;
class LteEnbPhy;
class LteEnbMac;
class LteEnbRrc;
class FfMacScheduler;
class LteHandoverAlgorithm;
class LteAnr;
class LteFfrAlgorithm;
class LteEnbComponentCarrierManager;

class LteEnbNetDevice : public LteNetDevice {
public:
  static TypeId GetTypeId();

  LteEnbNetDevice();

  ~LteEnbNetDevice() override;
  void DoDispose() override;

  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;

  Ptr<LteEnbMac> GetMac() const;

  Ptr<LteEnbMac> GetMac(uint8_t index) const;

  Ptr<LteEnbPhy> GetPhy() const;

  Ptr<LteEnbPhy> GetPhy(uint8_t index) const;

  Ptr<LteEnbRrc> GetRrc() const;

  Ptr<LteEnbComponentCarrierManager> GetComponentCarrierManager() const;

  uint16_t GetCellId() const;

  std::vector<uint16_t> GetCellIds() const;

  bool HasCellId(uint16_t cellId) const;

  uint16_t GetUlBandwidth() const;

  void SetUlBandwidth(uint16_t bw);

  uint16_t GetDlBandwidth() const;

  void SetDlBandwidth(uint16_t bw);

  uint32_t GetDlEarfcn() const;

  void SetDlEarfcn(uint32_t earfcn);

  uint32_t GetUlEarfcn() const;

  void SetUlEarfcn(uint32_t earfcn);

  uint32_t GetCsgId() const;

  void SetCsgId(uint32_t csgId);

  bool GetCsgIndication() const;

  void SetCsgIndication(bool csgIndication);

  void SetCcMap(std::map<uint8_t, Ptr<ComponentCarrierBaseStation>> ccm);

  std::map<uint8_t, Ptr<ComponentCarrierBaseStation>> GetCcMap() const;

protected:
  void DoInitialize() override;

private:
  bool m_isConstructed;
  bool m_isConfigured;

  void UpdateConfig();

  Ptr<LteEnbRrc> m_rrc;

  Ptr<LteHandoverAlgorithm> m_handoverAlgorithm;

  Ptr<LteAnr> m_anr;

  Ptr<LteFfrAlgorithm> m_ffrAlgorithm;

  uint16_t m_cellId;

  uint16_t m_dlBandwidth;
  uint16_t m_ulBandwidth;

  uint32_t m_dlEarfcn;
  uint32_t m_ulEarfcn;

  uint16_t m_csgId;
  bool m_csgIndication;

  std::map<uint8_t, Ptr<ComponentCarrierBaseStation>> m_ccMap;

  Ptr<LteEnbComponentCarrierManager> m_componentCarrierManager;
};

} // namespace ns3

#endif
