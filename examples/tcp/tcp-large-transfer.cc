

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpLargeTransfer");

static const uint32_t totalTxBytes = 2000000;
static uint32_t currentTxBytes = 0;

static const uint32_t writeSize = 1040;
uint8_t data[writeSize];

void StartFlow(Ptr<Socket> localSocket, Ipv4Address servAddress,
               uint16_t servPort);

void WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace);

static void CwndTracer(uint32_t oldval, uint32_t newval) {
  NS_LOG_INFO("Moving cwnd from " << oldval << " to " << newval);
}

int main(int argc, char *argv[]) {

  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  for (uint32_t i = 0; i < writeSize; ++i) {
    char m = toascii(97 + i % 26);
    data[i] = m;
  }

  NodeContainer n0n1;
  n0n1.Create(2);

  NodeContainer n1n2;
  n1n2.Add(n0n1.Get(1));
  n1n2.Create(1);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(10000000)));
  p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

  NetDeviceContainer dev0 = p2p.Install(n0n1);
  NetDeviceContainer dev1 = p2p.Install(n1n2);

  InternetStackHelper internet;
  internet.InstallAll();

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  ipv4.Assign(dev0);
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer ipInterfs = ipv4.Assign(dev1);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  uint16_t servPort = 50000;

  PacketSinkHelper sink("ns3::TcpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), servPort));

  ApplicationContainer apps = sink.Install(n1n2.Get(1));
  apps.Start(Seconds(0.0));
  apps.Stop(Seconds(3.0));

  Ptr<Socket> localSocket =
      Socket::CreateSocket(n0n1.Get(0), TcpSocketFactory::GetTypeId());
  localSocket->Bind();

  Config::ConnectWithoutContext(
      "/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
      MakeCallback(&CwndTracer));

  Simulator::ScheduleNow(&StartFlow, localSocket, ipInterfs.GetAddress(1),
                         servPort);

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("tcp-large-transfer.tr"));
  p2p.EnablePcapAll("tcp-large-transfer");

  Simulator::Stop(Seconds(1000));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

void StartFlow(Ptr<Socket> localSocket, Ipv4Address servAddress,
               uint16_t servPort) {
  NS_LOG_LOGIC("Starting flow at time " << Simulator::Now().GetSeconds());
  localSocket->Connect(InetSocketAddress(servAddress, servPort));

  localSocket->SetSendCallback(MakeCallback(&WriteUntilBufferFull));
  WriteUntilBufferFull(localSocket, localSocket->GetTxAvailable());
}

void WriteUntilBufferFull(Ptr<Socket> localSocket, uint32_t txSpace) {
  while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable() > 0) {
    uint32_t left = totalTxBytes - currentTxBytes;
    uint32_t dataOffset = currentTxBytes % writeSize;
    uint32_t toWrite = writeSize - dataOffset;
    toWrite = std::min(toWrite, left);
    toWrite = std::min(toWrite, localSocket->GetTxAvailable());
    int amountSent = localSocket->Send(&data[dataOffset], toWrite, 0);
    if (amountSent < 0) {
      return;
    }
    currentTxBytes += amountSent;
  }
  if (currentTxBytes >= totalTxBytes) {
    localSocket->Close();
  }
}
