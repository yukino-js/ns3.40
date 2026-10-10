
#include "csma-helper.h"

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/csma-channel.h"
#include "ns3/csma-net-device.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/trace-helper.h"

#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CsmaHelper");

CsmaHelper::CsmaHelper() {
  m_queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
  m_deviceFactory.SetTypeId("ns3::CsmaNetDevice");
  m_channelFactory.SetTypeId("ns3::CsmaChannel");
  m_enableFlowControl = true;
}

void CsmaHelper::SetDeviceAttribute(std::string n1, const AttributeValue &v1) {
  m_deviceFactory.Set(n1, v1);
}

void CsmaHelper::SetChannelAttribute(std::string n1, const AttributeValue &v1) {
  m_channelFactory.Set(n1, v1);
}

void CsmaHelper::DisableFlowControl() { m_enableFlowControl = false; }

void CsmaHelper::EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                                    bool promiscuous, bool explicitFilename) {
  Ptr<CsmaNetDevice> device = nd->GetObject<CsmaNetDevice>();
  if (!device) {
    NS_LOG_INFO("CsmaHelper::EnablePcapInternal(): Device "
                << device << " not of type ns3::CsmaNetDevice");
    return;
  }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename) {
    filename = prefix;
  } else {
    filename = pcapHelper.GetFilenameFromDevice(prefix, device);
  }

  Ptr<PcapFileWrapper> file =
      pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_EN10MB);
  if (promiscuous) {
    pcapHelper.HookDefaultSink<CsmaNetDevice>(device, "PromiscSniffer", file);
  } else {
    pcapHelper.HookDefaultSink<CsmaNetDevice>(device, "Sniffer", file);
  }
}

void CsmaHelper::EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                                     std::string prefix, Ptr<NetDevice> nd,
                                     bool explicitFilename) {
  Ptr<CsmaNetDevice> device = nd->GetObject<CsmaNetDevice>();
  if (!device) {
    NS_LOG_INFO("CsmaHelper::EnableAsciiInternal(): Device "
                << device << " not of type ns3::CsmaNetDevice");
    return;
  }

  Packet::EnablePrinting();

  if (!stream) {
    AsciiTraceHelper asciiTraceHelper;

    std::string filename;
    if (explicitFilename) {
      filename = prefix;
    } else {
      filename = asciiTraceHelper.GetFilenameFromDevice(prefix, device);
    }

    Ptr<OutputStreamWrapper> theStream =
        asciiTraceHelper.CreateFileStream(filename);

    asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<CsmaNetDevice>(
        device, "MacRx", theStream);

    Ptr<Queue<Packet>> queue = device->GetQueue();
    asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue<Packet>>(
        queue, "Enqueue", theStream);
    asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue<Packet>>(
        queue, "Drop", theStream);
    asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue<Packet>>(
        queue, "Dequeue", theStream);

    return;
  }

  uint32_t nodeid = nd->GetNode()->GetId();
  uint32_t deviceid = nd->GetIfIndex();
  std::ostringstream oss;

  oss << "/NodeList/" << nd->GetNode()->GetId() << "/DeviceList/" << deviceid
      << "/$ns3::CsmaNetDevice/MacRx";
  Config::Connect(
      oss.str(), MakeBoundCallback(
                     &AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::CsmaNetDevice/TxQueue/Enqueue";
  Config::Connect(
      oss.str(), MakeBoundCallback(
                     &AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::CsmaNetDevice/TxQueue/Dequeue";
  Config::Connect(
      oss.str(), MakeBoundCallback(
                     &AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::CsmaNetDevice/TxQueue/Drop";
  Config::Connect(
      oss.str(),
      MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer CsmaHelper::Install(Ptr<Node> node) const {
  Ptr<CsmaChannel> channel =
      m_channelFactory.Create()->GetObject<CsmaChannel>();
  return Install(node, channel);
}

NetDeviceContainer CsmaHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return Install(node);
}

NetDeviceContainer CsmaHelper::Install(Ptr<Node> node,
                                       Ptr<CsmaChannel> channel) const {
  return NetDeviceContainer(InstallPriv(node, channel));
}

NetDeviceContainer CsmaHelper::Install(Ptr<Node> node,
                                       std::string channelName) const {
  Ptr<CsmaChannel> channel = Names::Find<CsmaChannel>(channelName);
  return NetDeviceContainer(InstallPriv(node, channel));
}

NetDeviceContainer CsmaHelper::Install(std::string nodeName,
                                       Ptr<CsmaChannel> channel) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return NetDeviceContainer(InstallPriv(node, channel));
}

NetDeviceContainer CsmaHelper::Install(std::string nodeName,
                                       std::string channelName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  Ptr<CsmaChannel> channel = Names::Find<CsmaChannel>(channelName);
  return NetDeviceContainer(InstallPriv(node, channel));
}

NetDeviceContainer CsmaHelper::Install(const NodeContainer &c) const {
  Ptr<CsmaChannel> channel =
      m_channelFactory.Create()->GetObject<CsmaChannel>();

  return Install(c, channel);
}

NetDeviceContainer CsmaHelper::Install(const NodeContainer &c,
                                       Ptr<CsmaChannel> channel) const {
  NetDeviceContainer devs;

  for (auto i = c.Begin(); i != c.End(); i++) {
    devs.Add(InstallPriv(*i, channel));
  }

  return devs;
}

NetDeviceContainer CsmaHelper::Install(const NodeContainer &c,
                                       std::string channelName) const {
  Ptr<CsmaChannel> channel = Names::Find<CsmaChannel>(channelName);
  return Install(c, channel);
}

int64_t CsmaHelper::AssignStreams(NetDeviceContainer c, int64_t stream) {
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    netDevice = (*i);
    Ptr<CsmaNetDevice> csma = DynamicCast<CsmaNetDevice>(netDevice);
    if (csma) {
      currentStream += csma->AssignStreams(currentStream);
    }
  }
  return (currentStream - stream);
}

Ptr<NetDevice> CsmaHelper::InstallPriv(Ptr<Node> node,
                                       Ptr<CsmaChannel> channel) const {
  Ptr<CsmaNetDevice> device = m_deviceFactory.Create<CsmaNetDevice>();
  device->SetAddress(Mac48Address::Allocate());
  node->AddDevice(device);
  Ptr<Queue<Packet>> queue = m_queueFactory.Create<Queue<Packet>>();
  device->SetQueue(queue);
  device->Attach(channel);
  if (m_enableFlowControl) {
    Ptr<NetDeviceQueueInterface> ndqi = CreateObject<NetDeviceQueueInterface>();
    ndqi->GetTxQueue(0)->ConnectQueueTraces(queue);
    device->AggregateObject(ndqi);
  }
  return device;
}

} // namespace ns3
