
#include "three-gpp-http-client.h"

#include "three-gpp-http-variables.h"

#include <ns3/callback.h>
#include <ns3/double.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/uinteger.h>

NS_LOG_COMPONENT_DEFINE("ThreeGppHttpClient");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(ThreeGppHttpClient);

ThreeGppHttpClient::ThreeGppHttpClient()
    : m_state(NOT_STARTED), m_socket(nullptr), m_objectBytesToBeReceived(0),
      m_objectClientTs(MilliSeconds(0)), m_objectServerTs(MilliSeconds(0)),
      m_embeddedObjectsToBeRequested(0), m_pageLoadStartTs(MilliSeconds(0)),
      m_numberEmbeddedObjectsRequested(0), m_numberBytesPage(0),
      m_httpVariables(CreateObject<ThreeGppHttpVariables>()) {
  NS_LOG_FUNCTION(this);
}

TypeId ThreeGppHttpClient::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ThreeGppHttpClient")
          .SetParent<Application>()
          .AddConstructor<ThreeGppHttpClient>()
          .AddAttribute(
              "Variables",
              "Variable collection, which is used to control e.g. timing and "
              "HTTP request size.",
              PointerValue(),
              MakePointerAccessor(&ThreeGppHttpClient::m_httpVariables),
              MakePointerChecker<ThreeGppHttpVariables>())
          .AddAttribute(
              "RemoteServerAddress", "The address of the destination server.",
              AddressValue(),
              MakeAddressAccessor(&ThreeGppHttpClient::m_remoteServerAddress),
              MakeAddressChecker())
          .AddAttribute(
              "RemoteServerPort",
              "The destination port of the outbound packets.",
              UintegerValue(80),
              MakeUintegerAccessor(&ThreeGppHttpClient::m_remoteServerPort),
              MakeUintegerChecker<uint16_t>())
          .AddTraceSource(
              "RxPage", "A page has been received.",
              MakeTraceSourceAccessor(&ThreeGppHttpClient::m_rxPageTrace),
              "ns3::ThreeGppHttpClient::RxPageTracedCallback")
          .AddTraceSource(
              "ConnectionEstablished",
              "Connection to the destination web server has been established.",
              MakeTraceSourceAccessor(
                  &ThreeGppHttpClient::m_connectionEstablishedTrace),
              "ns3::ThreeGppHttpClient::TracedCallback")
          .AddTraceSource("ConnectionClosed",
                          "Connection to the destination web server is closed.",
                          MakeTraceSourceAccessor(
                              &ThreeGppHttpClient::m_connectionClosedTrace),
                          "ns3::ThreeGppHttpClient::TracedCallback")
          .AddTraceSource(
              "Tx", "General trace for sending a packet of any kind.",
              MakeTraceSourceAccessor(&ThreeGppHttpClient::m_txTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource("TxMainObjectRequest",
                          "Sent a request for a main object.",
                          MakeTraceSourceAccessor(
                              &ThreeGppHttpClient::m_txMainObjectRequestTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "TxEmbeddedObjectRequest",
              "Sent a request for an embedded object.",
              MakeTraceSourceAccessor(
                  &ThreeGppHttpClient::m_txEmbeddedObjectRequestTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource("RxMainObjectPacket",
                          "A packet of main object has been received.",
                          MakeTraceSourceAccessor(
                              &ThreeGppHttpClient::m_rxMainObjectPacketTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "RxMainObject",
              "Received a whole main object. Header is included.",
              MakeTraceSourceAccessor(&ThreeGppHttpClient::m_rxMainObjectTrace),
              "ns3::ThreeGppHttpClient::TracedCallback")
          .AddTraceSource(
              "RxEmbeddedObjectPacket",
              "A packet of embedded object has been received.",
              MakeTraceSourceAccessor(
                  &ThreeGppHttpClient::m_rxEmbeddedObjectPacketTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "RxEmbeddedObject",
              "Received a whole embedded object. Header is included.",
              MakeTraceSourceAccessor(
                  &ThreeGppHttpClient::m_rxEmbeddedObjectTrace),
              "ns3::ThreeGppHttpClient::TracedCallback")
          .AddTraceSource(
              "Rx", "General trace for receiving a packet of any kind.",
              MakeTraceSourceAccessor(&ThreeGppHttpClient::m_rxTrace),
              "ns3::Packet::PacketAddressTracedCallback")
          .AddTraceSource(
              "RxDelay",
              "General trace of delay for receiving a complete object.",
              MakeTraceSourceAccessor(&ThreeGppHttpClient::m_rxDelayTrace),
              "ns3::Application::DelayAddressCallback")
          .AddTraceSource(
              "RxRtt",
              "General trace of round trip delay time for receiving a complete "
              "object.",
              MakeTraceSourceAccessor(&ThreeGppHttpClient::m_rxRttTrace),
              "ns3::Application::DelayAddressCallback")
          .AddTraceSource(
              "StateTransition",
              "Trace fired upon every HTTP client state transition.",
              MakeTraceSourceAccessor(
                  &ThreeGppHttpClient::m_stateTransitionTrace),
              "ns3::Application::StateTransitionCallback");
  return tid;
}

Ptr<Socket> ThreeGppHttpClient::GetSocket() const { return m_socket; }

ThreeGppHttpClient::State_t ThreeGppHttpClient::GetState() const {
  return m_state;
}

std::string ThreeGppHttpClient::GetStateString() const {
  return GetStateString(m_state);
}

std::string
ThreeGppHttpClient::GetStateString(ThreeGppHttpClient::State_t state) {
  switch (state) {
  case NOT_STARTED:
    return "NOT_STARTED";
  case CONNECTING:
    return "CONNECTING";
  case EXPECTING_MAIN_OBJECT:
    return "EXPECTING_MAIN_OBJECT";
  case PARSING_MAIN_OBJECT:
    return "PARSING_MAIN_OBJECT";
  case EXPECTING_EMBEDDED_OBJECT:
    return "EXPECTING_EMBEDDED_OBJECT";
  case READING:
    return "READING";
  case STOPPED:
    return "STOPPED";
  default:
    NS_FATAL_ERROR("Unknown state");
    return "FATAL_ERROR";
  }
}

void ThreeGppHttpClient::DoDispose() {
  NS_LOG_FUNCTION(this);

  if (!Simulator::IsFinished()) {
    StopApplication();
  }

  Application::DoDispose();
}

void ThreeGppHttpClient::StartApplication() {
  NS_LOG_FUNCTION(this);

  if (m_state == NOT_STARTED) {
    m_httpVariables->Initialize();
    OpenConnection();
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for StartApplication().");
  }
}

void ThreeGppHttpClient::StopApplication() {
  NS_LOG_FUNCTION(this);

  SwitchToState(STOPPED);
  CancelAllPendingEvents();
  m_socket->Close();
  m_socket->SetConnectCallback(MakeNullCallback<void, Ptr<Socket>>(),
                               MakeNullCallback<void, Ptr<Socket>>());
  m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
}

void ThreeGppHttpClient::ConnectionSucceededCallback(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);

  if (m_state == CONNECTING) {
    NS_ASSERT_MSG(m_socket == socket, "Invalid socket.");
    m_connectionEstablishedTrace(this);
    socket->SetRecvCallback(
        MakeCallback(&ThreeGppHttpClient::ReceivedDataCallback, this));
    NS_ASSERT(m_embeddedObjectsToBeRequested == 0);
    m_eventRequestMainObject =
        Simulator::ScheduleNow(&ThreeGppHttpClient::RequestMainObject, this);
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for ConnectionSucceeded().");
  }
}

void ThreeGppHttpClient::ConnectionFailedCallback(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);

  if (m_state == CONNECTING) {
    NS_LOG_ERROR("Client failed to connect" << " to remote address "
                                            << m_remoteServerAddress << " port "
                                            << m_remoteServerPort << ".");
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for ConnectionFailed().");
  }
}

void ThreeGppHttpClient::NormalCloseCallback(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);

  CancelAllPendingEvents();

  if (socket->GetErrno() != Socket::ERROR_NOTERROR) {
    NS_LOG_ERROR(this << " Connection has been terminated,"
                      << " error code: " << socket->GetErrno() << ".");
  }

  m_socket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                              MakeNullCallback<void, Ptr<Socket>>());

  m_connectionClosedTrace(this);
}

void ThreeGppHttpClient::ErrorCloseCallback(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);

  CancelAllPendingEvents();
  if (socket->GetErrno() != Socket::ERROR_NOTERROR) {
    NS_LOG_ERROR(this << " Connection has been terminated,"
                      << " error code: " << socket->GetErrno() << ".");
  }

  m_connectionClosedTrace(this);
}

void ThreeGppHttpClient::ReceivedDataCallback(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom(from))) {
    if (packet->GetSize() == 0) {
      break;
    }

#ifdef NS3_LOG_ENABLE
    if (InetSocketAddress::IsMatchingType(from)) {
      NS_LOG_INFO(this << " A packet of " << packet->GetSize() << " bytes"
                       << " received from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4()
                       << " port "
                       << InetSocketAddress::ConvertFrom(from).GetPort()
                       << " / " << InetSocketAddress::ConvertFrom(from) << ".");
    } else if (Inet6SocketAddress::IsMatchingType(from)) {
      NS_LOG_INFO(
          this << " A packet of " << packet->GetSize() << " bytes"
               << " received from "
               << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
               << Inet6SocketAddress::ConvertFrom(from).GetPort() << " / "
               << Inet6SocketAddress::ConvertFrom(from) << ".");
    }
#endif

    m_rxTrace(packet, from);

    switch (m_state) {
    case EXPECTING_MAIN_OBJECT:
      ReceiveMainObject(packet, from);
      break;
    case EXPECTING_EMBEDDED_OBJECT:
      ReceiveEmbeddedObject(packet, from);
      break;
    default:
      NS_FATAL_ERROR("Invalid state " << GetStateString()
                                      << " for ReceivedData().");
      break;
    }
  }
}

