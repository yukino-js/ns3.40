
#ifndef WIMAX_CHANNEL_H
#define WIMAX_CHANNEL_H

#include "wimax-connection.h"

#include "ns3/channel.h"
#include "ns3/log.h"

#include <list>

namespace ns3 {

class WimaxPhy;
class Packet;
class Position;
class PacketBurst;

class WimaxChannel : public Channel {
public:
  static TypeId GetTypeId();
  WimaxChannel();
  ~WimaxChannel() override;
  void Attach(Ptr<WimaxPhy> phy);
  std::size_t GetNDevices() const override;
  Ptr<NetDevice> GetDevice(std::size_t i) const override;

  virtual int64_t AssignStreams(int64_t stream) = 0;

private:
  virtual void DoAttach(Ptr<WimaxPhy> phy) = 0;

  virtual std::size_t DoGetNDevices() const = 0;
  virtual Ptr<NetDevice> DoGetDevice(std::size_t i) const = 0;
};

} // namespace ns3

#endif
