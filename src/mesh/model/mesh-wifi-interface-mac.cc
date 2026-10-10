
#include "mesh-wifi-interface-mac.h"

#include "mesh-wifi-beacon.h"

#include "ns3/boolean.h"
#include "ns3/channel-access-manager.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/mac-tx-middle.h"
#include "ns3/pointer.h"
#include "ns3/qos-txop.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/wifi-mac-queue-scheduler.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-utils.h"
#include "ns3/yans-wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MeshWifiInterfaceMac");

NS_OBJECT_ENSURE_REGISTERED(MeshWifiInterfaceMac);

TypeId MeshWifiInterfaceMac::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::MeshWifiInterfaceMac")
          .SetParent<WifiMac>()
          .SetGroupName("Mesh")
          .AddConstructor<MeshWifiInterfaceMac>()
          .AddAttribute(
              "BeaconInterval", "Beacon Interval", TimeValue(Seconds(0.5)),

              MakeTimeAccessor(&MeshWifiInterfaceMac::m_beaconInterval),
              MakeTimeChecker())
          .AddAttribute("RandomStart",
                        "Window when beacon generating starts (uniform random) "
                        "in seconds",
                        TimeValue(Seconds(0.5)),
                        MakeTimeAccessor(&MeshWifiInterfaceMac::m_randomStart),
                        MakeTimeChecker())
          .AddAttribute(
              "BeaconGeneration", "Enable/Disable Beaconing.",
              BooleanValue(true),
              MakeBooleanAccessor(&MeshWifiInterfaceMac::SetBeaconGeneration,
                                  &MeshWifiInterfaceMac::GetBeaconGeneration),
              MakeBooleanChecker());
  return tid;
}

MeshWifiInterfaceMac::MeshWifiInterfaceMac()
    : m_standard(WIFI_STANDARD_80211a) {
  NS_LOG_FUNCTION(this);

  SetTypeOfStation(MESH);
  m_coefficient = CreateObject<UniformRandomVariable>();
}

MeshWifiInterfaceMac::~MeshWifiInterfaceMac() { NS_LOG_FUNCTION(this); }

bool MeshWifiInterfaceMac::CanForwardPacketsTo(Mac48Address to) const {
  return true;
}

void MeshWifiInterfaceMac::Enqueue(Ptr<Packet> packet, Mac48Address to,
                                   Mac48Address from) {
  NS_LOG_FUNCTION(this << packet << to << from);
  ForwardDown(packet, from, to);
}

void MeshWifiInterfaceMac::Enqueue(Ptr<Packet> packet, Mac48Address to) {
  NS_LOG_FUNCTION(this << packet << to);
  ForwardDown(packet, GetAddress(), to);
}

bool MeshWifiInterfaceMac::SupportsSendFrom() const { return true; }

void MeshWifiInterfaceMac::SetLinkUpCallback(Callback<void> linkUp) {
  NS_LOG_FUNCTION(this);
  WifiMac::SetLinkUpCallback(linkUp);

  linkUp();
}

void MeshWifiInterfaceMac::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_plugins.clear();
  m_beaconSendEvent.Cancel();

  WifiMac::DoDispose();
}

void MeshWifiInterfaceMac::DoInitialize() {
  NS_LOG_FUNCTION(this);
  m_coefficient->SetAttribute("Max", DoubleValue(m_randomStart.GetSeconds()));
  if (m_beaconEnable) {
    Time randomStart = Seconds(m_coefficient->GetValue());
    NS_ASSERT(!m_beaconSendEvent.IsRunning());
    m_beaconSendEvent = Simulator::Schedule(
        randomStart, &MeshWifiInterfaceMac::SendBeacon, this);
    m_tbtt = Simulator::Now() + randomStart;
  } else {
    m_beaconSendEvent.Cancel();
  }
}

int64_t MeshWifiInterfaceMac::AssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  int64_t currentStream = stream;
  m_coefficient->SetStream(currentStream++);
  for (auto i = m_plugins.begin(); i < m_plugins.end(); i++) {
    currentStream += (*i)->AssignStreams(currentStream);
  }
  return (currentStream - stream);
}

