
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/test.h"

#include <iostream>

using namespace ns3;

class TcpEndPointBug2211Test : public TestCase {
public:
  TcpEndPointBug2211Test(std::string desc, bool ipVersion);

  void Recv(Ptr<Socket> socket);
  void HandleAccept(Ptr<Socket> s, const Address &from);
  void HandleConnect(Ptr<Socket> socket);
  void DoRun() override;

private:
  bool m_v6;
};

void TcpEndPointBug2211Test::Recv(Ptr<Socket> socket) {
  if (socket->GetRxAvailable() == 536 * 2) {
    socket->Close();
  }
}

void TcpEndPointBug2211Test::HandleAccept(Ptr<Socket> s, const Address &from) {
  s->SetRecvCallback(MakeCallback(&TcpEndPointBug2211Test::Recv, this));
}

void TcpEndPointBug2211Test::HandleConnect(Ptr<Socket> socket) {
  socket->Send(Create<Packet>(536));
  socket->Send(Create<Packet>(536));
  socket->Send(Create<Packet>(536));
  socket->Close();
}

TcpEndPointBug2211Test::TcpEndPointBug2211Test(std::string desc, bool ipVersion)
    : TestCase(desc) {
  m_v6 = ipVersion;
}

void TcpEndPointBug2211Test::DoRun() {
  Ptr<Node> node = CreateObject<Node>();

  InternetStackHelper internet;
  internet.Install(node);

  TypeId tid = TcpSocketFactory::GetTypeId();
  Ptr<Socket> sink = Socket::CreateSocket(node, tid);
  if (!m_v6) {
    sink->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9));
  } else {
    sink->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), 9));
  }
  sink->Listen();
  sink->SetAcceptCallback(
      MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
      MakeCallback(&TcpEndPointBug2211Test::HandleAccept, this));

  Ptr<Socket> source = Socket::CreateSocket(node, tid);
  source->Bind();
  source->SetConnectCallback(
      MakeCallback(&TcpEndPointBug2211Test::HandleConnect, this),
      MakeNullCallback<void, Ptr<Socket>>());
  if (!m_v6) {
    source->Connect(InetSocketAddress(Ipv4Address::GetLoopback(), 9));
  } else {
    source->Connect(Inet6SocketAddress(Ipv6Address::GetLoopback(), 9));
  }

  Simulator::Run();
  Simulator::Destroy();
}

class TcpEndpointBug2211TestSuite : public TestSuite {
public:
  TcpEndpointBug2211TestSuite() : TestSuite("tcp-endpoint-bug2211-test", UNIT) {
    AddTestCase(new TcpEndPointBug2211Test("Bug 2211 testcase IPv4", false),
                TestCase::QUICK);
    AddTestCase(new TcpEndPointBug2211Test("Bug 2211 testcase IPv6", true),
                TestCase::QUICK);
  }
};

static TcpEndpointBug2211TestSuite g_TcpEndPoint2211TestSuite;
