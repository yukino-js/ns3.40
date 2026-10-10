
#include "ns3tcp-socket-writer.h"

#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/data-rate.h"
#include "ns3/error-model.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/pcap-file.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/test.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/uinteger.h"

#include <iomanip>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Ns3TcpStateTest");

const bool WRITE_VECTORS = false;
const bool WRITE_PCAP = false;
const bool WRITE_LOGGING = false;
const uint32_t PCAP_LINK_TYPE = 1187373554;
const uint32_t PCAP_SNAPLEN = 64;

class Ns3TcpStateTestCase : public TestCase {
public:
  Ns3TcpStateTestCase();
  Ns3TcpStateTestCase(uint32_t testCase);

  ~Ns3TcpStateTestCase() override {}

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;

  std::string m_pcapFilename;
  PcapFile m_pcapFile;
  uint32_t m_testCase;
  uint32_t m_totalTxBytes;
  uint32_t m_currentTxBytes;
  bool m_writeVectors;
  bool m_writeResults;
  bool m_writeLogging;
  bool m_needToClose;

  void Ipv4L3Tx(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ipv4,
                uint32_t interface);
  void Ipv4L3Rx(std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ipv4,
                uint32_t interface);
  void WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace);
  void StartFlow(Ptr<Socket> localSocket, Ipv4Address servAddress,
                 uint16_t servPort);
};

Ns3TcpStateTestCase::Ns3TcpStateTestCase()
    : TestCase(
          "Check the operation of the TCP state machine for several cases"),
      m_testCase(0), m_totalTxBytes(20000), m_currentTxBytes(0),
      m_writeVectors(WRITE_VECTORS), m_writeResults(WRITE_PCAP),
      m_writeLogging(WRITE_LOGGING), m_needToClose(true) {}

Ns3TcpStateTestCase::Ns3TcpStateTestCase(uint32_t testCase)
    : TestCase(
          "Check the operation of the TCP state machine for several cases"),
      m_testCase(testCase), m_totalTxBytes(20000), m_currentTxBytes(0),
      m_writeVectors(WRITE_VECTORS), m_writeResults(WRITE_PCAP),
      m_writeLogging(WRITE_LOGGING), m_needToClose(true) {}

void Ns3TcpStateTestCase::DoSetup() {
  std::ostringstream oss;
  oss << "ns3tcp-state" << m_testCase << "-response-vectors.pcap";
  m_pcapFilename = CreateDataDirFilename(oss.str());
  NS_LOG_INFO("m_pcapFilename=" << m_pcapFilename);

  if (m_writeVectors) {
    m_pcapFile.Open(m_pcapFilename, std::ios::out | std::ios::binary);
    m_pcapFile.Init(PCAP_LINK_TYPE, PCAP_SNAPLEN);
  } else {
    m_pcapFile.Open(m_pcapFilename, std::ios::in | std::ios::binary);
    NS_ABORT_MSG_UNLESS(m_pcapFile.GetDataLinkType() == PCAP_LINK_TYPE,
                        "Wrong response vectors in directory: opening "
                            << m_pcapFilename);
  }
}

void Ns3TcpStateTestCase::DoTeardown() { m_pcapFile.Close(); }

void Ns3TcpStateTestCase::Ipv4L3Rx(std::string, Ptr<const Packet> packet,
                                   Ptr<Ipv4>, uint32_t) {
  Ptr<Packet> received = packet->Copy();
  Ipv4Header ipHeader;
  TcpHeader tcpHeader;
  received->RemoveHeader(ipHeader);
  received->RemoveHeader(tcpHeader);

  NS_LOG_DEBUG("Received: " << tcpHeader);
}

