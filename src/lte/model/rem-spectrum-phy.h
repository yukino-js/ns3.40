
#ifndef REM_SPECTRUM_PHY_H
#define REM_SPECTRUM_PHY_H

#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-value.h>

#include <fstream>
#include <string>

namespace ns3 {

class RemSpectrumPhy : public SpectrumPhy {
public:
  RemSpectrumPhy();
  ~RemSpectrumPhy() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  void SetChannel(Ptr<SpectrumChannel> c) override;
  void SetMobility(Ptr<MobilityModel> m) override;
  void SetDevice(Ptr<NetDevice> d) override;
  Ptr<MobilityModel> GetMobility() const override;
  Ptr<NetDevice> GetDevice() const override;
  Ptr<const SpectrumModel> GetRxSpectrumModel() const override;
  Ptr<Object> GetAntenna() const override;
  void StartRx(Ptr<SpectrumSignalParameters> params) override;

  void SetRxSpectrumModel(Ptr<const SpectrumModel> m);

  double GetSinr(double noisePower) const;

  void Deactivate();

  bool IsActive() const;

  void Reset();

  void SetUseDataChannel(bool value);

  void SetRbId(int32_t rbId);

private:
  Ptr<MobilityModel> m_mobility;
  Ptr<const SpectrumModel> m_rxSpectrumModel;

  double m_referenceSignalPower;
  double m_sumPower;

  bool m_active;

  bool m_useDataChannel;
  int32_t m_rbId;
};

} // namespace ns3

#endif
