
#ifndef MESH_WIFI_INTERFACE_MAC_H
#define MESH_WIFI_INTERFACE_MAC_H

#include "mesh-wifi-interface-mac-plugin.h"

#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/mgt-headers.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/wifi-mac.h"

#include <map>
#include <stdint.h>

namespace ns3 {

class UniformRandomVariable;

class MeshWifiInterfaceMac : public WifiMac {
public:
  static TypeId GetTypeId();
  MeshWifiInterfaceMac();
  ~MeshWifiInterfaceMac() override;

  void Enqueue(Ptr<Packet> packet, Mac48Address to, Mac48Address from) override;
  void Enqueue(Ptr<Packet> packet, Mac48Address to) override;
  bool SupportsSendFrom() const override;
  void SetLinkUpCallback(Callback<void> linkUp) override;
  bool CanForwardPacketsTo(Mac48Address to) const override;

  void SetMeshPointAddress(Mac48Address addr);
  Mac48Address GetMeshPointAddress() const;

  void SetRandomStartDelay(Time interval);
  void SetBeaconInterval(Time interval);
  Time GetBeaconInterval() const;
  Time GetTbtt() const;
  void ShiftTbtt(Time shift);

  void InstallPlugin(Ptr<MeshWifiInterfaceMacPlugin> plugin);

  uint16_t GetFrequencyChannel() const;
  void SwitchFrequencyChannel(uint16_t new_id);

  void SendManagementFrame(Ptr<Packet> frame, const WifiMacHeader &hdr);
  bool CheckSupportedRates(AllSupportedRates rates) const;

  AllSupportedRates GetSupportedRates() const;

  void SetLinkMetricCallback(
      Callback<uint32_t, Mac48Address, Ptr<MeshWifiInterfaceMac>> cb);
  uint32_t GetLinkMetric(Mac48Address peerAddress);

  void Report(std::ostream &os) const;

  void ResetStats();

  void SetBeaconGeneration(bool enable);
  void ConfigureStandard(WifiStandard standard) override;
  void ConfigureContentionWindow(uint32_t cwMin, uint32_t cwMax) override;

  int64_t AssignStreams(int64_t stream);

private:
  void Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) override;
  void ForwardDown(Ptr<Packet> packet, Mac48Address from, Mac48Address to);
  void SendBeacon();
  void ScheduleNextBeacon();
  bool GetBeaconGeneration() const;
  void DoDispose() override;

private:
  typedef std::vector<Ptr<MeshWifiInterfaceMacPlugin>> PluginList;

  void DoInitialize() override;

  bool m_beaconEnable;
  Time m_beaconInterval;
  Time m_randomStart;
  Time m_tbtt;

  Mac48Address m_mpAddress;

  EventId m_beaconSendEvent;
  PluginList m_plugins;
  Callback<uint32_t, Mac48Address, Ptr<MeshWifiInterfaceMac>>
      m_linkMetricCallback;

  struct Statistics {
    uint16_t recvBeacons;
    uint32_t sentFrames;
    uint32_t sentBytes;
    uint32_t recvFrames;
    uint32_t recvBytes;
    void Print(std::ostream &os) const;
    Statistics();
  };

  Statistics m_stats;

  WifiStandard m_standard;

  Ptr<UniformRandomVariable> m_coefficient;
};

} // namespace ns3

#endif