void ThreeGppHttpClient::OpenConnection() {
  NS_LOG_FUNCTION(this);

  if (m_state == NOT_STARTED || m_state == EXPECTING_EMBEDDED_OBJECT ||
      m_state == PARSING_MAIN_OBJECT || m_state == READING) {
    m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

    if (Ipv4Address::IsMatchingType(m_remoteServerAddress)) {
      int ret [[maybe_unused]];

      ret = m_socket->Bind();
      NS_LOG_DEBUG(this << " Bind() return value= " << ret
                        << " GetErrNo= " << m_socket->GetErrno() << ".");

      Ipv4Address ipv4 = Ipv4Address::ConvertFrom(m_remoteServerAddress);
      InetSocketAddress inetSocket =
          InetSocketAddress(ipv4, m_remoteServerPort);
      NS_LOG_INFO(this << " Connecting to " << ipv4 << " port "
                       << m_remoteServerPort << " / " << inetSocket << ".");
      ret = m_socket->Connect(inetSocket);
      NS_LOG_DEBUG(this << " Connect() return value= " << ret
                        << " GetErrNo= " << m_socket->GetErrno() << ".");
    } else if (Ipv6Address::IsMatchingType(m_remoteServerAddress)) {
      int ret [[maybe_unused]];

      ret = m_socket->Bind6();
      NS_LOG_DEBUG(this << " Bind6() return value= " << ret
                        << " GetErrNo= " << m_socket->GetErrno() << ".");

      Ipv6Address ipv6 = Ipv6Address::ConvertFrom(m_remoteServerAddress);
      Inet6SocketAddress inet6Socket =
          Inet6SocketAddress(ipv6, m_remoteServerPort);
      NS_LOG_INFO(this << " connecting to " << ipv6 << " port "
                       << m_remoteServerPort << " / " << inet6Socket << ".");
      ret = m_socket->Connect(inet6Socket);
      NS_LOG_DEBUG(this << " Connect() return value= " << ret
                        << " GetErrNo= " << m_socket->GetErrno() << ".");
    }

    NS_ASSERT_MSG(m_socket, "Failed creating socket.");

    SwitchToState(CONNECTING);

    m_socket->SetConnectCallback(
        MakeCallback(&ThreeGppHttpClient::ConnectionSucceededCallback, this),
        MakeCallback(&ThreeGppHttpClient::ConnectionFailedCallback, this));
    m_socket->SetCloseCallbacks(
        MakeCallback(&ThreeGppHttpClient::NormalCloseCallback, this),
        MakeCallback(&ThreeGppHttpClient::ErrorCloseCallback, this));
    m_socket->SetRecvCallback(
        MakeCallback(&ThreeGppHttpClient::ReceivedDataCallback, this));
    m_socket->SetAttribute("MaxSegLifetime", DoubleValue(0.02));

  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for OpenConnection().");
  }
}

