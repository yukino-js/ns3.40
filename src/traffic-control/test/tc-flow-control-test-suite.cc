
#include "ns3/config.h"
#include "ns3/data-rate.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/node-container.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/uinteger.h"

#include <algorithm>
#include <string>

using namespace ns3;

class QueueDiscTestItem : public QueueDiscItem {
public:
  QueueDiscTestItem(Ptr<Packet> p);
  ~QueueDiscTestItem() override;

  QueueDiscTestItem() = delete;
  QueueDiscTestItem(const QueueDiscTestItem &) = delete;
  QueueDiscTestItem &operator=(const QueueDiscTestItem &) = delete;

  void AddHeader() override;
  bool Mark() override;
};

QueueDiscTestItem::QueueDiscTestItem(Ptr<Packet> p)
    : QueueDiscItem(p, Mac48Address(), 0) {}

QueueDiscTestItem::~QueueDiscTestItem() {}

void QueueDiscTestItem::AddHeader() {}

bool QueueDiscTestItem::Mark() { return false; }

class TcFlowControlTestCase : public TestCase {
public:
  TcFlowControlTestCase(QueueSizeUnit tt, uint32_t deviceQueueLength,
                        uint32_t totalTxPackets);
  ~TcFlowControlTestCase() override;

private:
  void DoRun() override;
  void SendPackets(Ptr<Node> n, uint16_t nPackets);
  void CheckPacketsInDeviceQueue(Ptr<NetDevice> dev, uint16_t nPackets,
                                 const std::string msg);
  void CheckDeviceQueueStopped(Ptr<NetDevice> dev, bool value,
                               const std::string msg);
  void CheckPacketsInQueueDisc(Ptr<NetDevice> dev, uint16_t nPackets,
                               const std::string msg);
  QueueSizeUnit m_type;
  uint32_t m_deviceQueueLength;
  uint32_t m_totalTxPackets;
};

TcFlowControlTestCase::TcFlowControlTestCase(QueueSizeUnit tt,
                                             uint32_t deviceQueueLength,
                                             uint32_t totalTxPackets)
    : TestCase("Test the operation of the flow control mechanism"), m_type(tt),
      m_deviceQueueLength(deviceQueueLength), m_totalTxPackets(totalTxPackets) {
}

TcFlowControlTestCase::~TcFlowControlTestCase() {}

void TcFlowControlTestCase::SendPackets(Ptr<Node> n, uint16_t nPackets) {
  Ptr<TrafficControlLayer> tc = n->GetObject<TrafficControlLayer>();
  for (uint16_t i = 0; i < nPackets; i++) {
    tc->Send(n->GetDevice(0), Create<QueueDiscTestItem>(Create<Packet>(1000)));
  }
}

void TcFlowControlTestCase::CheckPacketsInDeviceQueue(Ptr<NetDevice> dev,
                                                      uint16_t nPackets,
                                                      const std::string msg) {
  PointerValue ptr;
  dev->GetAttributeFailSafe("TxQueue", ptr);
  Ptr<Queue<Packet>> queue = ptr.Get<Queue<Packet>>();
  NS_TEST_EXPECT_MSG_EQ(queue->GetNPackets(), nPackets, msg);
}

void TcFlowControlTestCase::CheckDeviceQueueStopped(Ptr<NetDevice> dev,
                                                    bool value,
                                                    const std::string msg) {
  Ptr<NetDeviceQueueInterface> ndqi = dev->GetObject<NetDeviceQueueInterface>();
  NS_ASSERT_MSG(
      ndqi, "A device queue interface has not been aggregated to the device");
  NS_TEST_EXPECT_MSG_EQ(ndqi->GetTxQueue(0)->IsStopped(), value, msg);
}

void TcFlowControlTestCase::CheckPacketsInQueueDisc(Ptr<NetDevice> dev,
                                                    uint16_t nPackets,
                                                    const std::string msg) {
  Ptr<TrafficControlLayer> tc =
      dev->GetNode()->GetObject<TrafficControlLayer>();
  Ptr<QueueDisc> qdisc = tc->GetRootQueueDiscOnDevice(dev);
  NS_TEST_EXPECT_MSG_EQ(qdisc->GetNPackets(), nPackets, msg);
}