void Ns3TcpStateTestCase::Ipv4L3Tx(std::string, Ptr<const Packet> packet,
                                   Ptr<Ipv4>, uint32_t) {
  Ptr<Packet> received = packet->Copy();
  Ipv4Header ipHeader;
  received->RemoveHeader(ipHeader);

  if (m_writeVectors) {
    Time tNow = Simulator::Now();
    int64_t tMicroSeconds = tNow.GetMicroSeconds();

    m_pcapFile.Write(uint32_t(tMicroSeconds / 1000000),
                     uint32_t(tMicroSeconds % 1000000), received);
  } else {
    uint8_t expectedBuffer[PCAP_SNAPLEN];
    uint32_t tsSec;
    uint32_t tsUsec;
    uint32_t inclLen;
    uint32_t origLen;
    uint32_t readLen;
    m_pcapFile.Read(expectedBuffer, sizeof(expectedBuffer), tsSec, tsUsec,
                    inclLen, origLen, readLen);

    NS_LOG_INFO("read " << readLen << " bytes");

    auto actual = new uint8_t[readLen];
    received->CopyData(actual, readLen);

    int result = memcmp(actual, expectedBuffer, readLen);

    TcpHeader expectedHeader;
    TcpHeader receivedHeader;
    Ptr<Packet> expected = Create<Packet>(expectedBuffer, readLen);

    expected->RemoveHeader(expectedHeader);
    received->RemoveHeader(receivedHeader);

    NS_LOG_DEBUG("Expected " << expectedHeader
                             << " received: " << receivedHeader);

    delete[] actual;

    if (IsStatusSuccess()) {
      NS_TEST_EXPECT_MSG_EQ(result, 0, "Expected data comparison error");
    }
  }
}

void Ns3TcpStateTestCase::WriteUntilBufferFull(Ptr<Socket> localSocket,
                                               uint32_t txSpace) {
  while (m_currentTxBytes < m_totalTxBytes) {
    uint32_t left = m_totalTxBytes - m_currentTxBytes;
    uint32_t dataOffset = m_currentTxBytes % 1040;
    uint32_t toWrite = 1040 - dataOffset;
    uint32_t txAvail = localSocket->GetTxAvailable();
    toWrite = std::min(toWrite, left);
    toWrite = std::min(toWrite, txAvail);
    if (txAvail == 0) {
      return;
    }
    if (m_writeLogging) {
      std::clog << "Submitting " << toWrite << " bytes to TCP socket"
                << std::endl;
    }
    int amountSent = localSocket->Send(nullptr, toWrite, 0);
    NS_ASSERT(amountSent > 0);
    m_currentTxBytes += amountSent;
  }
  if (m_needToClose) {
    if (m_writeLogging) {
      std::clog << "Close socket at " << Simulator::Now().GetSeconds()
                << std::endl;
    }
    localSocket->Close();
    m_needToClose = false;
  }
}

void Ns3TcpStateTestCase::StartFlow(Ptr<Socket> localSocket,
                                    Ipv4Address servAddress,
                                    uint16_t servPort) {
  if (m_writeLogging) {
    std::clog << "Starting flow at time " << Simulator::Now().GetSeconds()
              << std::endl;
  }

  localSocket->Connect(InetSocketAddress(servAddress, servPort));

  localSocket->SetSendCallback(
      MakeCallback(&Ns3TcpStateTestCase::WriteUntilBufferFull, this));
  WriteUntilBufferFull(localSocket, localSocket->GetTxAvailable());
}

