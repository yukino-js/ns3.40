
#ifndef PING_H
#define PING_H

#include "ns3/application.h"
#include "ns3/average.h"
#include "ns3/traced-callback.h"

#include <map>

namespace ns3 {

class Socket;

class Ping : public Application {
public:
  static TypeId GetTypeId();

  enum VerboseMode {
    VERBOSE = 0,
    QUIET = 1,
    SILENT = 2,
  };

  enum DropReason {
    DROP_TIMEOUT = 0,
    DROP_HOST_UNREACHABLE,
    DROP_NET_UNREACHABLE,
  };

  struct PingReport {
    uint32_t m_transmitted{0};
    uint32_t m_received{0};
    uint16_t m_loss{0};
    Time m_duration{0};
    double m_rttMin{0};
    double m_rttAvg{0};
    double m_rttMax{0};
    double m_rttMdev{0};
  };

  Ping();

  ~Ping() override;

  void SetRouters(const std::vector<Ipv6Address> &routers);

  typedef void (*TxTrace)(uint16_t seq, Ptr<const Packet> p);

  typedef void (*RttTrace)(uint16_t seq, Time rtt);

  typedef void (*DropTrace)(uint16_t seq, DropReason reason);

  typedef void (*ReportTrace)(const PingReport &report);

private:
  void Write64(uint8_t *buffer, const uint64_t data);

  uint64_t Read64(const uint8_t *buffer);

  void StartApplication() override;
  void StopApplication() override;
  void DoDispose() override;

  uint64_t GetApplicationSignature() const;

  void Receive(Ptr<Socket> socket);

  void Send();

  void PrintReport();

  Address m_interfaceAddress;
  Address m_destination;
  Time m_interval{Seconds(1)};

  uint32_t m_size{56};
  Ptr<Socket> m_socket;
  uint16_t m_seq{0};
  TracedCallback<uint16_t, Ptr<Packet>> m_txTrace;
  TracedCallback<uint16_t, Time> m_rttTrace;
  TracedCallback<uint16_t, DropReason> m_dropTrace;
  TracedCallback<const PingReport &> m_reportTrace;
  VerboseMode m_verbose{VerboseMode::VERBOSE};
  uint32_t m_recv{0};
  uint32_t m_duplicate{0};
  Time m_started;
  Average<double> m_avgRtt;
  EventId m_next;

  class EchoRequestData {
  public:
    EchoRequestData(Time txTimePar, bool ackedPar)
        : txTime(txTimePar), acked(ackedPar) {}

    Time txTime;
    bool acked{false};
  };

  std::vector<EchoRequestData> m_sent;
  uint32_t m_count{0};
  Time m_timeout{Seconds(1)};
  bool m_reportPrinted{false};
  bool m_useIpv6{false};
  bool m_multipleDestinations{false};

  std::vector<Ipv6Address> m_routers;

  uint64_t m_appSignature{0};
};

} // namespace ns3

#endif
