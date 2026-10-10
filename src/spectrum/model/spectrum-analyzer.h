
#ifndef SPECTRUM_ANALYZER_H
#define SPECTRUM_ANALYZER_H

#include "spectrum-channel.h"
#include "spectrum-phy.h"
#include "spectrum-value.h"

#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>

#include <fstream>
#include <string>

namespace ns3 {

class SpectrumAnalyzer : public SpectrumPhy {
public:
  SpectrumAnalyzer();
  ~SpectrumAnalyzer() override;

  static TypeId GetTypeId();

  void SetChannel(Ptr<SpectrumChannel> c) override;
  void SetMobility(Ptr<MobilityModel> m) override;
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<MobilityModel> GetMobility() const override;
  Ptr<NetDevice> GetDevice() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  void SetRxSpectrumModel(Ptr<SpectrumModel> m);

  void SetAntenna(Ptr<AntennaModel> a);

  virtual void Start();

  virtual void Stop();

protected:
  void DoDispose() override;

private:
  Ptr<MobilityModel> m_mobility;
  Ptr<AntennaModel> m_antenna;
  Ptr<NetDevice> m_netDevice;
  Ptr<SpectrumChannel> m_channel;

  virtual void GenerateReport();

  void AddSignal(Ptr<const SpectrumValue> psd);
  void SubtractSignal(Ptr<const SpectrumValue> psd);
  void UpdateEnergyReceivedSoFar();

  Ptr<SpectrumModel> m_spectrumModel;
  Ptr<SpectrumValue> m_sumPowerSpectralDensity;
  Ptr<SpectrumValue> m_energySpectralDensity;
  double m_noisePowerSpectralDensity;
  Time m_resolution;
  Time m_lastChangeTime;
  bool m_active;

  TracedCallback<Ptr<const SpectrumValue>>
      m_averagePowerSpectralDensityReportTrace;
};

} // namespace ns3

#endif
