

#include "ns3/core-module.h"
#include "ns3/network-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
  bool verbose = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "turn on log components", verbose);
  cmd.Parse(argc, argv);

  if (verbose) {
    LogComponentEnable("PacketSocketServer", LOG_LEVEL_ALL);
    LogComponentEnable("PacketSocketClient", LOG_LEVEL_ALL);
    LogComponentEnable("SimpleNetDevice", LOG_LEVEL_ALL);
  }

  NodeContainer nodes;
  nodes.Create(2);

  ns3::PacketMetadata::Enable();

  PacketSocketHelper packetSocket;

  packetSocket.Install(nodes);

  Ptr<SimpleNetDevice> txDev;
  txDev = CreateObject<SimpleNetDevice>();
  nodes.Get(0)->AddDevice(txDev);

  Ptr<SimpleNetDevice> rxDev;
  rxDev = CreateObject<SimpleNetDevice>();
  nodes.Get(1)->AddDevice(rxDev);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
  txDev->SetChannel(channel);
  rxDev->SetChannel(channel);
  txDev->SetNode(nodes.Get(0));
  rxDev->SetNode(nodes.Get(1));

  PacketSocketAddress socketAddr;
  socketAddr.SetSingleDevice(txDev->GetIfIndex());
  socketAddr.SetPhysicalAddress(rxDev->GetAddress());
  socketAddr.SetProtocol(1);

  Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient>();
  client->SetRemote(socketAddr);
  nodes.Get(0)->AddApplication(client);

  Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer>();
  server->SetLocal(socketAddr);
  nodes.Get(1)->AddApplication(server);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
