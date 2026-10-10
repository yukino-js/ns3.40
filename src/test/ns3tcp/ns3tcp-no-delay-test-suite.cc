

#include "ns3tcp-socket-writer.h"

#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/csma-helper.h"
#include "ns3/data-rate.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/pcap-file.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Ns3TcpNoDelayTest");

class Ns3TcpNoDelayTestCase : public TestCase {
public:
  Ns3TcpNoDelayTestCase(bool noDelay);

  ~Ns3TcpNoDelayTestCase() override {}

private:
  void DoRun() override;
  bool m_noDelay;
  bool m_writeResults;

  void SinkRx(std::string path, Ptr<const Packet> p, const Address &address);

  TestVectors<uint32_t> m_inputs;
  TestVectors<uint32_t> m_responses;
};

Ns3TcpNoDelayTestCase::Ns3TcpNoDelayTestCase(bool noDelay)
    : TestCase("Check that ns-3 TCP Nagle's algorithm works correctly and that "
               "we can turn it off."),
      m_noDelay(noDelay), m_writeResults(false) {}

void Ns3TcpNoDelayTestCase::SinkRx(std::string, Ptr<const Packet> p,
                                   const Address &) {
  m_responses.Add(p->GetSize());
}

void Ns3TcpNoDelayTestCase::DoRun() {
  uint16_t sinkPort = 50000;
  double sinkStopTime = 8;
  double writerStopTime = 5;
  double simStopTime = 10;
  Time sinkStopTimeObj = Seconds(sinkStopTime);
  Time writerStopTimeObj = Seconds(writerStopTime);
  Time simStopTimeObj = Seconds(simStopTime);

  Ptr<Node> n0 = CreateObject<Node>();
  Ptr<Node> n1 = CreateObject<Node>();

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install(n0, n1);

  InternetStackHelper internet;
  internet.InstallAll();

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer ifContainer = address.Assign(devices);

  Ptr<SocketWriter> socketWriter = CreateObject<SocketWriter>();
  Address sinkAddress(InetSocketAddress(ifContainer.GetAddress(1), sinkPort));
  socketWriter->Setup(n0, sinkAddress);
  n0->AddApplication(socketWriter);
  socketWriter->SetStartTime(Seconds(0.));
  socketWriter->SetStopTime(writerStopTimeObj);

  PacketSinkHelper sink("ns3::TcpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
  ApplicationContainer apps = sink.Install(n1);
  apps.Start(Seconds(0.0));
  apps.Stop(sinkStopTimeObj);

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                  MakeCallback(&Ns3TcpNoDelayTestCase::SinkRx, this));

  Config::SetDefault("ns3::TcpSocket::TcpNoDelay", BooleanValue(m_noDelay));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));

  Simulator::Schedule(Seconds(1), &SocketWriter::Connect, socketWriter);

  Simulator::Schedule(Seconds(2), &SocketWriter::Write, socketWriter, 2680);
  m_inputs.Add(536);
  m_inputs.Add(536);
  m_inputs.Add(536);
  m_inputs.Add(536);
  m_inputs.Add(536);

  Simulator::Schedule(Seconds(2.010), &SocketWriter::Write, socketWriter, 1);

  if (m_noDelay) {
    m_inputs.Add(1);
  }

  Simulator::Schedule(Seconds(2.012), &SocketWriter::Write, socketWriter, 535);

  if (m_noDelay) {
    m_inputs.Add(535);
  } else {
    m_inputs.Add(536);
  }

  Simulator::Schedule(writerStopTimeObj, &SocketWriter::Close, socketWriter);

  if (m_writeResults) {
    std::ostringstream oss;
    if (m_noDelay) {
      oss << "tcp-no-delay-on-test-case";
      pointToPoint.EnablePcapAll(oss.str());
    } else {
      oss << "tcp-no-delay-off-test-case";
      pointToPoint.EnablePcapAll(oss.str());
    }
  }

  Simulator::Stop(simStopTimeObj);
  Simulator::Run();
  Simulator::Destroy();

  NS_TEST_ASSERT_MSG_EQ(m_inputs.GetN(), m_responses.GetN(),
                        "Incorrect number of expected receive events");
  for (uint32_t i = 0; i < m_responses.GetN(); i++) {
    uint32_t in = m_inputs.Get(i);
    uint32_t out = m_responses.Get(i);
    NS_TEST_ASSERT_MSG_EQ(in, out,
                          "Mismatch:  expected " << in << " bytes, got " << out
                                                 << " bytes");
  }
}

class Ns3TcpNoDelayTestSuite : public TestSuite {
public:
  Ns3TcpNoDelayTestSuite();
};

Ns3TcpNoDelayTestSuite::Ns3TcpNoDelayTestSuite()
    : TestSuite("ns3-tcp-no-delay", SYSTEM) {
  AddTestCase(new Ns3TcpNoDelayTestCase(true), TestCase::QUICK);
  AddTestCase(new Ns3TcpNoDelayTestCase(false), TestCase::QUICK);
}

static Ns3TcpNoDelayTestSuite g_ns3TcpNoDelayTestSuite;
