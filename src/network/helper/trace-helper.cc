
#include "trace-helper.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/pcap-file-wrapper.h"
#include "ns3/ptr.h"

#include <fstream>
#include <stdint.h>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TraceHelper");

PcapHelper::PcapHelper() { NS_LOG_FUNCTION_NOARGS(); }

PcapHelper::~PcapHelper() { NS_LOG_FUNCTION_NOARGS(); }

Ptr<PcapFileWrapper> PcapHelper::CreateFile(std::string filename,
                                            std::ios::openmode filemode,
                                            DataLinkType dataLinkType,
                                            uint32_t snapLen,
                                            int32_t tzCorrection) {
  NS_LOG_FUNCTION(filename << filemode << dataLinkType << snapLen
                           << tzCorrection);

  Ptr<PcapFileWrapper> file = CreateObject<PcapFileWrapper>();
  file->Open(filename, filemode);
  NS_ABORT_MSG_IF(file->Fail(),
                  "Unable to Open " << filename << " for mode " << filemode);

  file->Init(dataLinkType, snapLen, tzCorrection);
  NS_ABORT_MSG_IF(file->Fail(), "Unable to Init " << filename);

  return file;
}

std::string PcapHelper::GetFilenameFromDevice(std::string prefix,
                                              Ptr<NetDevice> device,
                                              bool useObjectNames) {
  NS_LOG_FUNCTION(prefix << device << useObjectNames);
  NS_ABORT_MSG_UNLESS(!prefix.empty(), "Empty prefix string");

  std::ostringstream oss;
  oss << prefix << "-";

  std::string nodename;
  std::string devicename;

  Ptr<Node> node = device->GetNode();

  if (useObjectNames) {
    nodename = Names::FindName(node);
    devicename = Names::FindName(device);
  }

  if (!nodename.empty()) {
    oss << nodename;
  } else {
    oss << node->GetId();
  }

  oss << "-";

  if (!devicename.empty()) {
    oss << devicename;
  } else {
    oss << device->GetIfIndex();
  }

  oss << ".pcap";

  return oss.str();
}

std::string PcapHelper::GetFilenameFromInterfacePair(std::string prefix,
                                                     Ptr<Object> object,
                                                     uint32_t interface,
                                                     bool useObjectNames) {
  NS_LOG_FUNCTION(prefix << object << interface << useObjectNames);
  NS_ABORT_MSG_UNLESS(!prefix.empty(), "Empty prefix string");

  std::ostringstream oss;
  oss << prefix << "-";

  std::string objname;
  std::string nodename;

  Ptr<Node> node = object->GetObject<Node>();

  if (useObjectNames) {
    objname = Names::FindName(object);
    nodename = Names::FindName(node);
  }

  if (!objname.empty()) {
    oss << objname;
  } else if (!nodename.empty()) {
    oss << nodename;
  } else {
    oss << "n" << node->GetId();
  }

  oss << "-i" << interface << ".pcap";

  return oss.str();
}

void PcapHelper::DefaultSink(Ptr<PcapFileWrapper> file, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(file << p);
  file->Write(Simulator::Now(), p);
}

void PcapHelper::SinkWithHeader(Ptr<PcapFileWrapper> file, const Header &header,
                                Ptr<const Packet> p) {
  NS_LOG_FUNCTION(file << p);
  file->Write(Simulator::Now(), header, p);
}

AsciiTraceHelper::AsciiTraceHelper() { NS_LOG_FUNCTION_NOARGS(); }

AsciiTraceHelper::~AsciiTraceHelper() { NS_LOG_FUNCTION_NOARGS(); }

Ptr<OutputStreamWrapper>
AsciiTraceHelper::CreateFileStream(std::string filename,
                                   std::ios::openmode filemode) {
  NS_LOG_FUNCTION(filename << filemode);

  Ptr<OutputStreamWrapper> StreamWrapper =
      Create<OutputStreamWrapper>(filename, filemode);

  return StreamWrapper;
}

std::string AsciiTraceHelper::GetFilenameFromDevice(std::string prefix,
                                                    Ptr<NetDevice> device,
                                                    bool useObjectNames) {
  NS_LOG_FUNCTION(prefix << device << useObjectNames);
  NS_ABORT_MSG_UNLESS(!prefix.empty(), "Empty prefix string");

  std::ostringstream oss;
  oss << prefix << "-";

  std::string nodename;
  std::string devicename;

  Ptr<Node> node = device->GetNode();

  if (useObjectNames) {
    nodename = Names::FindName(node);
    devicename = Names::FindName(device);
  }

  if (!nodename.empty()) {
    oss << nodename;
  } else {
    oss << node->GetId();
  }

  oss << "-";

  if (!devicename.empty()) {
    oss << devicename;
  } else {
    oss << device->GetIfIndex();
  }

  oss << ".tr";

  return oss.str();
}

