
#ifndef HWMP_STATE_H
#define HWMP_STATE_H

#include "hwmp-protocol.h"

#include "ns3/mesh-wifi-interface-mac-plugin.h"

namespace ns3 {

class MeshWifiInterfaceMac;
class WifiActionHeader;

namespace dot11s {

class IePreq;
class IePrep;
class IePerr;

class HwmpProtocolMac : public MeshWifiInterfaceMacPlugin {
public:
  HwmpProtocolMac(uint32_t ifIndex, Ptr<HwmpProtocol> protocol);
  ~HwmpProtocolMac() override;

  void SetParent(Ptr<MeshWifiInterfaceMac> parent) override;
  bool Receive(Ptr<Packet> packet, const WifiMacHeader &header) override;
  bool UpdateOutcomingFrame(Ptr<Packet> packet, WifiMacHeader &header,
                            Mac48Address from, Mac48Address to) override;
  void UpdateBeacon(MeshWifiBeacon &beacon) const override {};
  int64_t AssignStreams(int64_t stream) override;

private:
  friend class HwmpProtocol;
  static WifiActionHeader GetWifiActionHeader();
  void SendPreq(IePreq preq);
  void SendPreqVector(std::vector<IePreq> preq);
  void SendPrep(IePrep prep, Mac48Address receiver);
  void ForwardPerr(std::vector<HwmpProtocol::FailedDestination> destinations,
                   std::vector<Mac48Address> receivers);
  void InitiatePerr(std::vector<HwmpProtocol::FailedDestination> destinations,
                    std::vector<Mac48Address> receivers);
  void RequestDestination(Mac48Address dest, uint32_t originator_seqno,
                          uint32_t dst_seqno);

  void SendMyPreq();
  void SendMyPerr();
  uint32_t GetLinkMetric(Mac48Address peerAddress) const;
  uint16_t GetChannelId() const;
  void Report(std::ostream &os) const;
  void ResetStats();

private:
  Ptr<MeshWifiInterfaceMac> m_parent;
  uint32_t m_ifIndex;
  Ptr<HwmpProtocol> m_protocol;

  EventId m_preqTimer;
  std::vector<IePreq> m_myPreq;

  EventId m_perrTimer;

  struct MyPerr {
    std::vector<HwmpProtocol::FailedDestination> destinations;
    std::vector<Mac48Address> receivers;
  };

  MyPerr m_myPerr;

  struct Statistics {
    uint16_t txPreq;
    uint16_t rxPreq;
    uint16_t txPrep;
    uint16_t rxPrep;
    uint16_t txPerr;
    uint16_t rxPerr;
    uint16_t txMgt;
    uint32_t txMgtBytes;
    uint16_t rxMgt;
    uint32_t rxMgtBytes;
    uint16_t txData;
    uint32_t txDataBytes;
    uint16_t rxData;
    uint32_t rxDataBytes;
    void Print(std::ostream &os) const;
    Statistics();
  };

  Statistics m_stats;

private:
  bool ReceiveData(Ptr<Packet> packet, const WifiMacHeader &header);
  bool ReceiveAction(Ptr<Packet> packet, const WifiMacHeader &header);
};
} // namespace dot11s
} // namespace ns3
#endif
