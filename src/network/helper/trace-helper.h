
#ifndef TRACE_HELPER_H
#define TRACE_HELPER_H

#include "net-device-container.h"
#include "node-container.h"

#include "ns3/assert.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/pcap-file-wrapper.h"
#include "ns3/simulator.h"

namespace ns3 {

class PcapHelper {
public:
  enum DataLinkType {
    DLT_NULL = 0,
    DLT_EN10MB = 1,
    DLT_PPP = 9,
    DLT_RAW = 101,
    DLT_IEEE802_11 = 105,
    DLT_LINUX_SLL = 113,
    DLT_PRISM_HEADER = 119,
    DLT_IEEE802_11_RADIO = 127,
    DLT_IEEE802_15_4 = 195,
    DLT_NETLINK = 253,
    DLT_LORATAP = 270
  };

  PcapHelper();

  ~PcapHelper();

  std::string GetFilenameFromDevice(std::string prefix, Ptr<NetDevice> device,
                                    bool useObjectNames = true);

  std::string GetFilenameFromInterfacePair(std::string prefix,
                                           Ptr<Object> object,
                                           uint32_t interface,
                                           bool useObjectNames = true);

  Ptr<PcapFileWrapper>
  CreateFile(std::string filename, std::ios::openmode filemode,
             DataLinkType dataLinkType,
             uint32_t snapLen = std::numeric_limits<uint32_t>::max(),
             int32_t tzCorrection = 0);
  template <typename T>
  void HookDefaultSink(Ptr<T> object, std::string traceName,
                       Ptr<PcapFileWrapper> file);

private:
  static void DefaultSink(Ptr<PcapFileWrapper> file, Ptr<const Packet> p);

  static void SinkWithHeader(Ptr<PcapFileWrapper> file, const Header &header,
                             Ptr<const Packet> p);
};

template <typename T>
void PcapHelper::HookDefaultSink(Ptr<T> object, std::string tracename,
                                 Ptr<PcapFileWrapper> file) {
  bool result = object->TraceConnectWithoutContext(
      tracename, MakeBoundCallback(&DefaultSink, file));
  NS_ASSERT_MSG(result == true,
                "PcapHelper::HookDefaultSink():  Unable to hook \"" << tracename
                                                                    << "\"");
}

class AsciiTraceHelper {
public:
  AsciiTraceHelper();

  ~AsciiTraceHelper();

  std::string GetFilenameFromDevice(std::string prefix, Ptr<NetDevice> device,
                                    bool useObjectNames = true);

  std::string GetFilenameFromInterfacePair(std::string prefix,
                                           Ptr<Object> object,
                                           uint32_t interface,
                                           bool useObjectNames = true);

  Ptr<OutputStreamWrapper>
  CreateFileStream(std::string filename,
                   std::ios::openmode filemode = std::ios::out);

  template <typename T>
  void HookDefaultEnqueueSinkWithoutContext(Ptr<T> object,
                                            std::string traceName,
                                            Ptr<OutputStreamWrapper> stream);

  template <typename T>
  void HookDefaultEnqueueSinkWithContext(Ptr<T> object, std::string context,
                                         std::string traceName,
                                         Ptr<OutputStreamWrapper> stream);

  template <typename T>
  void HookDefaultDropSinkWithoutContext(Ptr<T> object, std::string traceName,
                                         Ptr<OutputStreamWrapper> stream);

  template <typename T>
  void HookDefaultDropSinkWithContext(Ptr<T> object, std::string context,
                                      std::string traceName,
                                      Ptr<OutputStreamWrapper> stream);

  template <typename T>
  void HookDefaultDequeueSinkWithoutContext(Ptr<T> object,
                                            std::string traceName,
                                            Ptr<OutputStreamWrapper> stream);

  template <typename T>
  void HookDefaultDequeueSinkWithContext(Ptr<T> object, std::string context,
                                         std::string traceName,
                                         Ptr<OutputStreamWrapper> stream);

  template <typename T>
  void HookDefaultReceiveSinkWithoutContext(Ptr<T> object,
                                            std::string traceName,
                                            Ptr<OutputStreamWrapper> stream);

