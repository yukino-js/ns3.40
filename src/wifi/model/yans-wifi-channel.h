
#ifndef YANS_WIFI_CHANNEL_H
#define YANS_WIFI_CHANNEL_H

#include "ns3/channel.h"

namespace ns3 {

class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
class YansWifiPhy;
class Packet;
class Time;
class WifiPpdu;

class YansWifiChannel : public Channel {
public:
  static TypeId GetTypeId();

  YansWifiChannel();
  ~YansWifiChannel() override;

  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

  void Add(Ptr<YansWifiPhy> phy);

  void SetPropagationLossModel(const Ptr<PropagationLossModel> loss);
  void SetPropagationDelayModel(const Ptr<PropagationDelayModel> delay);

  void Send(Ptr<YansWifiPhy> sender, Ptr<const WifiPpdu> ppdu,
            double txPowerDbm) const;

  int64_t AssignStreams(int64_t stream);

private:
  typedef std::vector<Ptr<YansWifiPhy>> PhyList;

  static void Receive(Ptr<YansWifiPhy> receiver, Ptr<const WifiPpdu> ppdu,
                      double txPowerDbm);

  PhyList m_phyList;
  Ptr<PropagationLossModel> m_loss;
  Ptr<PropagationDelayModel> m_delay;
};

} // namespace ns3

#endif
