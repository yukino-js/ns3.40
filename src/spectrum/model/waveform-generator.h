
#ifndef WAVEFORM_GENERATOR_H
#define WAVEFORM_GENERATOR_H

#include "spectrum-channel.h"
#include "spectrum-phy.h"
#include "spectrum-value.h"

#include <ns3/event-id.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/trace-source-accessor.h>

namespace ns3 {

class AntennaModel;

class WaveformGenerator : public SpectrumPhy {
public:
  WaveformGenerator();
  ~WaveformGenerator() override;

  static TypeId GetTypeId();

  void SetChannel(Ptr<SpectrumChannel> c) override;
  void SetMobility(Ptr<MobilityModel> m) override;
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<MobilityModel> GetMobility() const override;
  Ptr<NetDevice> GetDevice() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  void SetTxPowerSpectralDensity(Ptr<SpectrumValue> txs);

  void SetPeriod(Time period);

  Time GetPeriod() const;

  void SetDutyCycle(double value);

  double GetDutyCycle() const;

  void SetAntenna(Ptr<AntennaModel> a);

  virtual void Start();

  virtual void Stop();

private:
  void DoDispose() override;

  Ptr<MobilityModel> m_mobility;
  Ptr<AntennaModel> m_antenna;
  Ptr<NetDevice> m_netDevice;
  Ptr<SpectrumChannel> m_channel;

  virtual void GenerateWaveform();

  Ptr<SpectrumValue> m_txPowerSpectralDensity;
  Time m_period;
  double m_dutyCycle;
  Time m_startTime;
  EventId m_nextWave;

  TracedCallback<Ptr<const Packet>> m_phyTxStartTrace;
  TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;
};

} // namespace ns3

#endif
