
#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "cid.h"
#include "wimax-connection.h"

#include "ns3/mac48-address.h"

#include <stdint.h>

namespace ns3 {

class CidFactory;
class SSRecord;
class RngRsp;
class WimaxNetDevice;
class SubscriberStationNetDevice;

class ConnectionManager : public Object {
public:
  static TypeId GetTypeId();
  ConnectionManager();
  ~ConnectionManager() override;
  void DoDispose() override;
  void SetCidFactory(CidFactory *cidFactory);
  void AllocateManagementConnections(SSRecord *ssRecord, RngRsp *rngrsp);
  Ptr<WimaxConnection> CreateConnection(Cid::Type type);
  void AddConnection(Ptr<WimaxConnection> connection, Cid::Type type);
  Ptr<WimaxConnection> GetConnection(Cid cid);
  std::vector<Ptr<WimaxConnection>> GetConnections(Cid::Type type) const;
  uint32_t GetNPackets(Cid::Type type,
                       ServiceFlow::SchedulingType schedulingType) const;
  bool HasPackets() const;

private:
  std::vector<Ptr<WimaxConnection>> m_basicConnections;
  std::vector<Ptr<WimaxConnection>> m_primaryConnections;
  std::vector<Ptr<WimaxConnection>> m_transportConnections;
  std::vector<Ptr<WimaxConnection>> m_multicastConnections;
  CidFactory *m_cidFactory;
};

} // namespace ns3

#endif