std::string AsciiTraceHelper::GetFilenameFromInterfacePair(
    std::string prefix, Ptr<Object> object, uint32_t interface,
    bool useObjectNames) {
  NS_LOG_FUNCTION(prefix << object << interface << useObjectNames);
  NS_ABORT_MSG_UNLESS(!prefix.empty(), "Empty prefix string");

  std::ostringstream oss;
  oss << prefix << "-";

  std::string objname;
  std::string nodename;

  Ptr<Node> node = object->GetObject<Node>();

  if (useObjectNames) {
    objname = Names::FindName(object);
    nodename = Names::FindName(node);
  }

  if (!objname.empty()) {
    oss << objname;
  } else if (!nodename.empty()) {
    oss << nodename;
  } else {
    oss << "n" << node->GetId();
  }

  oss << "-i" << interface << ".tr";

  return oss.str();
}

void AsciiTraceHelper::DefaultEnqueueSinkWithoutContext(
    Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "+ " << Simulator::Now().GetSeconds() << " " << *p
                       << std::endl;
}

void AsciiTraceHelper::DefaultEnqueueSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "+ " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *p << std::endl;
}

void AsciiTraceHelper::DefaultDropSinkWithoutContext(
    Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << *p
                       << std::endl;
}

void AsciiTraceHelper::DefaultDropSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *p << std::endl;
}

void AsciiTraceHelper::DefaultDequeueSinkWithoutContext(
    Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "- " << Simulator::Now().GetSeconds() << " " << *p
                       << std::endl;
}

void AsciiTraceHelper::DefaultDequeueSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "- " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *p << std::endl;
}

void AsciiTraceHelper::DefaultReceiveSinkWithoutContext(
    Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << *p
                       << std::endl;
}

void AsciiTraceHelper::DefaultReceiveSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p) {
  NS_LOG_FUNCTION(stream << p);
  *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " "
                       << context << " " << *p << std::endl;
}

void PcapHelperForDevice::EnablePcap(std::string prefix, Ptr<NetDevice> nd,
                                     bool promiscuous, bool explicitFilename) {
  EnablePcapInternal(prefix, nd, promiscuous, explicitFilename);
}

void PcapHelperForDevice::EnablePcap(std::string prefix, std::string ndName,
                                     bool promiscuous, bool explicitFilename) {
  Ptr<NetDevice> nd = Names::Find<NetDevice>(ndName);
  EnablePcap(prefix, nd, promiscuous, explicitFilename);
}

void PcapHelperForDevice::EnablePcap(std::string prefix, NetDeviceContainer d,
                                     bool promiscuous) {
  for (auto i = d.Begin(); i != d.End(); ++i) {
    Ptr<NetDevice> dev = *i;
    EnablePcap(prefix, dev, promiscuous);
  }
}

void PcapHelperForDevice::EnablePcap(std::string prefix, NodeContainer n,
                                     bool promiscuous) {
  NetDeviceContainer devs;
  for (auto i = n.Begin(); i != n.End(); ++i) {
    Ptr<Node> node = *i;
    for (uint32_t j = 0; j < node->GetNDevices(); ++j) {
      devs.Add(node->GetDevice(j));
    }
  }
  EnablePcap(prefix, devs, promiscuous);
}

void PcapHelperForDevice::EnablePcapAll(std::string prefix, bool promiscuous) {
  EnablePcap(prefix, NodeContainer::GetGlobal(), promiscuous);
}

void PcapHelperForDevice::EnablePcap(std::string prefix, uint32_t nodeid,
                                     uint32_t deviceid, bool promiscuous) {
  NodeContainer n = NodeContainer::GetGlobal();

  for (auto i = n.Begin(); i != n.End(); ++i) {
    Ptr<Node> node = *i;
    if (node->GetId() != nodeid) {
      continue;
    }

    NS_ABORT_MSG_IF(
        deviceid >= node->GetNDevices(),
        "PcapHelperForDevice::EnablePcap(): Unknown deviceid = " << deviceid);
    Ptr<NetDevice> nd = node->GetDevice(deviceid);
    EnablePcap(prefix, nd, promiscuous);
    return;
  }
}

void AsciiTraceHelperForDevice::EnableAscii(std::string prefix,
                                            Ptr<NetDevice> nd,
                                            bool explicitFilename) {
  EnableAsciiInternal(Ptr<OutputStreamWrapper>(), prefix, nd, explicitFilename);
}