void ThreeGppHttpClient::RequestMainObject() {
  NS_LOG_FUNCTION(this);

  if (m_state == CONNECTING || m_state == READING) {
    ThreeGppHttpHeader header;
    header.SetContentLength(0);
    header.SetContentType(ThreeGppHttpHeader::MAIN_OBJECT);
    header.SetClientTs(Simulator::Now());

    const uint32_t requestSize = m_httpVariables->GetRequestSize();
    Ptr<Packet> packet = Create<Packet>(requestSize);
    packet->AddHeader(header);
    const uint32_t packetSize = packet->GetSize();
    m_txMainObjectRequestTrace(packet);
    m_txTrace(packet);
    const int actualBytes = m_socket->Send(packet);
    NS_LOG_DEBUG(this << " Send() packet " << packet << " of "
                      << packet->GetSize() << " bytes,"
                      << " return value= " << actualBytes << ".");
    if (actualBytes != static_cast<int>(packetSize)) {
      NS_LOG_ERROR(this << " Failed to send request for embedded object,"
                        << " GetErrNo= " << m_socket->GetErrno() << ","
                        << " waiting for another Tx opportunity.");
    } else {
      SwitchToState(EXPECTING_MAIN_OBJECT);
      m_pageLoadStartTs = Simulator::Now();
    }
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for RequestMainObject().");
  }
}

