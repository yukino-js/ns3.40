

#ifndef ONOFF_APPLICATION_H
#define ONOFF_APPLICATION_H

#include "seq-ts-size-header.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class RandomVariableStream;
class Socket;

class OnOffApplication : public Application {
public:
  static TypeId GetTypeId();

  OnOffApplication();

  ~OnOffApplication() override;

  void SetMaxBytes(uint64_t maxBytes);

  Ptr<Socket> GetSocket() const;

  int64_t AssignStreams(int64_t stream);

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void CancelEvents();

  void StartSending();
  void StopSending();
  void SendPacket();

  Ptr<Socket> m_socket;
  Address m_peer;
  Address m_local;
  bool m_connected;
  Ptr<RandomVariableStream> m_onTime;
  Ptr<RandomVariableStream> m_offTime;
  DataRate m_cbrRate;
  DataRate m_cbrRateFailSafe;
  uint32_t m_pktSize;
  uint32_t m_residualBits;
  Time m_lastStartTime;
  uint64_t m_maxBytes;
  uint64_t m_totBytes;
  EventId m_startStopEvent;
  EventId m_sendEvent;
  TypeId m_tid;
  uint32_t m_seq{0};
  Ptr<Packet> m_unsentPacket;
  bool m_enableSeqTsSizeHeader{false};

  TracedCallback<Ptr<const Packet>> m_txTrace;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_txTraceWithAddresses;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &,
                 const SeqTsSizeHeader &>
      m_txTraceWithSeqTsSize;

private:
  void ScheduleNextTx();
  void ScheduleStartEvent();
  void ScheduleStopEvent();
  void ConnectionSucceeded(Ptr<Socket> socket);
  void ConnectionFailed(Ptr<Socket> socket);
};

} // namespace ns3

#endif
