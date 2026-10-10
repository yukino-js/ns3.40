
#include "fd-net-device-helper.h"

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/fd-net-device.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/trace-helper.h"

#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FdNetDeviceHelper");

FdNetDeviceHelper::FdNetDeviceHelper() {
  m_deviceFactory.SetTypeId("ns3::FdNetDevice");
}

void FdNetDeviceHelper::SetTypeId(std::string type) {
  m_deviceFactory.SetTypeId(type);
}

void FdNetDeviceHelper::SetAttribute(std::string n1, const AttributeValue &v1) {
  NS_LOG_FUNCTION(this);
  m_deviceFactory.Set(n1, v1);
}

void FdNetDeviceHelper::EnablePcapInternal(std::string prefix,
                                           Ptr<NetDevice> nd, bool promiscuous,
                                           bool explicitFilename) {
  Ptr<FdNetDevice> device = nd->GetObject<FdNetDevice>();
  if (!device) {
    NS_LOG_INFO("FdNetDeviceHelper::EnablePcapInternal(): Device "
                << device << " not of type ns3::FdNetDevice");
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
    pcapHelper.HookDefaultSink<FdNetDevice>(device, "PromiscSniffer", file);
  } else {
    pcapHelper.HookDefaultSink<FdNetDevice>(device, "Sniffer", file);
  }
}

void FdNetDeviceHelper::EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                                            std::string prefix,
                                            Ptr<NetDevice> nd,
                                            bool explicitFilename) {
  Ptr<FdNetDevice> device = nd->GetObject<FdNetDevice>();
  if (!device) {
    NS_LOG_INFO("FdNetDeviceHelper::EnableAsciiInternal(): Device "
                << device << " not of type ns3::FdNetDevice");
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

    asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<FdNetDevice>(
        device, "MacRx", theStream);

    return;
  }

  uint32_t deviceid = nd->GetIfIndex();
  std::ostringstream oss;

  oss << "/NodeList/" << nd->GetNode()->GetId() << "/DeviceList/" << deviceid
      << "/$ns3::FdNetDevice/MacRx";
  Config::Connect(
      oss.str(), MakeBoundCallback(
                     &AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));
}

NetDeviceContainer FdNetDeviceHelper::Install(Ptr<Node> node) const {
  return NetDeviceContainer(InstallPriv(node));
}

NetDeviceContainer FdNetDeviceHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return NetDeviceContainer(InstallPriv(node));
}

NetDeviceContainer FdNetDeviceHelper::Install(const NodeContainer &c) const {
  NetDeviceContainer devs;

  for (auto i = c.Begin(); i != c.End(); i++) {
    devs.Add(InstallPriv(*i));
  }

  return devs;
}

Ptr<NetDevice> FdNetDeviceHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<FdNetDevice> device = m_deviceFactory.Create<FdNetDevice>();
  device->SetAddress(Mac48Address::Allocate());
  node->AddDevice(device);
  return device;
}

} // namespace ns3