void ThreeGppHttpClient::RequestEmbeddedObject() {
  NS_LOG_FUNCTION(this);

  if (m_state == CONNECTING || m_state == PARSING_MAIN_OBJECT ||
      m_state == EXPECTING_EMBEDDED_OBJECT) {
    if (m_embeddedObjectsToBeRequested > 0) {
      ThreeGppHttpHeader header;
      header.SetContentLength(0);
      header.SetContentType(ThreeGppHttpHeader::EMBEDDED_OBJECT);
      header.SetClientTs(Simulator::Now());

      const uint32_t requestSize = m_httpVariables->GetRequestSize();
      Ptr<Packet> packet = Create<Packet>(requestSize);
      packet->AddHeader(header);
      const uint32_t packetSize = packet->GetSize();
      m_txEmbeddedObjectRequestTrace(packet);
      m_txTrace(packet);
      const int actualBytes = m_socket->Send(packet);
      NS_LOG_DEBUG(this << " Send() packet " << packet << " of "
                        << packet->GetSize() << " bytes,"
                        << " return value= " << actualBytes << ".");

      if (actualBytes != static_cast<int>(packetSize)) {
        NS_LOG_ERROR(this << " Failed to send request for embedded object,"
                          << " GetErrNo= " << m_socket->GetErrno() << ","
                          << " waiting for another Tx opportunity.");
      } else {
        m_embeddedObjectsToBeRequested--;
        SwitchToState(EXPECTING_EMBEDDED_OBJECT);
      }
    } else {
      NS_LOG_WARN(this << " No embedded object to be requested.");
    }
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for RequestEmbeddedObject().");
  }
}

