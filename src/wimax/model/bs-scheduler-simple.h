

#ifndef BS_SCHEDULER_SIMPLE_H
#define BS_SCHEDULER_SIMPLE_H

#include "bs-scheduler.h"
#include "dl-mac-messages.h"
#include "wimax-phy.h"

#include "ns3/packet-burst.h"
#include "ns3/packet.h"

#include <list>

namespace ns3 {

class BaseStationNetDevice;
class GenericMacHeader;
class WimaxConnection;
class Cid;

class BSSchedulerSimple : public BSScheduler {
public:
  BSSchedulerSimple();
  BSSchedulerSimple(Ptr<BaseStationNetDevice> bs);
  ~BSSchedulerSimple() override;

  static TypeId GetTypeId();

  std::list<std::pair<OfdmDlMapIe *, Ptr<PacketBurst>>> *
  GetDownlinkBursts() const override;
  void AddDownlinkBurst(Ptr<const WimaxConnection> connection, uint8_t diuc,
                        WimaxPhy::ModulationType modulationType,
                        Ptr<PacketBurst> burst) override;

  void Schedule() override;
  bool SelectConnection(Ptr<WimaxConnection> &connection) override;
  Ptr<PacketBurst> CreateUgsBurst(ServiceFlow *serviceFlow,
                                  WimaxPhy::ModulationType modulationType,
                                  uint32_t availableSymbols) override;

private:
  std::list<std::pair<OfdmDlMapIe *, Ptr<PacketBurst>>> *m_downlinkBursts;
};

} // namespace ns3

#endif
