
#include "wifi-mac-helper.h"

#include "ns3/boolean.h"
#include "ns3/eht-configuration.h"
#include "ns3/emlsr-manager.h"
#include "ns3/frame-exchange-manager.h"
#include "ns3/multi-user-scheduler.h"
#include "ns3/wifi-ack-manager.h"
#include "ns3/wifi-assoc-manager.h"
#include "ns3/wifi-mac-queue-scheduler.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-protection-manager.h"

namespace ns3 {

WifiMacHelper::WifiMacHelper() {
  SetType("ns3::AdhocWifiMac");

  m_assocManager.SetTypeId("ns3::WifiDefaultAssocManager");
  m_queueScheduler.SetTypeId("ns3::FcfsWifiQueueScheduler");
  m_protectionManager.SetTypeId("ns3::WifiDefaultProtectionManager");
  m_ackManager.SetTypeId("ns3::WifiDefaultAckManager");
  m_emlsrManager.SetTypeId("ns3::DefaultEmlsrManager");
}

WifiMacHelper::~WifiMacHelper() {}

Ptr<WifiMac> WifiMacHelper::Create(Ptr<WifiNetDevice> device,
                                   WifiStandard standard) const {
  NS_ABORT_MSG_IF(standard == WIFI_STANDARD_UNSPECIFIED,
                  "No standard specified!");

  ObjectFactory macObjectFactory = m_mac;
  if (standard >= WIFI_STANDARD_80211n) {
    macObjectFactory.Set("QosSupported", BooleanValue(true));
  }

  Ptr<WifiMac> mac = macObjectFactory.Create<WifiMac>();
  mac->SetDevice(device);
  mac->SetAddress(Mac48Address::Allocate());
  device->SetMac(mac);
  mac->ConfigureStandard(standard);

  Ptr<WifiMacQueueScheduler> queueScheduler =
      m_queueScheduler.Create<WifiMacQueueScheduler>();
  mac->SetMacQueueScheduler(queueScheduler);

  for (uint8_t linkId = 0; linkId < std::max<uint8_t>(device->GetNPhys(), 1);
       ++linkId) {
    auto fem = mac->GetFrameExchangeManager(linkId);

    Ptr<WifiProtectionManager> protectionManager =
        m_protectionManager.Create<WifiProtectionManager>();
    protectionManager->SetWifiMac(mac);
    protectionManager->SetLinkId(linkId);
    fem->SetProtectionManager(protectionManager);

    Ptr<WifiAckManager> ackManager = m_ackManager.Create<WifiAckManager>();
    ackManager->SetWifiMac(mac);
    ackManager->SetLinkId(linkId);
    fem->SetAckManager(ackManager);

    if (device->GetNPhys() > 1) {
      fem->SetAddress(Mac48Address::Allocate());
    }
  }

  Ptr<ApWifiMac> apMac;
  if (standard >= WIFI_STANDARD_80211ax && m_muScheduler.IsTypeIdSet() &&
      (apMac = DynamicCast<ApWifiMac>(mac))) {
    Ptr<MultiUserScheduler> muScheduler =
        m_muScheduler.Create<MultiUserScheduler>();
    apMac->AggregateObject(muScheduler);
  }

  auto staMac = DynamicCast<StaWifiMac>(mac);
  if (staMac) {
    Ptr<WifiAssocManager> assocManager =
        m_assocManager.Create<WifiAssocManager>();
    staMac->SetAssocManager(assocManager);
  }

  if (BooleanValue emlsrActivated;
      standard >= WIFI_STANDARD_80211be && staMac && staMac->GetNLinks() > 1 &&
      device->GetEhtConfiguration()->GetAttributeFailSafe("EmlsrActivated",
                                                          emlsrActivated) &&
      emlsrActivated.Get()) {
    auto emlsrManager = m_emlsrManager.Create<EmlsrManager>();
    staMac->SetEmlsrManager(emlsrManager);
  }

  return mac;
}

} // namespace ns3