void MeshWifiInterfaceMac::InstallPlugin(
    Ptr<MeshWifiInterfaceMacPlugin> plugin) {
  NS_LOG_FUNCTION(this);

  plugin->SetParent(this);
  m_plugins.push_back(plugin);
}

uint16_t MeshWifiInterfaceMac::GetFrequencyChannel() const {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(GetWifiPhy());
  return GetWifiPhy()->GetChannelNumber();
}

void MeshWifiInterfaceMac::SwitchFrequencyChannel(uint16_t new_id) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(GetWifiPhy());
  GetWifiPhy()->SetOperatingChannel(
      WifiPhy::ChannelTuple{new_id, 0, GetWifiPhy()->GetPhyBand(), 0});
  GetLink(SINGLE_LINK_OP_ID)
      .channelAccessManager->NotifyNavResetNow(Seconds(0));
}

void MeshWifiInterfaceMac::ForwardDown(Ptr<Packet> packet, Mac48Address from,
                                       Mac48Address to) {
  WifiMacHeader hdr;
  hdr.SetType(WIFI_MAC_QOSDATA);
  hdr.SetAddr2(GetAddress());
  hdr.SetAddr3(to);
  hdr.SetAddr4(from);
  hdr.SetDsFrom();
  hdr.SetDsTo();
  hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
  hdr.SetQosNoEosp();
  hdr.SetQosNoAmsdu();
  hdr.SetQosTxopLimit(0);
  hdr.SetAddr1(Mac48Address());
  for (auto i = m_plugins.end() - 1; i != m_plugins.begin() - 1; i--) {
    bool drop = !((*i)->UpdateOutcomingFrame(packet, hdr, from, to));
    if (drop) {
      return;
    }
  }
  NS_ASSERT(hdr.GetAddr1() != Mac48Address());
  if (GetWifiRemoteStationManager()->IsBrandNew(hdr.GetAddr1())) {
    for (const auto &mode : GetWifiPhy()->GetModeList()) {
      GetWifiRemoteStationManager()->AddSupportedMode(hdr.GetAddr1(), mode);
    }
    GetWifiRemoteStationManager()->RecordDisassociated(hdr.GetAddr1());
  }
  AcIndex ac;
  SocketPriorityTag tag;
  if (packet->RemovePacketTag(tag)) {
    hdr.SetQosTid(tag.GetPriority());
    ac = QosUtilsMapTidToAc(tag.GetPriority());
  } else {
    ac = AC_BE;
    hdr.SetQosTid(0);
  }
  m_stats.sentFrames++;
  m_stats.sentBytes += packet->GetSize();
  NS_ASSERT(GetQosTxop(ac) != nullptr);
  GetQosTxop(ac)->Queue(packet, hdr);
}

void MeshWifiInterfaceMac::SendManagementFrame(Ptr<Packet> packet,
                                               const WifiMacHeader &hdr) {
  WifiMacHeader header = hdr;
  for (auto i = m_plugins.end() - 1; i != m_plugins.begin() - 1; i--) {
    bool drop = !((*i)->UpdateOutcomingFrame(packet, header, Mac48Address(),
                                             Mac48Address()));
    if (drop) {
      return;
    }
  }
  m_stats.sentFrames++;
  m_stats.sentBytes += packet->GetSize();
  if ((GetQosTxop(AC_VO) == nullptr) || (GetQosTxop(AC_BK) == nullptr)) {
    NS_FATAL_ERROR("Voice or Background queue is not set up!");
  }
  if (hdr.GetAddr1() != Mac48Address::GetBroadcast()) {
    GetQosTxop(AC_VO)->Queue(packet, header);
  } else {
    GetQosTxop(AC_BK)->Queue(packet, header);
  }
}

AllSupportedRates MeshWifiInterfaceMac::GetSupportedRates() const {
  AllSupportedRates rates;
  for (const auto &mode : GetWifiPhy()->GetModeList()) {
    uint16_t gi =
        ConvertGuardIntervalToNanoSeconds(mode, GetWifiPhy()->GetDevice());
    rates.AddSupportedRate(
        mode.GetDataRate(GetWifiPhy()->GetChannelWidth(), gi, 1));
  }
  for (uint32_t j = 0; j < GetWifiRemoteStationManager()->GetNBasicModes();
       j++) {
    WifiMode mode = GetWifiRemoteStationManager()->GetBasicMode(j);
    uint16_t gi =
        ConvertGuardIntervalToNanoSeconds(mode, GetWifiPhy()->GetDevice());
    rates.SetBasicRate(
        mode.GetDataRate(GetWifiPhy()->GetChannelWidth(), gi, 1));
  }
  return rates;
}

