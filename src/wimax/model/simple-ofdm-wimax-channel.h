
#ifndef SIMPLE_OFDM_WIMAX_CHANNEL_H
#define SIMPLE_OFDM_WIMAX_CHANNEL_H

#include "bvec.h"
#include "simple-ofdm-send-param.h"
#include "wimax-channel.h"
#include "wimax-phy.h"

#include "ns3/propagation-loss-model.h"

#include <list>

namespace ns3 {

class Packet;
class PacketBurst;
class SimpleOfdmWimaxPhy;

class SimpleOfdmWimaxChannel : public WimaxChannel {
public:
  SimpleOfdmWimaxChannel();
  ~SimpleOfdmWimaxChannel() override;

  enum PropModel {
    RANDOM_PROPAGATION,
    FRIIS_PROPAGATION,
    LOG_DISTANCE_PROPAGATION,
    COST231_PROPAGATION
  };

  static TypeId GetTypeId();

  SimpleOfdmWimaxChannel(PropModel propModel);

  void Send(Time BlockTime, uint32_t burstSize, Ptr<WimaxPhy> phy,
            bool isFirstBlock, bool isLastBlock, uint64_t frequency,
            WimaxPhy::ModulationType modulationType, uint8_t direction,
            double txPowerDbm, Ptr<PacketBurst> burst);
  void SetPropagationModel(PropModel propModel);

  int64_t AssignStreams(int64_t stream) override;

private:
  void DoAttach(Ptr<WimaxPhy> phy) override;
  std::list<Ptr<SimpleOfdmWimaxPhy>> m_phyList;
  std::size_t DoGetNDevices() const override;
  void EndSendDummyBlock(Ptr<SimpleOfdmWimaxPhy> rxphy,
                         SimpleOfdmSendParam *param);
  Ptr<NetDevice> DoGetDevice(std::size_t i) const override;
  Ptr<PropagationLossModel> m_loss;
};

} // namespace ns3

#endif
