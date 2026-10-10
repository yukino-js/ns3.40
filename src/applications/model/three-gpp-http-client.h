
#ifndef THREE_GPP_HTTP_CLIENT_H
#define THREE_GPP_HTTP_CLIENT_H

#include "three-gpp-http-header.h"

#include <ns3/address.h>
#include <ns3/application.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class Socket;
class Packet;
class ThreeGppHttpVariables;

class ThreeGppHttpClient : public Application {
public:
  ThreeGppHttpClient();

  static TypeId GetTypeId();

  Ptr<Socket> GetSocket() const;

  enum State_t {
    NOT_STARTED = 0,
    CONNECTING,
    EXPECTING_MAIN_OBJECT,
    PARSING_MAIN_OBJECT,
    EXPECTING_EMBEDDED_OBJECT,
    READING,
    STOPPED
  };

  State_t GetState() const;

  std::string GetStateString() const;

  static std::string GetStateString(State_t state);

  typedef void (*TracedCallback)(Ptr<const ThreeGppHttpClient> httpClient);

  typedef void (*RxPageTracedCallback)(Ptr<const ThreeGppHttpClient> httpClient,
                                       const Time &time, uint32_t numObjects,
                                       uint32_t numBytes);

protected:
  void DoDispose() override;

  void StartApplication() override;
  void StopApplication() override;

private:
  void ConnectionSucceededCallback(Ptr<Socket> socket);
  void ConnectionFailedCallback(Ptr<Socket> socket);
  void NormalCloseCallback(Ptr<Socket> socket);
  void ErrorCloseCallback(Ptr<Socket> socket);
  void ReceivedDataCallback(Ptr<Socket> socket);

  void OpenConnection();

  void RequestMainObject();
  void RequestEmbeddedObject();

  void ReceiveMainObject(Ptr<Packet> packet, const Address &from);
  void ReceiveEmbeddedObject(Ptr<Packet> packet, const Address &from);
  void Receive(Ptr<Packet> packet);

  void EnterParsingTime();
  void ParseMainObject();
  void EnterReadingTime();
  void CancelAllPendingEvents();

  void SwitchToState(State_t state);

  void FinishReceivingPage();

  State_t m_state;
  Ptr<Socket> m_socket;
  uint32_t m_objectBytesToBeReceived;
  Ptr<Packet> m_constructedPacket;
  Time m_objectClientTs;
  Time m_objectServerTs;
  uint32_t m_embeddedObjectsToBeRequested;
  Time m_pageLoadStartTs;
  uint32_t m_numberEmbeddedObjectsRequested;
  uint32_t m_numberBytesPage;

  Ptr<ThreeGppHttpVariables> m_httpVariables;
  Address m_remoteServerAddress;
  uint16_t m_remoteServerPort;

  ns3::TracedCallback<Ptr<const ThreeGppHttpClient>, const Time &, uint32_t,
                      uint32_t>
      m_rxPageTrace;
  ns3::TracedCallback<Ptr<const ThreeGppHttpClient>>
      m_connectionEstablishedTrace;
  ns3::TracedCallback<Ptr<const ThreeGppHttpClient>> m_connectionClosedTrace;
  ns3::TracedCallback<Ptr<const Packet>> m_txTrace;
  ns3::TracedCallback<Ptr<const Packet>> m_txMainObjectRequestTrace;
  ns3::TracedCallback<Ptr<const Packet>> m_txEmbeddedObjectRequestTrace;
  ns3::TracedCallback<Ptr<const Packet>> m_rxMainObjectPacketTrace;
  ns3::TracedCallback<Ptr<const ThreeGppHttpClient>, Ptr<const Packet>>
      m_rxMainObjectTrace;
  ns3::TracedCallback<Ptr<const Packet>> m_rxEmbeddedObjectPacketTrace;
  ns3::TracedCallback<Ptr<const ThreeGppHttpClient>, Ptr<const Packet>>
      m_rxEmbeddedObjectTrace;
  ns3::TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
  ns3::TracedCallback<const Time &, const Address &> m_rxDelayTrace;
  ns3::TracedCallback<const Time &, const Address &> m_rxRttTrace;
  ns3::TracedCallback<const std::string &, const std::string &>
      m_stateTransitionTrace;

  EventId m_eventRequestMainObject;
  EventId m_eventRequestEmbeddedObject;
  EventId m_eventParseMainObject;
};

} // namespace ns3

#endif