bool MeshWifiInterfaceMac::CheckSupportedRates(AllSupportedRates rates) const {
  for (uint32_t i = 0; i < GetWifiRemoteStationManager()->GetNBasicModes();
       i++) {
    WifiMode mode = GetWifiRemoteStationManager()->GetBasicMode(i);
    uint16_t gi =
        ConvertGuardIntervalToNanoSeconds(mode, GetWifiPhy()->GetDevice());
    if (!rates.IsSupportedRate(
            mode.GetDataRate(GetWifiPhy()->GetChannelWidth(), gi, 1))) {
      return false;
    }
  }
  return true;
}

void MeshWifiInterfaceMac::SetRandomStartDelay(Time interval) {
  NS_LOG_FUNCTION(this << interval);
  m_randomStart = interval;
}

void MeshWifiInterfaceMac::SetBeaconInterval(Time interval) {
  NS_LOG_FUNCTION(this << interval);
  m_beaconInterval = interval;
}

Time MeshWifiInterfaceMac::GetBeaconInterval() const {
  return m_beaconInterval;
}

void MeshWifiInterfaceMac::SetBeaconGeneration(bool enable) {
  NS_LOG_FUNCTION(this << enable);
  m_beaconEnable = enable;
}

bool MeshWifiInterfaceMac::GetBeaconGeneration() const {
  return m_beaconSendEvent.IsRunning();
}

Time MeshWifiInterfaceMac::GetTbtt() const { return m_tbtt; }

void MeshWifiInterfaceMac::ShiftTbtt(Time shift) {
  NS_ASSERT(GetTbtt() + shift > Simulator::Now());

  m_tbtt += shift;
  Simulator::Cancel(m_beaconSendEvent);
  m_beaconSendEvent = Simulator::Schedule(
      GetTbtt() - Simulator::Now(), &MeshWifiInterfaceMac::SendBeacon, this);
}

void MeshWifiInterfaceMac::ScheduleNextBeacon() {
  m_tbtt += GetBeaconInterval();
  m_beaconSendEvent = Simulator::Schedule(
      GetBeaconInterval(), &MeshWifiInterfaceMac::SendBeacon, this);
}

void MeshWifiInterfaceMac::SendBeacon() {
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG(GetAddress() << " is sending beacon");

  NS_ASSERT(!m_beaconSendEvent.IsRunning());

  MeshWifiBeacon beacon(GetSsid(), GetSupportedRates(),
                        m_beaconInterval.GetMicroSeconds());

  for (auto i = m_plugins.begin(); i != m_plugins.end(); ++i) {
    (*i)->UpdateBeacon(beacon);
  }
  m_txop->Queue(beacon.CreatePacket(),
                beacon.CreateHeader(GetAddress(), GetMeshPointAddress()));

  ScheduleNextBeacon();
}

