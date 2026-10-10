
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AttributeValueSample");

int main(int argc, char *argv[]) {
  LogComponentEnable("AttributeValueSample", LOG_LEVEL_INFO);

  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", StringValue("80p"));
  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize",
                     QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, 80)));

  CommandLine cmd(__FILE__);
  cmd.AddValue("maxSize", "ns3::DropTailQueue<Packet>::MaxSize");
  cmd.Parse(argc, argv);

  Ptr<Node> n0 = CreateObject<Node>();

  Ptr<PointToPointNetDevice> net0 = CreateObject<PointToPointNetDevice>();
  n0->AddDevice(net0);

  Ptr<Queue<Packet>> q = CreateObject<DropTailQueue<Packet>>();
  net0->SetQueue(q);

  PointerValue ptr;
  net0->GetAttribute("TxQueue", ptr);
  Ptr<Queue<Packet>> txQueue = ptr.Get<Queue<Packet>>();

  Ptr<DropTailQueue<Packet>> dtq = txQueue->GetObject<DropTailQueue<Packet>>();
  NS_ASSERT(dtq);

  QueueSizeValue limit;
  dtq->GetAttribute("MaxSize", limit);
  NS_LOG_INFO("1.  dtq limit: " << limit.Get());

  txQueue->GetAttribute("MaxSize", limit);
  NS_LOG_INFO("2.  txQueue limit: " << limit.Get());

  txQueue->SetAttribute("MaxSize", StringValue("60p"));
  txQueue->GetAttribute("MaxSize", limit);
  NS_LOG_INFO("3.  txQueue limit changed: " << limit.Get());

  Config::Set("/NodeList/0/DeviceList/0/TxQueue/MaxSize", StringValue("25p"));
  txQueue->GetAttribute("MaxSize", limit);
  NS_LOG_INFO("4.  txQueue limit changed through namespace: " << limit.Get());

  Config::Set("/NodeList/*/DeviceList/*/TxQueue/MaxSize", StringValue("15p"));
  txQueue->GetAttribute("MaxSize", limit);
  NS_LOG_INFO("5.  txQueue limit changed through wildcarded namespace: "
              << limit.Get());

  Simulator::Destroy();

  return 0;
}
