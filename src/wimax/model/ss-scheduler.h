

#ifndef SS_SCHEDULER_H
#define SS_SCHEDULER_H

#include "wimax-mac-header.h"
#include "wimax-phy.h"

#include "ns3/packet-burst.h"
#include "ns3/packet.h"

#include <stdint.h>

namespace ns3 {

class SubscriberStationNetDevice;
class WimaxConnection;

class SSScheduler : public Object {
public:
  static TypeId GetTypeId();
  SSScheduler(Ptr<SubscriberStationNetDevice> ss);
  ~SSScheduler() override;

  SSScheduler(const SSScheduler &) = delete;
  SSScheduler &operator=(const SSScheduler &) = delete;

  void SetPollMe(bool pollMe);
  bool GetPollMe() const;
  Ptr<PacketBurst> Schedule(uint16_t availableSymbols,
                            WimaxPhy::ModulationType modulationType,
                            MacHeaderType::HeaderType packetType,
                            Ptr<WimaxConnection> &connection);

  void DoDispose() override;

private:
  Ptr<WimaxConnection> SelectConnection();
  Ptr<SubscriberStationNetDevice> m_ss;
  bool m_pollMe;
};

} // namespace ns3

#endif
