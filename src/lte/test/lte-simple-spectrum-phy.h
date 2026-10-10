
#ifndef LTE_SIMPLE_SPECTRUM_PHY_H
#define LTE_SIMPLE_SPECTRUM_PHY_H

#include <ns3/event-id.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-value.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class LteSimpleSpectrumPhy : public SpectrumPhy {
public:
  LteSimpleSpectrumPhy();
  ~LteSimpleSpectrumPhy() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  void SetChannel(Ptr<SpectrumChannel> c) override;
  void SetMobility(Ptr<MobilityModel> m) override;
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<MobilityModel> GetMobility() const override;
  Ptr<NetDevice> GetDevice() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  void SetRxSpectrumModel(Ptr<const SpectrumModel> model);

  void SetCellId(uint16_t cellId);

private:
  Ptr<MobilityModel> m_mobility;
  Ptr<AntennaModel> m_antenna;
  Ptr<NetDevice> m_device;
  Ptr<SpectrumChannel> m_channel;
  Ptr<const SpectrumModel> m_rxSpectrumModel;

  uint16_t m_cellId;

  TracedCallback<Ptr<const SpectrumValue>> m_rxStart;
};

} // namespace ns3

#endif
