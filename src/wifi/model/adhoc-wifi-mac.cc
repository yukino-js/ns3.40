
#include "adhoc-wifi-mac.h"

#include "qos-txop.h"

#include "ns3/eht-capabilities.h"
#include "ns3/he-capabilities.h"
#include "ns3/ht-capabilities.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/vht-capabilities.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AdhocWifiMac");

NS_OBJECT_ENSURE_REGISTERED(AdhocWifiMac);

TypeId AdhocWifiMac::GetTypeId() {
  static TypeId tid = TypeId("ns3::AdhocWifiMac")
                          .SetParent<WifiMac>()
                          .SetGroupName("Wifi")
                          .AddConstructor<AdhocWifiMac>();
  return tid;
}

AdhocWifiMac::AdhocWifiMac() {
  NS_LOG_FUNCTION(this);
  SetTypeOfStation(ADHOC_STA);
}

AdhocWifiMac::~AdhocWifiMac() { NS_LOG_FUNCTION(this); }

bool AdhocWifiMac::CanForwardPacketsTo(Mac48Address to) const { return true; }

void AdhocWifiMac::Enqueue(Ptr<Packet> packet, Mac48Address to) {
  NS_LOG_FUNCTION(this << packet << to);
  if (GetWifiRemoteStationManager()->IsBrandNew(to)) {
    if (GetHtSupported()) {
      GetWifiRemoteStationManager()->AddAllSupportedMcs(to);
      GetWifiRemoteStationManager()->AddStationHtCapabilities(
          to, GetHtCapabilities(SINGLE_LINK_OP_ID));
    }
    if (GetVhtSupported(SINGLE_LINK_OP_ID)) {
      GetWifiRemoteStationManager()->AddStationVhtCapabilities(
          to, GetVhtCapabilities(SINGLE_LINK_OP_ID));
    }
    if (GetHeSupported()) {
      GetWifiRemoteStationManager()->AddStationHeCapabilities(
          to, GetHeCapabilities(SINGLE_LINK_OP_ID));
    }
    if (GetEhtSupported()) {
      GetWifiRemoteStationManager()->AddStationEhtCapabilities(
          to, GetEhtCapabilities(SINGLE_LINK_OP_ID));
    }
    GetWifiRemoteStationManager()->AddAllSupportedModes(to);
    GetWifiRemoteStationManager()->RecordDisassociated(to);
  }

  WifiMacHeader hdr;

  uint8_t tid = 0;

  if (GetQosSupported()) {
    hdr.SetType(WIFI_MAC_QOSDATA);
    hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
    hdr.SetQosNoEosp();
    hdr.SetQosNoAmsdu();
    hdr.SetQosTxopLimit(0);

    tid = QosUtilsGetTidForPacket(packet);
    if (tid > 7) {
      tid = 0;
    }
    hdr.SetQosTid(tid);
  } else {
    hdr.SetType(WIFI_MAC_DATA);
  }

  if (GetHtSupported()) {
    hdr.SetNoOrder();
  }
  hdr.SetAddr1(to);
  hdr.SetAddr2(GetAddress());
  hdr.SetAddr3(GetBssid(0));
  hdr.SetDsNotFrom();
  hdr.SetDsNotTo();

  if (GetQosSupported()) {
    NS_ASSERT(tid < 8);
    GetQosTxop(tid)->Queue(packet, hdr);
  } else {
    GetTxop()->Queue(packet, hdr);
  }
}

void AdhocWifiMac::SetLinkUpCallback(Callback<void> linkUp) {
  NS_LOG_FUNCTION(this << &linkUp);
  WifiMac::SetLinkUpCallback(linkUp);

  linkUp();
}

void AdhocWifiMac::Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << +linkId);
  const WifiMacHeader *hdr = &mpdu->GetHeader();
  NS_ASSERT(!hdr->IsCtl());
  Mac48Address from = hdr->GetAddr2();
  Mac48Address to = hdr->GetAddr1();
  if (GetWifiRemoteStationManager()->IsBrandNew(from)) {
    if (GetHtSupported()) {
      GetWifiRemoteStationManager()->AddAllSupportedMcs(from);
      GetWifiRemoteStationManager()->AddStationHtCapabilities(
          from, GetHtCapabilities(SINGLE_LINK_OP_ID));
    }
    if (GetVhtSupported(SINGLE_LINK_OP_ID)) {
      GetWifiRemoteStationManager()->AddStationVhtCapabilities(
          from, GetVhtCapabilities(SINGLE_LINK_OP_ID));
    }
    if (GetHeSupported()) {
      GetWifiRemoteStationManager()->AddStationHeCapabilities(
          from, GetHeCapabilities(SINGLE_LINK_OP_ID));
    }
    if (GetEhtSupported()) {
      GetWifiRemoteStationManager()->AddStationEhtCapabilities(
          from, GetEhtCapabilities(SINGLE_LINK_OP_ID));
    }
    GetWifiRemoteStationManager()->AddAllSupportedModes(from);
    GetWifiRemoteStationManager()->RecordDisassociated(from);
  }
  if (hdr->IsData()) {
    if (hdr->IsQosData() && hdr->IsQosAmsdu()) {
      NS_LOG_DEBUG("Received A-MSDU from" << from);
      DeaggregateAmsduAndForward(mpdu);
    } else {
      ForwardUp(mpdu->GetPacket()->Copy(), from, to);
    }
    return;
  }

  WifiMac::Receive(mpdu, linkId);
}

} // namespace ns3
