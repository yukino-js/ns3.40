
#ifndef TV_SPECTRUM_TRANSMITTER_H
#define TV_SPECTRUM_TRANSMITTER_H

#include "spectrum-channel.h"
#include "spectrum-phy.h"
#include "spectrum-signal-parameters.h"
#include "spectrum-value.h"

#include <ns3/antenna-model.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>

namespace ns3 {

class TvSpectrumTransmitter : public SpectrumPhy {
public:
  enum TvType { TVTYPE_ANALOG, TVTYPE_8VSB, TVTYPE_COFDM };

  TvSpectrumTransmitter();
  ~TvSpectrumTransmitter() override;

  static TypeId GetTypeId();

  void SetChannel(Ptr<SpectrumChannel> c) override;
  void SetMobility(Ptr<MobilityModel> m) override;
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<MobilityModel> GetMobility() const override;
  Ptr<NetDevice> GetDevice() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  Ptr<SpectrumChannel> GetChannel() const;

  virtual void CreateTvPsd();

  Ptr<SpectrumValue> GetTxPsd() const;

  virtual void Start();

  virtual void Stop();

private:
  Ptr<MobilityModel> m_mobility;
  Ptr<AntennaModel> m_antenna;
  Ptr<NetDevice> m_netDevice;
  Ptr<SpectrumChannel> m_channel;

  virtual void SetupTx();

  TvType m_tvType;
  double m_startFrequency;
  double m_channelBandwidth;
  double m_basePsd;
  Ptr<SpectrumValue> m_txPsd;
  Time m_startingTime;
  Time m_transmitDuration;
  bool m_active;
};

} // namespace ns3

#endif