void TcFlowControlTestCase::DoRun() {
  NodeContainer n;
  n.Create(2);

  n.Get(0)->AggregateObject(CreateObject<TrafficControlLayer>());
  n.Get(1)->AggregateObject(CreateObject<TrafficControlLayer>());

  SimpleNetDeviceHelper simple;

  NetDeviceContainer rxDevC = simple.Install(n.Get(1));

  simple.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Mb/s")));
  simple.SetQueue("ns3::DropTailQueue", "MaxSize",
                  StringValue(m_type == QueueSizeUnit::PACKETS
                                  ? std::to_string(m_deviceQueueLength) + "p"
                                  : std::to_string(m_deviceQueueLength) + "B"));

  Ptr<NetDevice> txDev;
  txDev = simple
              .Install(n.Get(0),
                       DynamicCast<SimpleChannel>(rxDevC.Get(0)->GetChannel()))
              .Get(0);
  txDev->SetMtu(2500);

  TrafficControlHelper tch = TrafficControlHelper::Default();
  tch.Install(txDev);

  Simulator::Schedule(Time(Seconds(0)), &TcFlowControlTestCase::SendPackets,
                      this, n.Get(0), m_totalTxPackets);

  if (m_type == QueueSizeUnit::PACKETS) {

    uint32_t checkTimeMs = 0;
    uint32_t deviceQueuePackets = 0;
    uint32_t qdiscPackets = 0;

    uint32_t txPackets = 0;
    for (txPackets = 1; txPackets <= m_totalTxPackets; txPackets++) {
      checkTimeMs = 8 * (txPackets - 1) + 1;
      deviceQueuePackets =
          std::min(m_totalTxPackets - txPackets, m_deviceQueueLength);
      qdiscPackets = std::max(m_totalTxPackets - txPackets - deviceQueuePackets,
                              (uint32_t)0);
      if (deviceQueuePackets == m_deviceQueueLength) {
        Simulator::Schedule(Time(MilliSeconds(checkTimeMs)),
                            &TcFlowControlTestCase::CheckDeviceQueueStopped,
                            this, txDev, true,
                            "The device queue must be stopped after " +
                                std::to_string(checkTimeMs) + "ms");
      } else {
        Simulator::Schedule(Time(MilliSeconds(checkTimeMs)),
                            &TcFlowControlTestCase::CheckDeviceQueueStopped,
                            this, txDev, false,
                            "The device queue must not be stopped after " +
                                std::to_string(checkTimeMs) + "ms");
      }

      Simulator::Schedule(Time(MilliSeconds(checkTimeMs)),
                          &TcFlowControlTestCase::CheckPacketsInDeviceQueue,
                          this, txDev, deviceQueuePackets,
                          "There must be " +
                              std::to_string(m_deviceQueueLength) +
                              " packets in the device after " +
                              std::to_string(checkTimeMs) + "ms");

      Simulator::Schedule(Time(MilliSeconds(checkTimeMs)),
                          &TcFlowControlTestCase::CheckPacketsInQueueDisc, this,
                          txDev, qdiscPackets,
                          "There must be " + std::to_string(qdiscPackets) +
                              " packets in the queue disc after " +
                              std::to_string(checkTimeMs) + "ms");
    }
  } else {

    Simulator::Schedule(
        Time(MilliSeconds(1)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 3,
        "There must be 3 packets in the device queue after 1ms");
    Simulator::Schedule(
        Time(MilliSeconds(1)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, true, "The device queue must be stopped after 1ms");
    Simulator::Schedule(
        Time(MilliSeconds(1)), &TcFlowControlTestCase::CheckPacketsInQueueDisc,
        this, txDev, 6, "There must be 6 packets in the queue disc after 1ms");

    Simulator::Schedule(
        Time(MilliSeconds(9)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 3,
        "There must be 3 packets in the device queue after 9ms");
    Simulator::Schedule(
        Time(MilliSeconds(9)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, true, "The device queue must be stopped after 9ms");
    Simulator::Schedule(
        Time(MilliSeconds(9)), &TcFlowControlTestCase::CheckPacketsInQueueDisc,
        this, txDev, 5, "There must be 5 packets in the queue disc after 9ms");

    Simulator::Schedule(
        Time(MilliSeconds(17)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 3,
        "There must be 3 packets in the device queue after 17ms");
    Simulator::Schedule(
        Time(MilliSeconds(17)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, true, "The device queue must be stopped after 17ms");
    Simulator::Schedule(
        Time(MilliSeconds(17)), &TcFlowControlTestCase::CheckPacketsInQueueDisc,
        this, txDev, 4, "There must be 4 packets in the queue disc after 17ms");

    Simulator::Schedule(
        Time(MilliSeconds(25)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 3,
        "There must be 3 packets in the device queue after 25ms");
    Simulator::Schedule(
        Time(MilliSeconds(25)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, true, "The device queue must be stopped after 25ms");
    Simulator::Schedule(
        Time(MilliSeconds(25)), &TcFlowControlTestCase::CheckPacketsInQueueDisc,
        this, txDev, 3, "There must be 3 packets in the queue disc after 25ms");

    Simulator::Schedule(
        Time(MilliSeconds(33)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 3,
        "There must be 3 packets in the device queue after 33ms");
    Simulator::Schedule(
        Time(MilliSeconds(33)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, true, "The device queue must be stopped after 33ms");
    Simulator::Schedule(
        Time(MilliSeconds(33)), &TcFlowControlTestCase::CheckPacketsInQueueDisc,
        this, txDev, 2, "There must be 2 packets in the queue disc after 33ms");

    Simulator::Schedule(
        Time(MilliSeconds(41)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 3,
        "There must be 3 packets in the device queue after 41ms");
    Simulator::Schedule(
        Time(MilliSeconds(41)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, true, "The device queue must be stopped after 41ms");
    Simulator::Schedule(
        Time(MilliSeconds(41)), &TcFlowControlTestCase::CheckPacketsInQueueDisc,
        this, txDev, 1, "There must be 1 packet in the queue disc after 41ms");

    Simulator::Schedule(
        Time(MilliSeconds(49)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 3,
        "There must be 3 packets in the device queue after 49ms");
    Simulator::Schedule(
        Time(MilliSeconds(49)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, true, "The device queue must be stopped after 49ms");
    Simulator::Schedule(Time(MilliSeconds(49)),
                        &TcFlowControlTestCase::CheckPacketsInQueueDisc, this,
                        txDev, 0, "The queue disc must be empty after 49ms");

    Simulator::Schedule(
        Time(MilliSeconds(57)),
        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this, txDev, 2,
        "There must be 2 packets in the device queue after 57ms");
    Simulator::Schedule(
        Time(MilliSeconds(57)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, false, "The device queue must not be stopped after 57ms");
    Simulator::Schedule(Time(MilliSeconds(57)),
                        &TcFlowControlTestCase::CheckPacketsInQueueDisc, this,
                        txDev, 0, "The queue disc must be empty after 57ms");

    Simulator::Schedule(Time(MilliSeconds(81)),
                        &TcFlowControlTestCase::CheckPacketsInDeviceQueue, this,
                        txDev, 0, "The device queue must be empty after 81ms");
    Simulator::Schedule(
        Time(MilliSeconds(81)), &TcFlowControlTestCase::CheckDeviceQueueStopped,
        this, txDev, false, "The device queue must not be stopped after 81ms");
    Simulator::Schedule(Time(MilliSeconds(81)),
                        &TcFlowControlTestCase::CheckPacketsInQueueDisc, this,
                        txDev, 0, "The queue disc must be empty after 81ms");
  }

  Simulator::Run();
  Simulator::Destroy();
}

static class TcFlowControlTestSuite : public TestSuite {
public:
  TcFlowControlTestSuite() : TestSuite("tc-flow-control", UNIT) {
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 1, 10),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 5, 10),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 9, 10),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 10, 10),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 11, 10),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 15, 10),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 1, 1),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 2, 1),
                TestCase::QUICK);
    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::PACKETS, 5, 1),
                TestCase::QUICK);

    AddTestCase(new TcFlowControlTestCase(QueueSizeUnit::BYTES, 5000, 10),
                TestCase::QUICK);
  }
} g_tcFlowControlTestSuite;