void Ns3TcpStateTestCase::DoRun() {

  std::string tcpModel("ns3::TcpNewReno");

  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(false));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(tcpModel));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1000));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", StringValue("20p"));
  Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue(false));

  if (m_writeLogging) {
    LogComponentEnableAll(LOG_PREFIX_FUNC);
    LogComponentEnable("ErrorModel", LOG_LEVEL_DEBUG);
    LogComponentEnable("Ns3TcpStateTest", LOG_LEVEL_DEBUG);
    LogComponentEnable("TcpCongestionOps", LOG_LEVEL_INFO);
    LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);
  }

  NodeContainer n0n1;
  n0n1.Create(2);

  NodeContainer n1n2;
  n1n2.Add(n0n1.Get(1));
  n1n2.Create(1);

  InternetStackHelper internet;
  internet.InstallAll();

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(1000000)));
  p2p.SetChannelAttribute("Delay", TimeValue(Seconds(0.0001)));
  NetDeviceContainer dev0 = p2p.Install(n0n1);
  NetDeviceContainer dev1 = p2p.Install(n1n2);

  TrafficControlHelper tch;
  tch.SetRootQueueDisc("ns3::PfifoFastQueueDisc");
  tch.Install(dev0);
  tch.Install(dev1);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  ipv4.Assign(dev0);
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer ipInterfs = ipv4.Assign(dev1);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  uint16_t servPort = 50000;
  PacketSinkHelper sink("ns3::TcpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), servPort));
  ApplicationContainer sinkApps = sink.Install(n1n2.Get(1));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(100.0));

  Ptr<Socket> localSocket =
      Socket::CreateSocket(n0n1.Get(0), TcpSocketFactory::GetTypeId());
  localSocket->Bind();
  Simulator::ScheduleNow(&Ns3TcpStateTestCase::StartFlow, this, localSocket,
                         ipInterfs.GetAddress(1), servPort);

  Config::Connect("/NodeList/0/$ns3::Ipv4L3Protocol/Tx",
                  MakeCallback(&Ns3TcpStateTestCase::Ipv4L3Tx, this));

  Config::Connect("/NodeList/0/$ns3::Ipv4L3Protocol/Rx",
                  MakeCallback(&Ns3TcpStateTestCase::Ipv4L3Rx, this));

  std::list<uint32_t> dropListN0;
  std::list<uint32_t> dropListN1;
  std::string caseDescription;
  switch (m_testCase) {
  case 0:
    m_totalTxBytes = 1000;
    caseDescription = "Verify connection establishment";
    break;
  case 1:
    m_totalTxBytes = 100 * 1000;
    caseDescription =
        "Verify a bigger (100 pkts) transfer: Sliding window operation, etc.";
    break;
  case 2:
    m_totalTxBytes = 1000;
    caseDescription = "Survive a SYN lost";
    dropListN0.push_back(0);
    break;
  case 3:
    m_totalTxBytes = 2000;
    caseDescription = "Survive a SYN+ACK lost";
    dropListN1.push_back(0);
    break;
  case 4:
    m_totalTxBytes = 2000;
    caseDescription = "Survive a ACK (last packet in 3-way handshake) lost";
    dropListN0.push_back(1);
    break;
  case 5:
    m_totalTxBytes = 0;
    caseDescription = "Immediate FIN upon SYN_RCVD";
    m_needToClose = false;
    dropListN0.push_back(1);
    Simulator::Schedule(Seconds(0.002), &Socket::Close, localSocket);
    break;
  case 6:
    m_totalTxBytes = 5000;
    caseDescription = "Simulated simultaneous close";
    dropListN1.push_back(5);
    break;
  case 7:
    m_totalTxBytes = 5000;
    caseDescription =
        "FIN check 1: Loss of initiator's FIN. Wait until app close";
    m_needToClose = false;
    dropListN0.push_back(7);
    Simulator::Schedule(Seconds(0.04), &Socket::Close, localSocket);
    break;
  case 8:
    m_totalTxBytes = 5000;
    caseDescription = "FIN check 2: Loss responder's FIN. FIN will be resent "
                      "after last ack timeout";
    dropListN1.push_back(6);
    break;
  default:
    NS_FATAL_ERROR("Program fatal error: specified test case not supported: "
                   << m_testCase);
    break;
  }

  Ptr<ReceiveListErrorModel> errN0 = CreateObject<ReceiveListErrorModel>();
  errN0->SetList(dropListN0);
  dev0.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errN0));

  Ptr<ReceiveListErrorModel> errN1 = CreateObject<ReceiveListErrorModel>();
  errN1->SetList(dropListN1);
  dev1.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(errN1));

  std::ostringstream oss;
  oss << "tcp-state" << m_testCase << "-test-case";
  if (m_writeResults) {
    p2p.EnablePcapAll(oss.str());
    p2p.EnableAsciiAll(oss.str());
  }

  if (m_writeLogging) {
    Ptr<OutputStreamWrapper> osw = Create<OutputStreamWrapper>(&std::clog);
    *(osw->GetStream()) << std::setprecision(9) << std::fixed;
    p2p.EnableAsciiAll(osw);

    std::clog << std::endl
              << "Running TCP test-case " << m_testCase << ": "
              << caseDescription << std::endl;
  }

  Simulator::Stop(Seconds(1000));
  Simulator::Run();
  Simulator::Destroy();
}

class Ns3TcpStateTestSuite : public TestSuite {
public:
  Ns3TcpStateTestSuite();
};

Ns3TcpStateTestSuite::Ns3TcpStateTestSuite()
    : TestSuite("ns3-tcp-state", SYSTEM) {
  SetDataDir("src/test/ns3tcp/response-vectors");
  Packet::EnablePrinting();

  AddTestCase(new Ns3TcpStateTestCase(0), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(1), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(2), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(3), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(4), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(5), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(6), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(7), TestCase::QUICK);
  AddTestCase(new Ns3TcpStateTestCase(8), TestCase::QUICK);
}

static Ns3TcpStateTestSuite g_ns3TcpLossTestSuite;
