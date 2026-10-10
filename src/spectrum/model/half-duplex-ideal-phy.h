
#ifndef HALF_DUPLEX_IDEAL_PHY_H
#define HALF_DUPLEX_IDEAL_PHY_H

#include "spectrum-channel.h"
#include "spectrum-interference.h"
#include "spectrum-phy.h"
#include "spectrum-signal-parameters.h"
#include "spectrum-value.h"

#include <ns3/data-rate.h>
#include <ns3/event-id.h>
#include <ns3/generic-phy.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>

namespace ns3 {

class HalfDuplexIdealPhy : public SpectrumPhy {
public:
  HalfDuplexIdealPhy();
  ~HalfDuplexIdealPhy() override;

  enum State { IDLE, TX, RX };

  static TypeId GetTypeId();

  void SetChannel(Ptr<SpectrumChannel> c) override;
  void SetMobility(Ptr<MobilityModel> m) override;
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<MobilityModel> GetMobility() const override;
  Ptr<NetDevice> GetDevice() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  void SetTxPowerSpectralDensity(Ptr<SpectrumValue> txPsd);

  void SetNoisePowerSpectralDensity(Ptr<const SpectrumValue> noisePsd);

  bool StartTx(Ptr<Packet> p);

  void SetRate(DataRate rate);

  DataRate GetRate() const;

  void SetGenericPhyTxEndCallback(GenericPhyTxEndCallback c);

  void SetGenericPhyRxStartCallback(GenericPhyRxStartCallback c);

  void SetGenericPhyRxEndErrorCallback(GenericPhyRxEndErrorCallback c);

  void SetGenericPhyRxEndOkCallback(GenericPhyRxEndOkCallback c);

  void SetAntenna(Ptr<AntennaModel> a);

private:
  void DoDispose() override;

  void ChangeState(State newState);
  void EndTx();
  void AbortRx();
  void EndRx();

  EventId m_endRxEventId;

  Ptr<MobilityModel> m_mobility;
  Ptr<AntennaModel> m_antenna;
  Ptr<NetDevice> m_netDevice;
  Ptr<SpectrumChannel> m_channel;

  Ptr<SpectrumValue> m_txPsd;
  Ptr<const SpectrumValue> m_rxPsd;
  Ptr<Packet> m_txPacket;
  Ptr<Packet> m_rxPacket;

  DataRate m_rate;
  State m_state;

  TracedCallback<Ptr<const Packet>> m_phyTxStartTrace;
  TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;
  TracedCallback<Ptr<const Packet>> m_phyRxStartTrace;
  TracedCallback<Ptr<const Packet>> m_phyRxAbortTrace;
  TracedCallback<Ptr<const Packet>> m_phyRxEndOkTrace;
  TracedCallback<Ptr<const Packet>> m_phyRxEndErrorTrace;

  GenericPhyTxEndCallback m_phyMacTxEndCallback;
  GenericPhyRxStartCallback m_phyMacRxStartCallback;
  GenericPhyRxEndErrorCallback m_phyMacRxEndErrorCallback;
  GenericPhyRxEndOkCallback m_phyMacRxEndOkCallback;

  SpectrumInterference m_interference;
};

} // namespace ns3

#endif
