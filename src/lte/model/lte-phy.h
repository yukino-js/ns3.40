
#ifndef LTE_PHY_H
#define LTE_PHY_H

#include "lte-spectrum-phy.h"

#include <ns3/generic-phy.h>
#include <ns3/mobility-model.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-interference.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-signal-parameters.h>
#include <ns3/spectrum-value.h>

namespace ns3 {

class PacketBurst;
class LteNetDevice;
class LteControlMessage;

class LtePhy : public Object {
public:
  LtePhy();

  LtePhy(Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy);

  ~LtePhy() override;

  static TypeId GetTypeId();

  void SetDevice(Ptr<LteNetDevice> d);
  Ptr<LteNetDevice> GetDevice() const;

  Ptr<LteSpectrumPhy> GetDownlinkSpectrumPhy();

  Ptr<LteSpectrumPhy> GetUplinkSpectrumPhy();

  virtual void DoSendMacPdu(Ptr<Packet> p) = 0;

  void SetDownlinkChannel(Ptr<SpectrumChannel> c);

  void SetUplinkChannel(Ptr<SpectrumChannel> c);

  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity() = 0;

  void DoDispose() override;

  void SetTti(double tti);
  double GetTti() const;

  void DoSetCellId(uint16_t cellId);

  uint8_t GetRbgSize() const;

  uint16_t GetSrsPeriodicity(uint16_t srcCi) const;

  uint16_t GetSrsSubframeOffset(uint16_t srcCi) const;

  void SetMacPdu(Ptr<Packet> p);

  Ptr<PacketBurst> GetPacketBurst();

  void SetControlMessages(Ptr<LteControlMessage> m);

  std::list<Ptr<LteControlMessage>> GetControlMessages();

  virtual void GenerateCtrlCqiReport(const SpectrumValue &sinr) = 0;

  virtual void GenerateDataCqiReport(const SpectrumValue &sinr) = 0;

  virtual void ReportInterference(const SpectrumValue &interf) = 0;

  virtual void ReportRsReceivedPower(const SpectrumValue &power) = 0;

  void SetComponentCarrierId(uint8_t index);

  uint8_t GetComponentCarrierId() const;

protected:
  Ptr<LteNetDevice> m_netDevice;

  Ptr<LteSpectrumPhy> m_downlinkSpectrumPhy;
  Ptr<LteSpectrumPhy> m_uplinkSpectrumPhy;

  double m_txPower;
  double m_noiseFigure;

  double m_tti;
  uint16_t m_ulBandwidth;
  uint16_t m_dlBandwidth;
  uint8_t m_rbgSize;
  uint32_t m_dlEarfcn;
  uint32_t m_ulEarfcn;

  std::vector<Ptr<PacketBurst>> m_packetBurstQueue;
  std::vector<std::list<Ptr<LteControlMessage>>> m_controlMessagesQueue;
  uint8_t m_macChTtiDelay;

  uint16_t m_cellId;

  uint8_t m_componentCarrierId;
};

} // namespace ns3

#endif
