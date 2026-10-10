
#include "default-emlsr-manager.h"

#include "ns3/boolean.h"
#include "ns3/channel-access-manager.h"
#include "ns3/log.h"
#include "ns3/wifi-mpdu.h"
#include "ns3/wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DefaultEmlsrManager");

NS_OBJECT_ENSURE_REGISTERED(DefaultEmlsrManager);

TypeId DefaultEmlsrManager::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::DefaultEmlsrManager")
          .SetParent<EmlsrManager>()
          .SetGroupName("Wifi")
          .AddConstructor<DefaultEmlsrManager>()
          .AddAttribute(
              "SwitchAuxPhy",
              "Whether Aux PHY should switch channel to operate on the link on "
              "which "
              "the Main PHY was operating before moving to the link of the Aux "
              "PHY.",
              BooleanValue(true),
              MakeBooleanAccessor(&DefaultEmlsrManager::m_switchAuxPhy),
              MakeBooleanChecker());
  return tid;
}

DefaultEmlsrManager::DefaultEmlsrManager() { NS_LOG_FUNCTION(this); }

DefaultEmlsrManager::~DefaultEmlsrManager() { NS_LOG_FUNCTION_NOARGS(); }

void DefaultEmlsrManager::DoNotifyMgtFrameReceived(Ptr<const WifiMpdu> mpdu,
                                                   uint8_t linkId) {
  NS_LOG_FUNCTION(this << *mpdu << linkId);
}

uint8_t DefaultEmlsrManager::GetLinkToSendEmlOmn() {
  NS_LOG_FUNCTION(this);
  auto linkId = GetStaMac()->GetLinkForPhy(m_mainPhyId);
  NS_ASSERT_MSG(linkId, "Link on which the main PHY is operating not found");
  return *linkId;
}

std::optional<uint8_t>
DefaultEmlsrManager::ResendNotification(Ptr<const WifiMpdu> mpdu) {
  NS_LOG_FUNCTION(this);
  auto linkId = GetStaMac()->GetLinkForPhy(m_mainPhyId);
  NS_ASSERT_MSG(linkId, "Link on which the main PHY is operating not found");
  return *linkId;
}

void DefaultEmlsrManager::NotifyEmlsrModeChanged() { NS_LOG_FUNCTION(this); }

void DefaultEmlsrManager::NotifyMainPhySwitch(uint8_t currLinkId,
                                              uint8_t nextLinkId) {
  NS_LOG_FUNCTION(this << currLinkId << nextLinkId);

  if (!m_switchAuxPhy) {
    return;
  }

  auto auxPhy = GetStaMac()->GetWifiPhy(nextLinkId);

  auto newAuxPhyChannel = GetChannelForAuxPhy(currLinkId);

  NS_LOG_DEBUG("Aux PHY (" << auxPhy << ") is about to switch to "
                           << newAuxPhyChannel << " to operate on link "
                           << +currLinkId);

  GetStaMac()
      ->GetChannelAccessManager(nextLinkId)
      ->NotifySwitchingEmlsrLink(auxPhy, newAuxPhyChannel, currLinkId);

  void (WifiPhy::*fp)(const WifiPhyOperatingChannel &) =
      &WifiPhy::SetOperatingChannel;
  Simulator::ScheduleNow(fp, auxPhy, newAuxPhyChannel);
}

} // namespace ns3
