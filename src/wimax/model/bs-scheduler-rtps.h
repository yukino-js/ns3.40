

#ifndef BS_SCHEDULER_RTPS_H
#define BS_SCHEDULER_RTPS_H

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

class BSSchedulerRtps : public BSScheduler {
public:
  BSSchedulerRtps();
  BSSchedulerRtps(Ptr<BaseStationNetDevice> bs);
  ~BSSchedulerRtps() override;

  static TypeId GetTypeId();

  std::list<std::pair<OfdmDlMapIe *, Ptr<PacketBurst>>> *
  GetDownlinkBursts() const override;

  void AddDownlinkBurst(Ptr<const WimaxConnection> connection, uint8_t diuc,
                        WimaxPhy::ModulationType modulationType,
                        Ptr<PacketBurst> burst) override;
  void Schedule() override;
  bool SelectIRandBCConnection(Ptr<WimaxConnection> &connection);
  bool SelectMenagementConnection(Ptr<WimaxConnection> &connection);
  bool SelectUGSConnection(Ptr<WimaxConnection> &connection);
  bool SelectRTPSConnection(Ptr<WimaxConnection> &connection);
  bool SelectNRTPSConnection(Ptr<WimaxConnection> &connection);
  bool SelectBEConnection(Ptr<WimaxConnection> &connection);

  bool SelectConnection(Ptr<WimaxConnection> &connection) override;

  void BSSchedulerBroadcastConnection(uint32_t &availableSymbols);

  void BSSchedulerInitialRangingConnection(uint32_t &availableSymbols);

  void BSSchedulerBasicConnection(uint32_t &availableSymbols);

  void BSSchedulerPrimaryConnection(uint32_t &availableSymbols);
  void BSSchedulerUGSConnection(uint32_t &availableSymbols);
  void BSSchedulerRTPSConnection(uint32_t &availableSymbols);

  void BSSchedulerNRTPSConnection(uint32_t &availableSymbols);

  void BSSchedulerBEConnection(uint32_t &availableSymbols);

  Ptr<PacketBurst> CreateUgsBurst(ServiceFlow *serviceFlow,
                                  WimaxPhy::ModulationType modulationType,
                                  uint32_t availableSymbols) override;

private:
  std::list<std::pair<OfdmDlMapIe *, Ptr<PacketBurst>>> *m_downlinkBursts;
};

} // namespace ns3

#endif