void AsciiTraceHelperForDevice::EnableAscii(Ptr<OutputStreamWrapper> stream,
                                            Ptr<NetDevice> nd) {
  EnableAsciiInternal(stream, std::string(), nd, false);
}

void AsciiTraceHelperForDevice::EnableAscii(std::string prefix,
                                            std::string ndName,
                                            bool explicitFilename) {
  EnableAsciiImpl(Ptr<OutputStreamWrapper>(), prefix, ndName, explicitFilename);
}

void AsciiTraceHelperForDevice::EnableAscii(Ptr<OutputStreamWrapper> stream,
                                            std::string ndName) {
  EnableAsciiImpl(stream, std::string(), ndName, false);
}

void AsciiTraceHelperForDevice::EnableAsciiImpl(Ptr<OutputStreamWrapper> stream,
                                                std::string prefix,
                                                std::string ndName,
                                                bool explicitFilename) {
  Ptr<NetDevice> nd = Names::Find<NetDevice>(ndName);
  EnableAsciiInternal(stream, prefix, nd, explicitFilename);
}

void AsciiTraceHelperForDevice::EnableAscii(std::string prefix,
                                            NetDeviceContainer d) {
  EnableAsciiImpl(Ptr<OutputStreamWrapper>(), prefix, d);
}

void AsciiTraceHelperForDevice::EnableAscii(Ptr<OutputStreamWrapper> stream,
                                            NetDeviceContainer d) {
  EnableAsciiImpl(stream, std::string(), d);
}

void AsciiTraceHelperForDevice::EnableAsciiImpl(Ptr<OutputStreamWrapper> stream,
                                                std::string prefix,
                                                NetDeviceContainer d) {
  for (auto i = d.Begin(); i != d.End(); ++i) {
    Ptr<NetDevice> dev = *i;
    EnableAsciiInternal(stream, prefix, dev, false);
  }
}

void AsciiTraceHelperForDevice::EnableAscii(std::string prefix,
                                            NodeContainer n) {
  EnableAsciiImpl(Ptr<OutputStreamWrapper>(), prefix, n);
}

void AsciiTraceHelperForDevice::EnableAscii(Ptr<OutputStreamWrapper> stream,
                                            NodeContainer n) {
  EnableAsciiImpl(stream, std::string(), n);
}

void AsciiTraceHelperForDevice::EnableAsciiImpl(Ptr<OutputStreamWrapper> stream,
                                                std::string prefix,
                                                NodeContainer n) {
  NetDeviceContainer devs;
  for (auto i = n.Begin(); i != n.End(); ++i) {
    Ptr<Node> node = *i;
    for (uint32_t j = 0; j < node->GetNDevices(); ++j) {
      devs.Add(node->GetDevice(j));
    }
  }
  EnableAsciiImpl(stream, prefix, devs);
}

void AsciiTraceHelperForDevice::EnableAsciiAll(std::string prefix) {
  EnableAsciiImpl(Ptr<OutputStreamWrapper>(), prefix,
                  NodeContainer::GetGlobal());
}

void AsciiTraceHelperForDevice::EnableAsciiAll(
    Ptr<OutputStreamWrapper> stream) {
  EnableAsciiImpl(stream, std::string(), NodeContainer::GetGlobal());
}

void AsciiTraceHelperForDevice::EnableAscii(Ptr<OutputStreamWrapper> stream,
                                            uint32_t nodeid,
                                            uint32_t deviceid) {
  EnableAsciiImpl(stream, std::string(), nodeid, deviceid, false);
}

void AsciiTraceHelperForDevice::EnableAscii(std::string prefix, uint32_t nodeid,
                                            uint32_t deviceid,
                                            bool explicitFilename) {
  EnableAsciiImpl(Ptr<OutputStreamWrapper>(), prefix, nodeid, deviceid,
                  explicitFilename);
}

void AsciiTraceHelperForDevice::EnableAsciiImpl(Ptr<OutputStreamWrapper> stream,
                                                std::string prefix,
                                                uint32_t nodeid,
                                                uint32_t deviceid,
                                                bool explicitFilename) {
  NodeContainer n = NodeContainer::GetGlobal();

  for (auto i = n.Begin(); i != n.End(); ++i) {
    Ptr<Node> node = *i;
    if (node->GetId() != nodeid) {
      continue;
    }

    NS_ABORT_MSG_IF(
        deviceid >= node->GetNDevices(),
        "AsciiTraceHelperForDevice::EnableAscii(): Unknown deviceid = "
            << deviceid);

    Ptr<NetDevice> nd = node->GetDevice(deviceid);

    EnableAsciiInternal(stream, prefix, nd, explicitFilename);
    return;
  }
}

} // namespace ns3