void ThreeGppHttpClient::ReceiveMainObject(Ptr<Packet> packet,
                                           const Address &from) {
  NS_LOG_FUNCTION(this << packet << from);

  if (m_state == EXPECTING_MAIN_OBJECT) {
    Receive(packet);
    m_rxMainObjectPacketTrace(packet);

    if (m_objectBytesToBeReceived > 0) {
      NS_LOG_INFO(this << " " << m_objectBytesToBeReceived << " byte(s)"
                       << " remains from this chunk of main object.");
    } else {
      NS_LOG_INFO(this << " Finished receiving a main object.");
      m_rxMainObjectTrace(this, m_constructedPacket);

      if (!m_objectServerTs.IsZero()) {
        m_rxDelayTrace(Simulator::Now() - m_objectServerTs, from);
        m_objectServerTs = MilliSeconds(0);
      }

      if (!m_objectClientTs.IsZero()) {
        m_rxRttTrace(Simulator::Now() - m_objectClientTs, from);
        m_objectClientTs = MilliSeconds(0);
      }

      EnterParsingTime();
    }

  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for ReceiveMainObject().");
  }
}

void ThreeGppHttpClient::ReceiveEmbeddedObject(Ptr<Packet> packet,
                                               const Address &from) {
  NS_LOG_FUNCTION(this << packet << from);

  if (m_state == EXPECTING_EMBEDDED_OBJECT) {
    Receive(packet);
    m_rxEmbeddedObjectPacketTrace(packet);

    if (m_objectBytesToBeReceived > 0) {
      NS_LOG_INFO(this << " " << m_objectBytesToBeReceived << " byte(s)"
                       << " remains from this chunk of embedded object");
    } else {
      NS_LOG_INFO(this << " Finished receiving an embedded object.");
      m_rxEmbeddedObjectTrace(this, m_constructedPacket);

      if (!m_objectServerTs.IsZero()) {
        m_rxDelayTrace(Simulator::Now() - m_objectServerTs, from);
        m_objectServerTs = MilliSeconds(0);
      }

      if (!m_objectClientTs.IsZero()) {
        m_rxRttTrace(Simulator::Now() - m_objectClientTs, from);
        m_objectClientTs = MilliSeconds(0);
      }

      if (m_embeddedObjectsToBeRequested > 0) {
        NS_LOG_INFO(this << " " << m_embeddedObjectsToBeRequested
                         << " more embedded object(s) to be requested.");
        m_eventRequestEmbeddedObject = Simulator::ScheduleNow(
            &ThreeGppHttpClient::RequestEmbeddedObject, this);
      } else {
        NS_LOG_INFO(this << " Finished receiving a web page.");
        FinishReceivingPage();
        EnterReadingTime();
      }
    }

  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for ReceiveEmbeddedObject().");
  }
}

void ThreeGppHttpClient::Receive(Ptr<Packet> packet) {
  NS_LOG_FUNCTION(this << packet);

  bool firstPacket = false;

  if (m_objectBytesToBeReceived == 0) {
    firstPacket = true;

    ThreeGppHttpHeader httpHeader;
    packet->RemoveHeader(httpHeader);

    m_objectBytesToBeReceived = httpHeader.GetContentLength();
    m_objectClientTs = httpHeader.GetClientTs();
    m_objectServerTs = httpHeader.GetServerTs();

    m_constructedPacket = packet->Copy();
    m_constructedPacket->AddHeader(httpHeader);
  }
  uint32_t contentSize = packet->GetSize();
  m_numberBytesPage += contentSize;

  if (m_objectBytesToBeReceived < contentSize) {
    NS_LOG_WARN(this << " The received packet"
                     << " (" << contentSize << " bytes of content)"
                     << " is larger than"
                     << " the content that we expected to receive"
                     << " (" << m_objectBytesToBeReceived << " bytes).");
    m_objectBytesToBeReceived = 0;
    m_constructedPacket = nullptr;
  } else {
    m_objectBytesToBeReceived -= contentSize;
    if (!firstPacket) {
      Ptr<Packet> packetCopy = packet->Copy();
      m_constructedPacket->AddAtEnd(packetCopy);
    }
  }
}

void ThreeGppHttpClient::EnterParsingTime() {
  NS_LOG_FUNCTION(this);

  if (m_state == EXPECTING_MAIN_OBJECT) {
    const Time parsingTime = m_httpVariables->GetParsingTime();
    NS_LOG_INFO(this << " The parsing of this main object"
                     << " will complete in " << parsingTime.As(Time::S) << ".");
    m_eventParseMainObject = Simulator::Schedule(
        parsingTime, &ThreeGppHttpClient::ParseMainObject, this);
    SwitchToState(PARSING_MAIN_OBJECT);
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for EnterParsingTime().");
  }
}

