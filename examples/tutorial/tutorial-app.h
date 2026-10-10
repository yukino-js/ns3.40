

#ifndef TUTORIAL_APP_H
#define TUTORIAL_APP_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

namespace ns3 {

class Application;

class TutorialApp : public Application {
public:
  TutorialApp();
  ~TutorialApp() override;

  static TypeId GetTypeId();

  void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize,
             uint32_t nPackets, DataRate dataRate);

private:
  void StartApplication() override;
  void StopApplication() override;

  void ScheduleTx();
  void SendPacket();

  Ptr<Socket> m_socket;
  Address m_peer;
  uint32_t m_packetSize;
  uint32_t m_nPackets;
  DataRate m_dataRate;
  EventId m_sendEvent;
  bool m_running;
  uint32_t m_packetsSent;
};

} // namespace ns3

#endif