  template <typename T>
  void HookDefaultReceiveSinkWithContext(Ptr<T> object, std::string context,
                                         std::string traceName,
                                         Ptr<OutputStreamWrapper> stream);

  static void DefaultEnqueueSinkWithoutContext(Ptr<OutputStreamWrapper> file,
                                               Ptr<const Packet> p);

  static void DefaultEnqueueSinkWithContext(Ptr<OutputStreamWrapper> file,
                                            std::string context,
                                            Ptr<const Packet> p);

  static void DefaultDropSinkWithoutContext(Ptr<OutputStreamWrapper> file,
                                            Ptr<const Packet> p);

  static void DefaultDropSinkWithContext(Ptr<OutputStreamWrapper> file,
                                         std::string context,
                                         Ptr<const Packet> p);

  static void DefaultDequeueSinkWithoutContext(Ptr<OutputStreamWrapper> file,
                                               Ptr<const Packet> p);

  static void DefaultDequeueSinkWithContext(Ptr<OutputStreamWrapper> file,
                                            std::string context,
                                            Ptr<const Packet> p);

  static void DefaultReceiveSinkWithoutContext(Ptr<OutputStreamWrapper> file,
                                               Ptr<const Packet> p);

  static void DefaultReceiveSinkWithContext(Ptr<OutputStreamWrapper> file,
                                            std::string context,
                                            Ptr<const Packet> p);
};

template <typename T>
void AsciiTraceHelper::HookDefaultEnqueueSinkWithoutContext(
    Ptr<T> object, std::string tracename, Ptr<OutputStreamWrapper> file) {
  bool result = object->TraceConnectWithoutContext(
      tracename, MakeBoundCallback(&DefaultEnqueueSinkWithoutContext, file));
  NS_ASSERT_MSG(result == true,
                "AsciiTraceHelper::HookDefaultEnqueueSinkWithoutContext():  "
                "Unable to hook \""
                    << tracename << "\"");
}

template <typename T>
void AsciiTraceHelper::HookDefaultEnqueueSinkWithContext(
    Ptr<T> object, std::string context, std::string tracename,
    Ptr<OutputStreamWrapper> stream) {
  bool result = object->TraceConnect(
      tracename, context,
      MakeBoundCallback(&DefaultEnqueueSinkWithContext, stream));
  NS_ASSERT_MSG(
      result == true,
      "AsciiTraceHelper::HookDefaultEnqueueSinkWithContext():  Unable to hook "
      "\"" << tracename
           << "\"");
}

template <typename T>
void AsciiTraceHelper::HookDefaultDropSinkWithoutContext(
    Ptr<T> object, std::string tracename, Ptr<OutputStreamWrapper> file) {
  bool result = object->TraceConnectWithoutContext(
      tracename, MakeBoundCallback(&DefaultDropSinkWithoutContext, file));
  NS_ASSERT_MSG(
      result == true,
      "AsciiTraceHelper::HookDefaultDropSinkWithoutContext():  Unable to hook "
      "\"" << tracename
           << "\"");
}

template <typename T>
void AsciiTraceHelper::HookDefaultDropSinkWithContext(
    Ptr<T> object, std::string context, std::string tracename,
    Ptr<OutputStreamWrapper> stream) {
  bool result = object->TraceConnect(
      tracename, context,
      MakeBoundCallback(&DefaultDropSinkWithContext, stream));
  NS_ASSERT_MSG(
      result == true,
      "AsciiTraceHelper::HookDefaultDropSinkWithContext():  Unable to hook \""
          << tracename << "\"");
}

template <typename T>
void AsciiTraceHelper::HookDefaultDequeueSinkWithoutContext(
    Ptr<T> object, std::string tracename, Ptr<OutputStreamWrapper> file) {
  bool result = object->TraceConnectWithoutContext(
      tracename, MakeBoundCallback(&DefaultDequeueSinkWithoutContext, file));
  NS_ASSERT_MSG(result == true,
                "AsciiTraceHelper::HookDefaultDequeueSinkWithoutContext():  "
                "Unable to hook \""
                    << tracename << "\"");
}

template <typename T>
void AsciiTraceHelper::HookDefaultDequeueSinkWithContext(
    Ptr<T> object, std::string context, std::string tracename,
    Ptr<OutputStreamWrapper> stream) {
  bool result = object->TraceConnect(
      tracename, context,
      MakeBoundCallback(&DefaultDequeueSinkWithContext, stream));
  NS_ASSERT_MSG(
      result == true,
      "AsciiTraceHelper::HookDefaultDequeueSinkWithContext():  Unable to hook "
      "\"" << tracename
           << "\"");
}

template <typename T>
void AsciiTraceHelper::HookDefaultReceiveSinkWithoutContext(
    Ptr<T> object, std::string tracename, Ptr<OutputStreamWrapper> file) {
  bool result = object->TraceConnectWithoutContext(
      tracename, MakeBoundCallback(&DefaultReceiveSinkWithoutContext, file));
  NS_ASSERT_MSG(result == true,
                "AsciiTraceHelper::HookDefaultReceiveSinkWithoutContext():  "
                "Unable to hook \""
                    << tracename << "\"");
}

template <typename T>
void AsciiTraceHelper::HookDefaultReceiveSinkWithContext(
    Ptr<T> object, std::string context, std::string tracename,
    Ptr<OutputStreamWrapper> stream) {
  bool result = object->TraceConnect(
      tracename, context,
      MakeBoundCallback(&DefaultReceiveSinkWithContext, stream));
  NS_ASSERT_MSG(
      result == true,
      "AsciiTraceHelper::HookDefaultReceiveSinkWithContext():  Unable to hook "
      "\"" << tracename
           << "\"");
}

class PcapHelperForDevice {
public:
  PcapHelperForDevice() {}

