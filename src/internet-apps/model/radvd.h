
#ifndef RADVD_H
#define RADVD_H

#include "radvd-interface.h"

#include "ns3/application.h"

#include <map>

namespace ns3 {

class UniformRandomVariable;
class Socket;

class Radvd : public Application {
public:
  static TypeId GetTypeId();

  Radvd();

  ~Radvd() override;

  static const uint32_t MAX_RA_DELAY_TIME = 500;
  static const uint32_t MAX_INITIAL_RTR_ADVERTISEMENTS = 3;
  static const uint32_t MAX_INITIAL_RTR_ADVERT_INTERVAL = 16000;
  static const uint32_t MIN_DELAY_BETWEEN_RAS = 3000;

  void AddConfiguration(Ptr<RadvdInterface> routerInterface);

  int64_t AssignStreams(int64_t stream);

protected:
  void DoDispose() override;

private:
  typedef std::list<Ptr<RadvdInterface>> RadvdInterfaceList;
  typedef std::list<Ptr<RadvdInterface>>::iterator RadvdInterfaceListI;
  typedef std::list<Ptr<RadvdInterface>>::const_iterator RadvdInterfaceListCI;

  typedef std::map<uint32_t, EventId> EventIdMap;
  typedef std::map<uint32_t, EventId>::iterator EventIdMapI;
  typedef std::map<uint32_t, EventId>::const_iterator EventIdMapCI;

  typedef std::map<uint32_t, Ptr<Socket>> SocketMap;
  typedef std::map<uint32_t, Ptr<Socket>>::iterator SocketMapI;
  typedef std::map<uint32_t, Ptr<Socket>>::const_iterator SocketMapCI;

  void StartApplication() override;

  void StopApplication() override;

  void Send(Ptr<RadvdInterface> config,
            Ipv6Address dst = Ipv6Address::GetAllNodesMulticast(),
            bool reschedule = false);

  void HandleRead(Ptr<Socket> socket);

  Ptr<Socket> m_recvSocket;

  SocketMap m_sendSockets;

  RadvdInterfaceList m_configurations;

  EventIdMap m_unsolicitedEventIds;

  EventIdMap m_solicitedEventIds;

  Ptr<UniformRandomVariable> m_jitter;
};

} // namespace ns3

#endif
