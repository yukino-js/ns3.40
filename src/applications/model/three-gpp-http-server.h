
#ifndef THREE_GPP_HTTP_SERVER_H
#define THREE_GPP_HTTP_SERVER_H

#include "three-gpp-http-header.h"

#include <ns3/address.h>
#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>
#include <ns3/traced-callback.h>

#include <map>
#include <ostream>

namespace ns3 {

class Socket;
class Packet;
class ThreeGppHttpVariables;
class ThreeGppHttpServerTxBuffer;

class ThreeGppHttpServer : public Application {
public:
  ThreeGppHttpServer();

  static TypeId GetTypeId();

  void SetMtuSize(uint32_t mtuSize);

  Ptr<Socket> GetSocket() const;

  enum State_t { NOT_STARTED = 0, STARTED, STOPPED };

  State_t GetState() const;

  std::string GetStateString() const;

  static std::string GetStateString(State_t state);

  typedef void (*ThreeGppHttpObjectCallback)(uint32_t size);

  typedef void (*ConnectionEstablishedCallback)(
      Ptr<const ThreeGppHttpServer> httpServer, Ptr<Socket> socket);

protected:
  void DoDispose() override;

  void StartApplication() override;
  void StopApplication() override;

private:
  bool ConnectionRequestCallback(Ptr<Socket> socket, const Address &address);
  void NewConnectionCreatedCallback(Ptr<Socket> socket, const Address &address);
  void NormalCloseCallback(Ptr<Socket> socket);
  void ErrorCloseCallback(Ptr<Socket> socket);
  void ReceivedDataCallback(Ptr<Socket> socket);
  void SendCallback(Ptr<Socket> socket, uint32_t availableBufferSize);

  void ServeNewMainObject(Ptr<Socket> socket);
  void ServeNewEmbeddedObject(Ptr<Socket> socket);
  uint32_t ServeFromTxBuffer(Ptr<Socket> socket);

  void SwitchToState(State_t state);

  State_t m_state;
  Ptr<Socket> m_initialSocket;
  Ptr<ThreeGppHttpServerTxBuffer> m_txBuffer;

  Ptr<ThreeGppHttpVariables> m_httpVariables;
  Address m_localAddress;
  uint16_t m_localPort;
  uint32_t m_mtuSize;

  TracedCallback<Ptr<const ThreeGppHttpServer>, Ptr<Socket>>
      m_connectionEstablishedTrace;
  TracedCallback<uint32_t> m_mainObjectTrace;
  TracedCallback<uint32_t> m_embeddedObjectTrace;
  TracedCallback<Ptr<const Packet>> m_txTrace;
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
  TracedCallback<const Time &, const Address &> m_rxDelayTrace;
  TracedCallback<const std::string &, const std::string &>
      m_stateTransitionTrace;
};

class ThreeGppHttpServerTxBuffer
    : public SimpleRefCount<ThreeGppHttpServerTxBuffer> {
public:
  ThreeGppHttpServerTxBuffer();

  bool IsSocketAvailable(Ptr<Socket> socket) const;

  void AddSocket(Ptr<Socket> socket);

  void RemoveSocket(Ptr<Socket> socket);

  void CloseSocket(Ptr<Socket> socket);

  void CloseAllSockets();

  bool IsBufferEmpty(Ptr<Socket> socket) const;

  Time GetClientTs(Ptr<Socket> socket) const;

  ThreeGppHttpHeader::ContentType_t
  GetBufferContentType(Ptr<Socket> socket) const;

  uint32_t GetBufferSize(Ptr<Socket> socket) const;

  bool HasTxedPartOfObject(Ptr<Socket> socket) const;

  void WriteNewObject(Ptr<Socket> socket,
                      ThreeGppHttpHeader::ContentType_t contentType,
                      uint32_t objectSize);

  void RecordNextServe(Ptr<Socket> socket, const EventId &eventId,
                       const Time &clientTs);

  void DepleteBufferSize(Ptr<Socket> socket, uint32_t amount);

  void PrepareClose(Ptr<Socket> socket);

private:
  struct TxBuffer_t {
    EventId nextServe;
    Time clientTs;
    ThreeGppHttpHeader::ContentType_t txBufferContentType;
    uint32_t txBufferSize;
    bool isClosing;
    bool hasTxedPartOfObject;
  };

  std::map<Ptr<Socket>, TxBuffer_t> m_txBuffer;
};

} // namespace ns3

#endif
