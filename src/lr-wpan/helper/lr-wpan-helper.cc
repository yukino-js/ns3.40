#include "lr-wpan-helper.h"

#include "ns3/names.h"
#include <ns3/log.h>
#include <ns3/lr-wpan-csmaca.h>
#include <ns3/lr-wpan-error-model.h>
#include <ns3/lr-wpan-net-device.h>
#include <ns3/mobility-model.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/single-model-spectrum-channel.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LrWpanHelper");

static void AsciiLrWpanMacTransmitSinkWithContext(
    Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p) {
  *stream->GetStream() << "t " << Simulator::Now().As(Time::S) << " " << context
                       << " " << *p << std::endl;
}

static void
AsciiLrWpanMacTransmitSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                         Ptr<const Packet> p) {
  *stream->GetStream() << "t " << Simulator::Now().As(Time::S) << " " << *p
                       << std::endl;
}

LrWpanHelper::LrWpanHelper() {
  m_channel = CreateObject<SingleModelSpectrumChannel>();

  Ptr<LogDistancePropagationLossModel> lossModel =
      CreateObject<LogDistancePropagationLossModel>();
  m_channel->AddPropagationLossModel(lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  m_channel->SetPropagationDelayModel(delayModel);
}

LrWpanHelper::LrWpanHelper(bool useMultiModelSpectrumChannel) {
  if (useMultiModelSpectrumChannel) {
    m_channel = CreateObject<MultiModelSpectrumChannel>();
  } else {
    m_channel = CreateObject<SingleModelSpectrumChannel>();
  }
  Ptr<LogDistancePropagationLossModel> lossModel =
      CreateObject<LogDistancePropagationLossModel>();
  m_channel->AddPropagationLossModel(lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel =
      CreateObject<ConstantSpeedPropagationDelayModel>();
  m_channel->SetPropagationDelayModel(delayModel);
}

LrWpanHelper::~LrWpanHelper() {
  m_channel->Dispose();
  m_channel = nullptr;
}

void LrWpanHelper::EnableLogComponents() {
  LogComponentEnableAll(LOG_PREFIX_TIME);
  LogComponentEnableAll(LOG_PREFIX_FUNC);
  LogComponentEnable("LrWpanCsmaCa", LOG_LEVEL_ALL);
  LogComponentEnable("LrWpanErrorModel", LOG_LEVEL_ALL);
  LogComponentEnable("LrWpanInterferenceHelper", LOG_LEVEL_ALL);
  LogComponentEnable("LrWpanMac", LOG_LEVEL_ALL);
  LogComponentEnable("LrWpanNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable("LrWpanPhy", LOG_LEVEL_ALL);
  LogComponentEnable("LrWpanSpectrumSignalParameters", LOG_LEVEL_ALL);
  LogComponentEnable("LrWpanSpectrumValueHelper", LOG_LEVEL_ALL);
}

std::string LrWpanHelper::LrWpanPhyEnumerationPrinter(LrWpanPhyEnumeration e) {
  switch (e) {
  case IEEE_802_15_4_PHY_BUSY:
    return std::string("BUSY");
  case IEEE_802_15_4_PHY_BUSY_RX:
    return std::string("BUSY_RX");
  case IEEE_802_15_4_PHY_BUSY_TX:
    return std::string("BUSY_TX");
  case IEEE_802_15_4_PHY_FORCE_TRX_OFF:
    return std::string("FORCE_TRX_OFF");
  case IEEE_802_15_4_PHY_IDLE:
    return std::string("IDLE");
  case IEEE_802_15_4_PHY_INVALID_PARAMETER:
    return std::string("INVALID_PARAMETER");
  case IEEE_802_15_4_PHY_RX_ON:
    return std::string("RX_ON");
  case IEEE_802_15_4_PHY_SUCCESS:
    return std::string("SUCCESS");
  case IEEE_802_15_4_PHY_TRX_OFF:
    return std::string("TRX_OFF");
  case IEEE_802_15_4_PHY_TX_ON:
    return std::string("TX_ON");
  case IEEE_802_15_4_PHY_UNSUPPORTED_ATTRIBUTE:
    return std::string("UNSUPPORTED_ATTRIBUTE");
  case IEEE_802_15_4_PHY_READ_ONLY:
    return std::string("READ_ONLY");
  case IEEE_802_15_4_PHY_UNSPECIFIED:
    return std::string("UNSPECIFIED");
  default:
    return std::string("INVALID");
  }
}

std::string LrWpanHelper::LrWpanMacStatePrinter(LrWpanMacState e) {
  switch (e) {
  case MAC_IDLE:
    return std::string("MAC_IDLE");
  case CHANNEL_ACCESS_FAILURE:
    return std::string("CHANNEL_ACCESS_FAILURE");
  case CHANNEL_IDLE:
    return std::string("CHANNEL_IDLE");
  case SET_PHY_TX_ON:
    return std::string("SET_PHY_TX_ON");
  default:
    return std::string("INVALID");
  }
}

void LrWpanHelper::AddMobility(Ptr<LrWpanPhy> phy, Ptr<MobilityModel> m) {
  phy->SetMobility(m);
}

NetDeviceContainer LrWpanHelper::Install(NodeContainer c) {
  NetDeviceContainer devices;
  for (auto i = c.Begin(); i != c.End(); i++) {
    Ptr<Node> node = *i;

    Ptr<LrWpanNetDevice> netDevice = CreateObject<LrWpanNetDevice>();
    netDevice->SetChannel(m_channel);
    node->AddDevice(netDevice);
    netDevice->SetNode(node);
    devices.Add(netDevice);
  }
  return devices;
}

Ptr<SpectrumChannel> LrWpanHelper::GetChannel() { return m_channel; }

void LrWpanHelper::SetChannel(Ptr<SpectrumChannel> channel) {
  m_channel = channel;
}

void LrWpanHelper::SetChannel(std::string channelName) {
  Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel>(channelName);
  m_channel = channel;
}

int64_t LrWpanHelper::AssignStreams(NetDeviceContainer c, int64_t stream) {
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (auto i = c.Begin(); i != c.End(); ++i) {
    netDevice = (*i);
    Ptr<LrWpanNetDevice> lrwpan = DynamicCast<LrWpanNetDevice>(netDevice);
    if (lrwpan) {
      currentStream += lrwpan->AssignStreams(currentStream);
    }
  }
  return (currentStream - stream);
}

void LrWpanHelper::CreateAssociatedPan(NetDeviceContainer c, uint16_t panId) {
  NetDeviceContainer devices;
  uint16_t id = 1;
  uint8_t idBuf[2] = {0, 0};
  uint8_t idBuf2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  Mac16Address address16;
  Mac64Address address64;
  Mac16Address coordShortAddr;
  Mac64Address coordExtAddr;

  for (auto i = c.Begin(); i != c.End(); i++) {
    if (id < 0x0001 || id > 0xFFFD) {
      NS_ABORT_MSG("Only 65533 addresses supported. Range [00:01]-[FF:FD]");
    }

    Ptr<LrWpanNetDevice> device = DynamicCast<LrWpanNetDevice>(*i);
    if (device) {
      idBuf[0] = (id >> 8) & 0xff;
      idBuf[1] = (id >> 0) & 0xff;
      address16.CopyFrom(idBuf);

      idBuf2[6] = (id >> 8) & 0xff;
      idBuf2[7] = (id >> 0) & 0xff;
      address64.CopyFrom(idBuf2);

      if (address64 == Mac64Address("00:00:00:00:00:00:00:01")) {
        coordShortAddr = address16;
        coordExtAddr = address64;
      }

      device->GetMac()->SetExtendedAddress(address64);
      device->SetPanAssociation(panId, coordExtAddr, coordShortAddr, address16);

      id++;
    }
  }
}

void LrWpanHelper::SetExtendedAddresses(NetDeviceContainer c) {
  NetDeviceContainer devices;
  uint64_t id = 1;
  uint8_t idBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  Mac64Address address64;

  for (auto i = c.Begin(); i != c.End(); i++) {
    Ptr<LrWpanNetDevice> device = DynamicCast<LrWpanNetDevice>(*i);
    if (device) {
      idBuf[0] = (id >> 56) & 0xff;
      idBuf[1] = (id >> 48) & 0xff;
      idBuf[2] = (id >> 40) & 0xff;
      idBuf[3] = (id >> 32) & 0xff;
      idBuf[4] = (id >> 24) & 0xff;
      idBuf[5] = (id >> 16) & 0xff;
      idBuf[6] = (id >> 8) & 0xff;
      idBuf[7] = (id >> 0) & 0xff;

      address64.CopyFrom(idBuf);

      device->GetMac()->SetExtendedAddress(address64);

      id++;
    }
  }
}

static void PcapSniffLrWpan(Ptr<PcapFileWrapper> file,
                            Ptr<const Packet> packet) {
  file->Write(Simulator::Now(), packet);
}

void LrWpanHelper::EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                                      bool promiscuous, bool explicitFilename) {
  NS_LOG_FUNCTION(this << prefix << nd << promiscuous << explicitFilename);

  Ptr<LrWpanNetDevice> device = nd->GetObject<LrWpanNetDevice>();
  if (!device) {
    NS_LOG_INFO("LrWpanHelper::EnablePcapInternal(): Device "
                << device << " not of type ns3::LrWpanNetDevice");
    return;
  }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename) {
    filename = prefix;
  } else {
    filename = pcapHelper.GetFilenameFromDevice(prefix, device);
  }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(
      filename, std::ios::out, PcapHelper::DLT_IEEE802_15_4);

  if (promiscuous) {
    device->GetMac()->TraceConnectWithoutContext(
        "PromiscSniffer", MakeBoundCallback(&PcapSniffLrWpan, file));
  } else {
    device->GetMac()->TraceConnectWithoutContext(
        "Sniffer", MakeBoundCallback(&PcapSniffLrWpan, file));
  }
}

void LrWpanHelper::EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                                       std::string prefix, Ptr<NetDevice> nd,
                                       bool explicitFilename) {
  uint32_t nodeid = nd->GetNode()->GetId();
  uint32_t deviceid = nd->GetIfIndex();
  std::ostringstream oss;

  Ptr<LrWpanNetDevice> device = nd->GetObject<LrWpanNetDevice>();
  if (!device) {
    NS_LOG_INFO("LrWpanHelper::EnableAsciiInternal(): Device "
                << device << " not of type ns3::LrWpanNetDevice");
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

    asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<LrWpanMac>(
        device->GetMac(), "MacRx", theStream);

    device->GetMac()->TraceConnectWithoutContext(
        "MacTx", MakeBoundCallback(&AsciiLrWpanMacTransmitSinkWithoutContext,
                                   theStream));

    asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<LrWpanMac>(
        device->GetMac(), "MacTxEnqueue", theStream);
    asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<LrWpanMac>(
        device->GetMac(), "MacTxDequeue", theStream);
    asciiTraceHelper.HookDefaultDropSinkWithoutContext<LrWpanMac>(
        device->GetMac(), "MacTxDrop", theStream);

    return;
  }

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::LrWpanNetDevice/Mac/MacRx";
  device->GetMac()->TraceConnect(
      "MacRx", oss.str(),
      MakeBoundCallback(&AsciiTraceHelper::DefaultReceiveSinkWithContext,
                        stream));

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::LrWpanNetDevice/Mac/MacTx";
  device->GetMac()->TraceConnect(
      "MacTx", oss.str(),
      MakeBoundCallback(&AsciiLrWpanMacTransmitSinkWithContext, stream));

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::LrWpanNetDevice/Mac/MacTxEnqueue";
  device->GetMac()->TraceConnect(
      "MacTxEnqueue", oss.str(),
      MakeBoundCallback(&AsciiTraceHelper::DefaultEnqueueSinkWithContext,
                        stream));

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::LrWpanNetDevice/Mac/MacTxDequeue";
  device->GetMac()->TraceConnect(
      "MacTxDequeue", oss.str(),
      MakeBoundCallback(&AsciiTraceHelper::DefaultDequeueSinkWithContext,
                        stream));

  oss.str("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
      << "/$ns3::LrWpanNetDevice/Mac/MacTxDrop";
  device->GetMac()->TraceConnect(
      "MacTxDrop", oss.str(),
      MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

} // namespace ns3
