

#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/stats-module.h"

using namespace ns3;

class Sender : public Application {
public:
  static TypeId GetTypeId();

  Sender();
  ~Sender() override;

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void SendPacket();

  Ipv4Address m_destAddr;
  uint32_t m_destPort{0};
  uint32_t m_packetSize{0};
  Ptr<ConstantRandomVariable> m_interval;
  uint32_t m_nPackets{0};
  uint32_t m_count{0};

  Ptr<Socket> m_socket;
  EventId m_sendEvent;

  TracedCallback<Ptr<const Packet>> m_txTrace;
};

class Receiver : public Application {
public:
  static TypeId GetTypeId();

  Receiver();
  ~Receiver() override;

  void SetCounter(Ptr<CounterCalculator<>> calc);

  void SetDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> delay);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void Receive(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
  uint32_t m_port{0};

  Ptr<CounterCalculator<>> m_calc;
  Ptr<TimeMinMaxAvgTotalCalculator> m_delay;
};
