
#ifndef POINT_TO_POINT_CHANNEL_H
#define POINT_TO_POINT_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <list>

namespace ns3 {

class PointToPointNetDevice;
class Packet;

class PointToPointChannel : public Channel {
public:
  static TypeId GetTypeId();

  PointToPointChannel();

  void Attach(Ptr<PointToPointNetDevice> device);

  virtual bool TransmitStart(Ptr<const Packet> p,
                             Ptr<PointToPointNetDevice> src, Time txTime);

  std::size_t GetNDevices() const override;

  Ptr<PointToPointNetDevice> GetPointToPointDevice(std::size_t i) const;

  Ptr<NetDevice> GetDevice(std::size_t i) const override;

protected:
  Time GetDelay() const;

  bool IsInitialized() const;

  Ptr<PointToPointNetDevice> GetSource(uint32_t i) const;

  Ptr<PointToPointNetDevice> GetDestination(uint32_t i) const;

  typedef void (*TxRxAnimationCallback)(Ptr<const Packet> packet,
                                        Ptr<NetDevice> txDevice,
                                        Ptr<NetDevice> rxDevice, Time duration,
                                        Time lastBitTime);

private:
  static const std::size_t N_DEVICES = 2;

  Time m_delay;
  std::size_t m_nDevices;

  TracedCallback<Ptr<const Packet>, Ptr<NetDevice>, Ptr<NetDevice>, Time, Time>
      m_txrxPointToPoint;

  enum WireState { INITIALIZING, IDLE, TRANSMITTING, PROPAGATING };

  class Link {
  public:
    Link() : m_state(INITIALIZING), m_src(nullptr), m_dst(nullptr) {}

    WireState m_state;
    Ptr<PointToPointNetDevice> m_src;
    Ptr<PointToPointNetDevice> m_dst;
  };

  Link m_link[N_DEVICES];
};

} // namespace ns3

#endif
