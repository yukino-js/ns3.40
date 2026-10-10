
#ifndef UAN_CHANNEL_H
#define UAN_CHANNEL_H

#include "uan-noise-model.h"
#include "uan-prop-model.h"

#include "ns3/channel.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"

#include <list>
#include <vector>

namespace ns3 {

class UanNetDevice;
class UanPhy;
class UanTransducer;
class UanTxMode;

class UanChannel : public Channel {
public:
  typedef std::vector<std::pair<Ptr<UanNetDevice>, Ptr<UanTransducer>>>
      UanDeviceList;

  UanChannel();
  ~UanChannel() override;

  static TypeId GetTypeId();

  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

  virtual void TxPacket(Ptr<UanTransducer> src, Ptr<Packet> packet,
                        double txPowerDb, UanTxMode txmode);

  void AddDevice(Ptr<UanNetDevice> dev, Ptr<UanTransducer> trans);

  void SetPropagationModel(Ptr<UanPropModel> prop);

  void SetNoiseModel(Ptr<UanNoiseModel> noise);

  double GetNoiseDbHz(double fKhz);

  void Clear();

protected:
  UanDeviceList m_devList;
  Ptr<UanPropModel> m_prop;
  Ptr<UanNoiseModel> m_noise;
  bool m_cleared;

  void SendUp(uint32_t i, Ptr<Packet> packet, double rxPowerDb,
              UanTxMode txMode, UanPdp pdp);

  void DoDispose() override;
};

} // namespace ns3

#endif