void ThreeGppHttpClient::ParseMainObject() {
  NS_LOG_FUNCTION(this);

  if (m_state == PARSING_MAIN_OBJECT) {
    m_embeddedObjectsToBeRequested = m_httpVariables->GetNumOfEmbeddedObjects();
    m_numberEmbeddedObjectsRequested = m_embeddedObjectsToBeRequested;
    NS_LOG_INFO(this << " Parsing has determined "
                     << m_embeddedObjectsToBeRequested
                     << " embedded object(s) in the main object.");

    if (m_embeddedObjectsToBeRequested > 0) {
      m_eventRequestEmbeddedObject = Simulator::ScheduleNow(
          &ThreeGppHttpClient::RequestEmbeddedObject, this);
    } else {
      NS_LOG_INFO(this << " Finished receiving a web page.");
      FinishReceivingPage();
      EnterReadingTime();
    }
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for ParseMainObject().");
  }
}

void ThreeGppHttpClient::EnterReadingTime() {
  NS_LOG_FUNCTION(this);

  if (m_state == EXPECTING_EMBEDDED_OBJECT || m_state == PARSING_MAIN_OBJECT) {
    const Time readingTime = m_httpVariables->GetReadingTime();
    NS_LOG_INFO(this << " Client will finish reading this web page in "
                     << readingTime.As(Time::S) << ".");

    m_eventRequestMainObject = Simulator::Schedule(
        readingTime, &ThreeGppHttpClient::RequestMainObject, this);
    SwitchToState(READING);
  } else {
    NS_FATAL_ERROR("Invalid state " << GetStateString()
                                    << " for EnterReadingTime().");
  }
}

void ThreeGppHttpClient::CancelAllPendingEvents() {
  NS_LOG_FUNCTION(this);

  if (!Simulator::IsExpired(m_eventRequestMainObject)) {
    NS_LOG_INFO(
        this << " Canceling RequestMainObject() which is due in "
             << Simulator::GetDelayLeft(m_eventRequestMainObject).As(Time::S)
             << ".");
    Simulator::Cancel(m_eventRequestMainObject);
  }

  if (!Simulator::IsExpired(m_eventRequestEmbeddedObject)) {
    NS_LOG_INFO(this << " Canceling RequestEmbeddedObject() which is due in "
                     << Simulator::GetDelayLeft(m_eventRequestEmbeddedObject)
                            .As(Time::S)
                     << ".");
    Simulator::Cancel(m_eventRequestEmbeddedObject);
  }

  if (!Simulator::IsExpired(m_eventParseMainObject)) {
    NS_LOG_INFO(
        this << " Canceling ParseMainObject() which is due in "
             << Simulator::GetDelayLeft(m_eventParseMainObject).As(Time::S)
             << ".");
    Simulator::Cancel(m_eventParseMainObject);
  }
}

void ThreeGppHttpClient::SwitchToState(ThreeGppHttpClient::State_t state) {
  const std::string oldState = GetStateString();
  const std::string newState = GetStateString(state);
  NS_LOG_FUNCTION(this << oldState << newState);

  if ((state == EXPECTING_MAIN_OBJECT) ||
      (state == EXPECTING_EMBEDDED_OBJECT)) {
    if (m_objectBytesToBeReceived > 0) {
      NS_FATAL_ERROR("Cannot start a new receiving session"
                     << " if the previous object"
                     << " (" << m_objectBytesToBeReceived << " bytes)"
                     << " is not completely received yet.");
    }
  }

  m_state = state;
  NS_LOG_INFO(this << " HttpClient " << oldState << " --> " << newState << ".");
  m_stateTransitionTrace(oldState, newState);
}

void ThreeGppHttpClient::FinishReceivingPage() {
  m_rxPageTrace(this, Simulator::Now() - m_pageLoadStartTs,
                m_numberEmbeddedObjectsRequested, m_numberBytesPage);
  m_numberEmbeddedObjectsRequested = 0;
  m_numberBytesPage = 0;
}

} // namespace ns3