  virtual ~PcapHelperForDevice() {}

  virtual void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                                  bool promiscuous, bool explicitFilename) = 0;

  void EnablePcap(std::string prefix, Ptr<NetDevice> nd,
                  bool promiscuous = false, bool explicitFilename = false);

  void EnablePcap(std::string prefix, std::string ndName,
                  bool promiscuous = false, bool explicitFilename = false);

  void EnablePcap(std::string prefix, NetDeviceContainer d,
                  bool promiscuous = false);

  void EnablePcap(std::string prefix, NodeContainer n,
                  bool promiscuous = false);

  void EnablePcap(std::string prefix, uint32_t nodeid, uint32_t deviceid,
                  bool promiscuous = false);

  void EnablePcapAll(std::string prefix, bool promiscuous = false);
};

class AsciiTraceHelperForDevice {
public:
  AsciiTraceHelperForDevice() {}

  virtual ~AsciiTraceHelperForDevice() {}

  virtual void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                                   std::string prefix, Ptr<NetDevice> nd,
                                   bool explicitFilename) = 0;

  void EnableAscii(std::string prefix, Ptr<NetDevice> nd,
                   bool explicitFilename = false);

  void EnableAscii(Ptr<OutputStreamWrapper> stream, Ptr<NetDevice> nd);

  void EnableAscii(std::string prefix, std::string ndName,
                   bool explicitFilename = false);

  void EnableAscii(Ptr<OutputStreamWrapper> stream, std::string ndName);

  void EnableAscii(std::string prefix, NetDeviceContainer d);

  void EnableAscii(Ptr<OutputStreamWrapper> stream, NetDeviceContainer d);

  void EnableAscii(std::string prefix, NodeContainer n);

  void EnableAscii(Ptr<OutputStreamWrapper> stream, NodeContainer n);

  void EnableAsciiAll(std::string prefix);

  void EnableAsciiAll(Ptr<OutputStreamWrapper> stream);

  void EnableAscii(std::string prefix, uint32_t nodeid, uint32_t deviceid,
                   bool explicitFilename);

  void EnableAscii(Ptr<OutputStreamWrapper> stream, uint32_t nodeid,
                   uint32_t deviceid);

private:
  void EnableAsciiImpl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                       uint32_t nodeid, uint32_t deviceid,
                       bool explicitFilename);

  void EnableAsciiImpl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                       NodeContainer n);

  void EnableAsciiImpl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                       NetDeviceContainer d);

  void EnableAsciiImpl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                       std::string ndName, bool explicitFilename);

  void EnableAsciiImpl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                       Ptr<NetDevice> nd, bool explicitFilename);
};

} // namespace ns3

#endif
