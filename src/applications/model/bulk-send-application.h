
#ifndef BULK_SEND_APPLICATION_H
#define BULK_SEND_APPLICATION_H

#include "seq-ts-size-header.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class Socket;

class BulkSendApplication : public Application {
public:
  static TypeId GetTypeId();

  BulkSendApplication();

  ~BulkSendApplication() override;

  void SetMaxBytes(uint64_t maxBytes);

  Ptr<Socket> GetSocket() const;

protected:
  void DoDispose() override;

private:
  void StartApplication() override;
  void StopApplication() override;

  void SendData(const Address &from, const Address &to);

  Ptr<Socket> m_socket;
  Address m_peer;
  Address m_local;
  bool m_connected;
  uint32_t m_sendSize;
  uint64_t m_maxBytes;
  uint64_t m_totBytes;
  TypeId m_tid;
  uint32_t m_seq{0};
  Ptr<Packet> m_unsentPacket;
  bool m_enableSeqTsSizeHeader{false};

  TracedCallback<Ptr<const Packet>> m_txTrace;

  TracedCallback<Ptr<const Packet>, const Address &, const Address &,
                 const SeqTsSizeHeader &>
      m_txTraceWithSeqTsSize;

private:
  void ConnectionSucceeded(Ptr<Socket> socket);
  void ConnectionFailed(Ptr<Socket> socket);
  void DataSend(Ptr<Socket> socket, uint32_t unused);
};

} // namespace ns3

#endif
