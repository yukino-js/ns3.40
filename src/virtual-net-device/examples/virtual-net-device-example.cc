

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/virtual-net-device.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VirtualNetDeviceExample");

class Tunnel {
  Ptr<Socket> m_n3Socket;
  Ptr<Socket> m_n0Socket;
  Ptr<Socket> m_n1Socket;
  Ipv4Address m_n3Address;
  Ipv4Address m_n0Address;
  Ipv4Address m_n1Address;
  Ptr<UniformRandomVariable> m_rng;
  Ptr<VirtualNetDevice> m_n0Tap;
  Ptr<VirtualNetDevice> m_n1Tap;
  Ptr<VirtualNetDevice> m_n3Tap;

  bool N0VirtualSend(Ptr<Packet> packet, const Address &source,
                     const Address &dest, uint16_t protocolNumber) {
    NS_LOG_DEBUG("Send to " << m_n3Address << ": " << *packet);
    m_n0Socket->SendTo(packet, 0, InetSocketAddress(m_n3Address, 667));
    return true;
  }

  bool N1VirtualSend(Ptr<Packet> packet, const Address &source,
                     const Address &dest, uint16_t protocolNumber) {
    NS_LOG_DEBUG("Send to " << m_n3Address << ": " << *packet);
    m_n1Socket->SendTo(packet, 0, InetSocketAddress(m_n3Address, 667));
    return true;
  }

  bool N3VirtualSend(Ptr<Packet> packet, const Address &source,
                     const Address &dest, uint16_t protocolNumber) {
    if (m_rng->GetValue() < 0.25) {
      NS_LOG_DEBUG("Send to " << m_n0Address << ": " << *packet);
      m_n3Socket->SendTo(packet, 0, InetSocketAddress(m_n0Address, 667));
    } else {
      NS_LOG_DEBUG("Send to " << m_n1Address << ": " << *packet);
      m_n3Socket->SendTo(packet, 0, InetSocketAddress(m_n1Address, 667));
    }
    return true;
  }

  void N3SocketRecv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv(65535, 0);
    NS_LOG_DEBUG("N3SocketRecv: " << *packet);
    m_n3Tap->Receive(packet, 0x0800, m_n3Tap->GetAddress(),
                     m_n3Tap->GetAddress(), NetDevice::PACKET_HOST);
  }

  void N0SocketRecv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv(65535, 0);
    NS_LOG_DEBUG("N0SocketRecv: " << *packet);
    m_n0Tap->Receive(packet, 0x0800, m_n0Tap->GetAddress(),
                     m_n0Tap->GetAddress(), NetDevice::PACKET_HOST);
  }

  void N1SocketRecv(Ptr<Socket> socket) {
    Ptr<Packet> packet = socket->Recv(65535, 0);
    NS_LOG_DEBUG("N1SocketRecv: " << *packet);
    m_n1Tap->Receive(packet, 0x0800, m_n1Tap->GetAddress(),
                     m_n1Tap->GetAddress(), NetDevice::PACKET_HOST);
  }

public:
  Tunnel(Ptr<Node> n3, Ptr<Node> n0, Ptr<Node> n1, Ipv4Address n3Addr,
         Ipv4Address n0Addr, Ipv4Address n1Addr)
      : m_n3Address(n3Addr), m_n0Address(n0Addr), m_n1Address(n1Addr) {
    m_rng = CreateObject<UniformRandomVariable>();
    m_n3Socket =
        Socket::CreateSocket(n3, TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_n3Socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 667));
    m_n3Socket->SetRecvCallback(MakeCallback(&Tunnel::N3SocketRecv, this));

    m_n0Socket =
        Socket::CreateSocket(n0, TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_n0Socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 667));
    m_n0Socket->SetRecvCallback(MakeCallback(&Tunnel::N0SocketRecv, this));

    m_n1Socket =
        Socket::CreateSocket(n1, TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_n1Socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 667));
    m_n1Socket->SetRecvCallback(MakeCallback(&Tunnel::N1SocketRecv, this));

    m_n0Tap = CreateObject<VirtualNetDevice>();
    m_n0Tap->SetAddress(Mac48Address("11:00:01:02:03:01"));
    m_n0Tap->SetSendCallback(MakeCallback(&Tunnel::N0VirtualSend, this));
    n0->AddDevice(m_n0Tap);
    Ptr<Ipv4> ipv4 = n0->GetObject<Ipv4>();
    uint32_t i = ipv4->AddInterface(m_n0Tap);
    ipv4->AddAddress(i, Ipv4InterfaceAddress(Ipv4Address("11.0.0.1"),
                                             Ipv4Mask("255.255.255.0")));
    ipv4->SetUp(i);

    m_n1Tap = CreateObject<VirtualNetDevice>();
    m_n1Tap->SetAddress(Mac48Address("11:00:01:02:03:02"));
    m_n1Tap->SetSendCallback(MakeCallback(&Tunnel::N1VirtualSend, this));
    n1->AddDevice(m_n1Tap);
    ipv4 = n1->GetObject<Ipv4>();
    i = ipv4->AddInterface(m_n1Tap);
    ipv4->AddAddress(i, Ipv4InterfaceAddress(Ipv4Address("11.0.0.1"),
                                             Ipv4Mask("255.255.255.0")));
    ipv4->SetUp(i);

    m_n3Tap = CreateObject<VirtualNetDevice>();
    m_n3Tap->SetAddress(Mac48Address("11:00:01:02:03:04"));
    m_n3Tap->SetSendCallback(MakeCallback(&Tunnel::N3VirtualSend, this));
    n3->AddDevice(m_n3Tap);
    ipv4 = n3->GetObject<Ipv4>();
    i = ipv4->AddInterface(m_n3Tap);
    ipv4->AddAddress(i, Ipv4InterfaceAddress(Ipv4Address("11.0.0.254"),
                                             Ipv4Mask("255.255.255.0")));
    ipv4->SetUp(i);
  }
};

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("VirtualNetDeviceExample", LOG_LEVEL_INFO);
#endif
  Packet::EnablePrinting();

  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("448kb/s"));

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  c.Create(4);
  NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
  NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer n3n2 = NodeContainer(c.Get(3), c.Get(2));

  InternetStackHelper internet;
  internet.Install(c);

  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer d0d2 = p2p.Install(n0n2);

  NetDeviceContainer d1d2 = p2p.Install(n1n2);

  p2p.SetDeviceAttribute("DataRate", StringValue("1500kbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));
  NetDeviceContainer d3d2 = p2p.Install(n3n2);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = ipv4.Assign(d0d2);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i2 = ipv4.Assign(d3d2);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Tunnel tunnel(c.Get(3), c.Get(0), c.Get(1), i3i2.GetAddress(0),
                i0i2.GetAddress(0), i1i2.GetAddress(0));

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;
  OnOffHelper onoff(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address("11.0.0.254"), port)));
  onoff.SetConstantRate(DataRate("448kb/s"));
  ApplicationContainer apps = onoff.Install(c.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sink.Install(c.Get(3));
  apps.Start(Seconds(1.0));

  onoff.SetAttribute(
      "Remote", AddressValue(InetSocketAddress(Ipv4Address("11.0.0.1"), port)));
  apps = onoff.Install(c.Get(3));
  apps.Start(Seconds(1.1));
  apps.Stop(Seconds(10.0));

  apps = sink.Install(c.Get(1));
  apps.Start(Seconds(1.1));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("virtual-net-device.tr"));
  p2p.EnablePcapAll("virtual-net-device");

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
