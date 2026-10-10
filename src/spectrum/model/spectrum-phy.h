
#ifndef SPECTRUM_PHY_H
#define SPECTRUM_PHY_H

#include <ns3/nstime.h>
#include <ns3/object.h>

namespace ns3 {

class PacketBurst;
class SpectrumChannel;
class MobilityModel;
class AntennaModel;
class PhasedArrayModel;
class SpectrumValue;
class SpectrumModel;
class NetDevice;
struct SpectrumSignalParameters;

class SpectrumPhy : public Object {
public:
  SpectrumPhy();
  ~SpectrumPhy() override;

  SpectrumPhy(const SpectrumPhy &) = delete;
  SpectrumPhy &operator=(const SpectrumPhy &) = delete;

  static TypeId GetTypeId();

  virtual void SetDevice(Ptr<NetDevice> d) = 0;

  virtual Ptr<NetDevice> GetDevice() const = 0;

  virtual void SetMobility(Ptr<MobilityModel> m) = 0;

  virtual Ptr<MobilityModel> GetMobility() const = 0;

  virtual void SetChannel(Ptr<SpectrumChannel> c) = 0;

  virtual Ptr<const SpectrumModel> GetRxSpectrumModel() const = 0;

  virtual Ptr<Object> GetAntenna() const = 0;

  virtual void StartRx(Ptr<SpectrumSignalParameters> params) = 0;
};
} // namespace ns3

#endif