void MeshWifiInterfaceMac::Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  const WifiMacHeader *hdr = &mpdu->GetHeader();
  Ptr<Packet> packet = mpdu->GetPacket()->Copy();
  if ((hdr->GetAddr1() != GetAddress()) &&
      (hdr->GetAddr1() != Mac48Address::GetBroadcast())) {
    return;
  }
  if (hdr->IsBeacon()) {
    m_stats.recvBeacons++;
    MgtBeaconHeader beacon_hdr;

    packet->PeekHeader(beacon_hdr);

    NS_LOG_DEBUG("Beacon received from "
                 << hdr->GetAddr2() << " I am " << GetAddress() << " at "
                 << Simulator::Now().GetMicroSeconds() << " microseconds");

    if (beacon_hdr.Get<Ssid>()->IsEqual(GetSsid())) {
      NS_ASSERT(beacon_hdr.Get<SupportedRates>());
      auto rates =
          AllSupportedRates{*beacon_hdr.Get<SupportedRates>(),
                            beacon_hdr.Get<ExtendedSupportedRatesIE>()};

      for (const auto &mode : GetWifiPhy()->GetModeList()) {
        uint16_t gi =
            ConvertGuardIntervalToNanoSeconds(mode, GetWifiPhy()->GetDevice());
        uint64_t rate =
            mode.GetDataRate(GetWifiPhy()->GetChannelWidth(), gi, 1);
        if (rates.IsSupportedRate(rate)) {
          GetWifiRemoteStationManager()->AddSupportedMode(hdr->GetAddr2(),
                                                          mode);
          if (rates.IsBasicRate(rate)) {
            GetWifiRemoteStationManager()->AddBasicMode(mode);
          }
        }
      }
    }
  } else {
    m_stats.recvBytes += packet->GetSize();
    m_stats.recvFrames++;
  }
  for (auto i = m_plugins.begin(); i != m_plugins.end(); ++i) {
    bool drop = !((*i)->Receive(packet, *hdr));
    if (drop) {
      return;
    }
  }
  if (hdr->IsQosData()) {
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(hdr->GetQosTid());
    packet->ReplacePacketTag(priorityTag);
  }
  if (hdr->IsData()) {
    ForwardUp(packet, hdr->GetAddr4(), hdr->GetAddr3());
  }
}

uint32_t MeshWifiInterfaceMac::GetLinkMetric(Mac48Address peerAddress) {
  uint32_t metric = 1;
  if (!m_linkMetricCallback.IsNull()) {
    metric = m_linkMetricCallback(peerAddress, this);
  }
  return metric;
}

void MeshWifiInterfaceMac::SetLinkMetricCallback(
    Callback<uint32_t, Mac48Address, Ptr<MeshWifiInterfaceMac>> cb) {
  m_linkMetricCallback = cb;
}

void MeshWifiInterfaceMac::SetMeshPointAddress(Mac48Address a) {
  m_mpAddress = a;
}

Mac48Address MeshWifiInterfaceMac::GetMeshPointAddress() const {
  return m_mpAddress;
}

MeshWifiInterfaceMac::Statistics::Statistics()
    : recvBeacons(0), sentFrames(0), sentBytes(0), recvFrames(0), recvBytes(0) {
}

void MeshWifiInterfaceMac::Statistics::Print(std::ostream &os) const {
  os << "<Statistics "
        "rxBeacons=\""
     << recvBeacons
     << "\" "
        "txFrames=\""
     << sentFrames
     << "\" "
        "txBytes=\""
     << sentBytes
     << "\" "
        "rxFrames=\""
     << recvFrames
     << "\" "
        "rxBytes=\""
     << recvBytes << "\"/>" << std::endl;
}

void MeshWifiInterfaceMac::Report(std::ostream &os) const {
  os << "<Interface "
        "BeaconInterval=\""
     << GetBeaconInterval().GetSeconds()
     << "\" "
        "Channel=\""
     << GetFrequencyChannel()
     << "\" "
        "Address = \""
     << GetAddress() << "\">" << std::endl;
  m_stats.Print(os);
  os << "</Interface>" << std::endl;
}

void MeshWifiInterfaceMac::ResetStats() { m_stats = Statistics(); }

void MeshWifiInterfaceMac::ConfigureStandard(WifiStandard standard) {
  NS_ABORT_IF(!GetQosSupported());
  WifiMac::ConfigureStandard(standard);
  m_standard = standard;
}

void MeshWifiInterfaceMac::ConfigureContentionWindow(uint32_t cwMin,
                                                     uint32_t cwMax) {
  WifiMac::ConfigureContentionWindow(cwMin, cwMax);
  m_txop = CreateObject<Txop>();
  m_txop->SetWifiMac(this);
  GetLink(0).channelAccessManager->Add(m_txop);
  m_txop->SetTxMiddle(m_txMiddle);
  m_txop->SetMinCw(0);
  m_txop->SetMaxCw(0);
  m_txop->SetAifsn(1);
  m_scheduler->SetWifiMac(this);
}
} // namespace ns3
