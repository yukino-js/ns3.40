

#ifndef BS_SCHEDULER_H
#define BS_SCHEDULER_H

#include "dl-mac-messages.h"
#include "service-flow.h"
#include "wimax-phy.h"

#include "ns3/packet-burst.h"
#include "ns3/packet.h"

#include <list>

namespace ns3 {

class BaseStationNetDevice;
class GenericMacHeader;
class WimaxConnection;
class Cid;

class BSScheduler : public Object {
public:
  BSScheduler();
  BSScheduler(Ptr<BaseStationNetDevice> bs);
  ~BSScheduler() override;

  static TypeId GetTypeId();

  virtual std::list<std::pair<OfdmDlMapIe *, Ptr<PacketBurst>>> *
  GetDownlinkBursts() const = 0;
  virtual void AddDownlinkBurst(Ptr<const WimaxConnection> connection,
                                uint8_t diuc,
                                WimaxPhy::ModulationType modulationType,
                                Ptr<PacketBurst> burst) = 0;

  virtual void Schedule() = 0;
  virtual bool SelectConnection(Ptr<WimaxConnection> &connection) = 0;

  virtual Ptr<PacketBurst>
  CreateUgsBurst(ServiceFlow *serviceFlow,
                 WimaxPhy::ModulationType modulationType,
                 uint32_t availableSymbols) = 0;

  virtual Ptr<BaseStationNetDevice> GetBs();
  virtual void SetBs(Ptr<BaseStationNetDevice> bs);

  bool CheckForFragmentation(Ptr<WimaxConnection> connection,
                             int availableSymbols,
                             WimaxPhy::ModulationType modulationType);

private:
  Ptr<BaseStationNetDevice> m_bs;
  std::list<std::pair<OfdmDlMapIe *, Ptr<PacketBurst>>> *m_downlinkBursts;
};

} // namespace ns3

#endif
